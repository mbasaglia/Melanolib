/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2016 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "melanolib/string/text_generator.hpp"

#include "melanolib/math/math.hpp"
#include "melanolib/string/stringutils.hpp"
#include "melanolib/time/time_string.hpp"
#include "melanolib/time/time_parser.hpp"

namespace melanolib {
namespace string {

struct TextGenerator::Node
{
    using Adjacency = std::unordered_multimap<Node*, Node*>;
    using Iterator = Adjacency::const_iterator;
    enum class ExitPolicy
    {
        ShouldNotExit,
        MayExit,
        ShouldExit,
    };

    explicit Node(std::string word)
        : word(std::move(word))
    {
        bump();
    }

    void bump()
    {
        last_updated = Clock::now();
    }

    void connect(Node* before, Node* after)
    {
        forward.insert({before, after});
        backward.insert({after, before});
    }

    Node* walk(Direction direction, Node* from, ExitPolicy policy) const
    {
        auto map = direction == Direction::Forward ? &forward : &backward;
        auto range = map->equal_range(from);
        return choose(range.first, range.second, policy);
    }

    Node* walk_nocontext(Direction direction, ExitPolicy policy) const
    {
        auto map = direction == Direction::Forward ? &forward : &backward;
        return choose(map->begin(), map->end(), policy);
    }

    void drop(Node* node)
    {
        drop_on(forward, node);
        drop_on(backward, node);
    }

    void drop_on(Adjacency& map, Node* node)
    {
        map.erase(node);
        for ( auto iter = map.begin(); iter != map.end(); )
        {
            if ( iter->second == node )
                iter = map.erase(iter);
            else
                ++iter;
        }
    }

    Node* choose(Iterator begin, Iterator end, ExitPolicy policy) const
    {
        if ( begin == end )
            return nullptr;

        std::size_t size = std::distance(begin, end);
        auto iter = begin;
        auto pos = math::random(size-1);
        std::advance(iter, pos);
        if ( policy != ExitPolicy::MayExit )
        {
            for ( std::size_t i = 1; i < size && !acceptable(iter->second, policy); i++ )
            {
                iter++;
                if ( i + pos == size )
                    iter = begin;
            }
        }
        return iter->second;
    }

    bool acceptable(Node* node, ExitPolicy policy) const
    {
        if ( policy == ExitPolicy::ShouldExit )
            return node == nullptr;
        if ( policy == ExitPolicy::ShouldNotExit )
            return node != nullptr;
        return true;
    }

    static ExitPolicy policy_from_size(std::size_t size, std::size_t min, std::size_t max)
    {
        if ( size < min )
            return ExitPolicy::ShouldNotExit;
        if ( size > max )
            return ExitPolicy::ShouldExit;
        return ExitPolicy::MayExit;
    }

    std::string word;
    Adjacency forward;
    Adjacency backward;
    Clock::time_point last_updated;
};


struct TextGenerator::NodeIterator
{
    NodeIterator(Direction direction, Node* node = nullptr, Node* context = nullptr)
        : direction(direction), context(context), node(node)
    {}

    void advance(Node::ExitPolicy exit_policy)
    {
        context = node->walk(direction, context, exit_policy);
        std::swap(context, node);
    }

    void advance_nocontext(Node::ExitPolicy exit_policy)
    {
        context = node->walk_nocontext(direction, exit_policy);
        std::swap(context, node);
    }

    explicit operator bool() const
    {
        return node;
    }

    const std::string& operator*() const
    {
        return node->word;
    }

    Direction direction;
    Node* context;
    Node* node;
};

TextGenerator::TextGenerator(std::size_t max_size, time::days max_age)
    : _max_size(max_size), _max_age(max_age)
{}

TextGenerator::TextGenerator(TextGenerator&& oth)
    : start(std::move(oth.start)),
      words(std::move(oth.words)),
      _max_size(oth._max_size),
      _max_age(oth._max_age),
      last_cleanup(oth.last_cleanup)
{}

TextGenerator& TextGenerator::operator=(TextGenerator&& oth)
{
    std::swap(start, oth.start);
    std::swap(words, oth.words);
    std::swap(_max_size, _max_size);
    std::swap(_max_age, _max_age);
    std::swap(last_cleanup, last_cleanup);

    return *this;
}

TextGenerator::~TextGenerator()
{}

void TextGenerator::add_text(std::istream& stream)
{
    static const uint8_t is_begin = 1;
    static const uint8_t is_end = 2;

    std::lock_guard<std::mutex> lock(mutex);

    std::string word;
    std::deque<std::pair<Node*, uint8_t>> context;

    bool at_bounday = true;
    while ( stream.good() )
    {
        // skip whitespaces
        int ch = ' ';
        while ( ascii::is_space(ch) && stream.good() )
        {
            ch = stream.get();
            // On newline, mark the last item as a good ending point
            if ( ch == '\n' )
            {
                if ( !context.empty() )
                    context.back().second |= is_end;
                at_bounday = true;
            }
        }
        if ( !stream.good() )
            break;
        stream.unget();

        if ( !(stream >> word) )
            break;

        Node* node = node_for(word);
        uint8_t flags = 0;
        if ( at_bounday )
        {
            mark_start(node);
            flags |= is_begin;
        }
        // If the current word ends with a period, is a good ending point
        at_bounday = word.back() == '.' || stream.eof();
        if ( at_bounday )
            flags |= is_end;

        // Add the current word to the end of the context queue
        if ( context.size() == 3 )
            context.pop_front();
        context.push_back({node, flags});

        // Link all the transitions relative to the central node
        if ( context.size() == 3 )
        {
            context[1].first->connect(context[0].first, context[2].first);

            if ( context[1].second & is_begin )
                context[1].first->connect(nullptr, context[2].first);

            if ( context[1].second & is_end )
                context[1].first->connect(context[0].first, nullptr);
        }
        // This only happens once at the start, we know the first node is a starting node
        else if ( context.size() == 2 )
        {
            context[0].first->connect(nullptr, context[1].first);
        }
    }

    // We need to handle the last ending node
    if ( context.size() >= 2 )
        context.back().first->connect(context[context.size()-2].first, nullptr);
    // Handle single words
    else if ( context.size() == 1 )
        context.back().first->connect(nullptr, nullptr);
}

std::vector<std::string> TextGenerator::generate_words(
    std::size_t min_words,
    std::size_t max_words) const
{
    std::lock_guard<std::mutex> lock(mutex);
    if ( start.empty() )
        return {};

    std::vector<std::string> words;
    words.reserve(max_words);
    NodeIterator iterator(
        Direction::Forward,
        start[math::random(start.size()-1)]
    );
    generate(iterator, words, min_words, min_words, max_words);
    return words;
}

std::vector<std::string> TextGenerator::generate_words(
    const std::string& prompt,
    std::size_t min_words,
    std::size_t max_words) const
{
    std::vector<std::string> words = regex_split(prompt, "\\s+");

    if ( words.empty() )
        return generate_words(min_words, max_words);

    std::lock_guard<std::mutex> lock(mutex);
    if ( start.empty() || words.size() >= max_words )
        return words;

    words.reserve(max_words);
    expand(Direction::Backward, words, 0, min_words, max_words);
    expand(Direction::Forward, words, min_words, min_words, max_words);
    return words;
}

void TextGenerator::cleanup()
{
    std::lock_guard<std::mutex> lock(mutex);
    cleanup_unlocked();
}

void TextGenerator::cleanup_unlocked()
{
    auto previous_cleanup = last_cleanup;
    last_cleanup = Clock::now();
    auto death = last_cleanup - _max_age;

    std::vector<Node*> deleted;

    // If the last cleanup was recent, there's no point in trying to erase
    // old nodes as they would already been removed
    if ( previous_cleanup < death )
    {
        // Remove all nodes that are too old
        for ( auto iter = words.begin(); iter != words.end(); )
        {
            if ( iter->second->last_updated < death )
            {
                deleted.push_back(iter->second.get());
                iter = words.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    // Remove random nodes until we get to the maximum allowed size
    while ( words.size() > _max_size )
    {
        auto iter = words.begin();
        std::advance(iter, math::random(words.size() - 1));
        deleted.push_back(iter->second.get());
        words.erase(iter);
    }

    // Remove references to deleted nodes
    for ( auto& node : words )
        for ( auto old : deleted )
            node.second->drop(old);
    std::sort(deleted.begin(), deleted.end());
    auto delete_if = [&deleted](Node* node) {
        return std::binary_search(deleted.begin(), deleted.end(), node);
    };
    start.erase(std::remove_if(start.begin(), start.end(), delete_if), start.end());
}

void TextGenerator::set_max_size(std::size_t entries)
{
    std::lock_guard<std::mutex> lock(mutex);
    _max_size = entries;
    if ( words.size() > entries )
        cleanup_unlocked();
}

void TextGenerator::set_max_age(time::days days)
{
    std::lock_guard<std::mutex> lock(mutex);
    _max_age = days;
}

void TextGenerator::expand(
    Direction direction,
    std::vector<std::string>& words,
    std::size_t min_words,
    std::size_t enough_words,
    std::size_t cutoff) const
{
    if ( words.size() >= cutoff )
        return;

    if ( direction == Direction::Backward )
        std::reverse(words.begin(), words.end());

    NodeIterator iterator(
        direction,
        node_for_nocreate(words.back()),
        words.size() > 1 ? node_for_nocreate(words[words.size() - 2]) : nullptr
    );

    if ( iterator )
    {
        auto policy = Node::policy_from_size(words.size(), min_words, enough_words);
        if ( iterator.context )
            iterator.advance(policy);
        else
            iterator.advance_nocontext(policy);

        generate(iterator, words, min_words, enough_words, cutoff);
    }

    if ( direction == Direction::Backward )
        std::reverse(words.begin(), words.end());
}

void TextGenerator::generate(
    NodeIterator iterator,
    std::vector<std::string>& words,
    std::size_t min_words,
    std::size_t enough_words,
    std::size_t cutoff) const
{
    while ( iterator && words.size() < cutoff )
    {
        words.push_back(*iterator);
        iterator.advance(Node::policy_from_size(words.size(), min_words, enough_words));
    }
}

TextGenerator::Node* TextGenerator::node_for_nocreate(const std::string& word) const
{
    auto iter = words.find(word);
    if ( iter == words.end() )
        return nullptr;
    return iter->second.get();
}

TextGenerator::Node* TextGenerator::node_for(const std::string& word)
{
    auto& node = words[word];

    if ( !node )
        node = New<Node>(word);
    else
        node->bump();

    if ( words.size() > _max_size )
        cleanup_unlocked();

    return node.get();
}

void TextGenerator::mark_start(Node* node)
{
    start.push_back(node);
}

struct TextGenerator::GraphFormatter
{
    using NodeId = uintptr_t;
    using NodeIdMap = std::unordered_map<NodeId, Node*>;

    GraphFormatter(std::ostream& output)
        : stream(output.rdbuf())
    {}

    GraphFormatter(std::istream& input)
        : stream(input.rdbuf())
    {}

    template<class Uint>
        std::enable_if_t<std::is_unsigned<Uint>::value>
        write(Uint value)
    {
        stream << value;
    }

    template<class Uint>
        std::enable_if_t<std::is_unsigned<Uint>::value>
        read(Uint& value)
    {
        if ( !(stream >> value) )
            error();
    }

    template<class Container>
        std::enable_if_t<sizeof(typename Container::value_type)>
        write(const Container& value)
    {
        write(value.size());
        for ( const auto& item : value )
        {
            write_separator(itemsep);
            write(item);
        }
        write_separator(recordsep);
    }

    template<class ValueType>
        void read(std::vector<ValueType>& container)
    {
        std::size_t size;
        read(size);
        container.reserve(size);
        for ( std::size_t i = 0; i < size; i++ )
        {
            read_separator(itemsep);
            ValueType value;
            read(value);
            container.emplace_back(std::move(value));
        }
        read_separator(recordsep);
    }

    void write(Node* node)
    {
        write(NodeId(node));
    }

    void read(Node*& node)
    {
        NodeId node_id = 0;
        read(node_id);
        node = (Node*)(node_id);
    }


    void write(const Clock::time_point& time)
    {
        write(time::format_char(time::DateTime(time), 'c'));
    }

    void read(Clock::time_point& time)
    {
        std::string iso;
        read(iso);
        std::istringstream ss(iso); // this ensures we aren't eating up spaces
        time = time::TimeParser(ss).parse_time_point().time_point();
    }

    void write_separator(char c)
    {
        stream.put(c);
    }

    void read_separator(char c)
    {
        if ( stream.get() != c )
            error();
    }

    void write(const std::string& string)
    {
        stream.write(string.data(), string.size());
    }

    void read(std::string& string)
    {
        if ( !(stream >> string) )
            error();
    }

    void write(const Node::Adjacency::value_type& item)
    {
        write(item.first);
        write_separator(transsep);
        write(item.second);
    }

    void write(const std::unique_ptr<Node>& node)
    {
        write(node.get());
        write_separator(itemsep);
        write(node->word);
        write_separator(itemsep);
        write(node->last_updated);
        write_separator(recordsep);

        write_separator(attrsep);
        write(node->forward);

        write_separator(attrsep);
        write(node->backward);

    }

    void read(Node::Adjacency& adj)
    {
        std::size_t size;
        read(size);
        for ( std::size_t i = 0; i < size; i++ )
        {
            std::remove_const_t<Node::Adjacency::key_type> key;
            read(key);
            read_separator(transsep);
            Node::Adjacency::mapped_type value;
            read(value);
            adj.insert({key, value});
        }
        read_separator(recordsep);
    }

    void read(std::unique_ptr<Node>& node)
    {
        read(node->last_updated);
        read_separator(recordsep);

        read_separator(attrsep);
        read(node->forward);

        read_separator(attrsep);
        read(node->backward);
    }

    void write(const TextGenerator& tg)
    {
        write(tg.start);

        write(tg.words.size());
        write_separator(recordsep);
        for ( const auto& item : tg.words )
            write(item.second);
    }

    void read(TextGenerator& tg)
    {
        tg.words.clear();
        tg.start.clear();

        read(tg.start);

        std::size_t size;
        read(size);
        read_separator(recordsep);

        NodeIdMap node_ids;
        for ( std::size_t i = 0; i < size; i++ )
        {
            NodeId id;
            read(id);
            read_separator(itemsep);

            std::string word;
            read(word);
            read_separator(itemsep);

            auto& ptr = tg.words[word];
            if ( ptr )
                error();

            ptr = New<Node>(word);
            read(ptr);
            node_ids[id] = ptr.get();
        }

        translate(tg.start, node_ids);
        for ( auto& item : tg.words )
        {
            translate(item.second->forward, node_ids);
            translate(item.second->backward, node_ids);
        }
    }

    void translate(std::vector<Node*>& nodes, const NodeIdMap& node_ids)
    {
        for ( auto& node : nodes )
        {
            node = translate(node, node_ids);
        }
    }

    Node* translate(Node* node, const NodeIdMap& node_ids)
    {
        if ( !node )
            return node;

        auto iter = node_ids.find(NodeId(node));
        if ( iter == node_ids.end() )
            error();
        return iter->second;
    }

    void translate(Node::Adjacency& adj_list, const NodeIdMap& node_ids)
    {
        Node::Adjacency new_list;
        for ( const auto& item : adj_list )
        {
            new_list.insert({
                translate(item.first, node_ids),
                translate(item.second, node_ids)
            });
        }
        adj_list = new_list;
    }

    static void error()
    {
        throw std::runtime_error("Invalid format");
    }

    std::iostream stream;
    char itemsep = ' ';
    char recordsep = '\n';
    char attrsep = '\t';
    char transsep = '~';
};

struct TextGenerator::GraphDotFormatter
{
    GraphDotFormatter(std::ostream& output)
        : stream(output.rdbuf())
    {}

    GraphDotFormatter(std::istream& input)
        : stream(input.rdbuf())
    {}

    void write(Node* node)
    {
        if ( !node )
            stream << "finish";
        else
            stream << (uintptr_t(node));
    }

    void write_label(Node* node)
    {
        if ( node )
            stream << "label=\"" << add_slashes(node->word, "\"\\") << '"';
    }

    void write(Node* from, const Node::Adjacency::value_type& item)
    {
        stream << "\t";
        write(from);
        stream << " -> ";
        write(item.second);
        stream << "[";
        if ( item.second == nullptr )
            stream << "color=\"red\" ";
        write_label(item.first);
        stream << "];\n";
    }

    void write(const std::unique_ptr<Node>& node)
    {
        write(node.get());
        stream << "[";
        write_label(node.get());
        stream << "];\n";

        for ( const auto& item: node->forward )
            write(node.get(), item);
    }

    void write(const TextGenerator& tg)
    {
        stream << "// Best rendered with sfdp -Goverlap=scale -Tsvg\n";
        stream << "digraph {\n";
        for ( Node* node : tg.start )
        {
            stream << "\tstart -> ";
            write(node);
            stream << "[color=\"blue\"];\n";
        }

        for ( const auto& item : tg.words )
            write(item.second);

        stream << "}";
    }

    std::ostream stream;
};

void TextGenerator::store(std::ostream& output, StorageFormat format) const
{
    if ( format == StorageFormat::TextPlain )
        GraphFormatter(output).write(*this);
    else if ( format == StorageFormat::Dot )
        GraphDotFormatter(output).write(*this);
    else
        GraphFormatter::error();
}

void TextGenerator::load(std::istream& input, StorageFormat format)
{
    if ( format == StorageFormat::TextPlain )
        GraphFormatter(input).read(*this);
    else
        GraphFormatter::error();
}

} // namespace string
} // namespace melanolib
