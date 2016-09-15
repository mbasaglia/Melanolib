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
 */
#ifndef MELANOLIB_STRING_TEXT_GENERATOR_HPP
#define MELANOLIB_STRING_TEXT_GENERATOR_HPP

#include <sstream>
#include <unordered_map>
#include <iterator>
#include <vector>
#include <algorithm>
#include <mutex>

#include "melanolib/math/random.hpp"
#include "melanolib/string/simple_stringutils.hpp"
#include "melanolib/time/units.hpp"
#include "melanolib/time/date_time.hpp"
#include "melanolib/data_structures/icase_functors.hpp"
#include "melanolib/data_structures/id_pool.hpp"

namespace melanolib {
namespace string {

/**
 * \brief Markov text generator
 */
class TextGenerator
{
    using Clock = time::DateTime::Clock;
    struct Node;
    using NodeId = std::uintptr_t;
    struct NodeIterator;
    struct GraphFormatter;
    struct GraphDotFormatter;
    enum class Direction
    {
        Forward,
        Backward
    };

public:
    enum class StorageFormat
    {
        TextPlain,
        Binary,
        Dot
    };

    struct Stats
    {
        std::size_t word_count = 0;  ///< Number of known words
        std::size_t transitions = 0; ///< Word pair to next word transitions
        std::string most_common;     ///< Most common word
    };

    /**
     * \brief Creates a TextGenerator
     * \param max_size      Maximum number of transition in the Markov chain
     * \param max_age       Transition age cutoff
     */
    explicit TextGenerator(std::size_t max_size = 65535,
                           time::days max_age = time::days(30));
    
    TextGenerator(TextGenerator&& oth);
    TextGenerator& operator=(TextGenerator&& oth);

    ~TextGenerator();

    /**
     * \brief Parses the contents of \p stream and adds the required states to
     *        the graph.
     */
    void add_text(std::istream& stream);

    void add_text(std::istream&& stream)
    {
        add_text(stream);
    }

    void add_text(const std::string& input)
    {
        add_text(std::istringstream(input));
    }

    /**
     * \brief Attempts to generate between \p min_words and \p max_words words
     *
     * Depending on the available data, it might end up with fewer words.
     * \p max_words is a hard upper limit.
     *
     * It will attempt to generate at least \p min_words.
     * After \p enough_words it will try to find a good ending spot.
     * After \p max_words it will stop even if it isn't on a terminating node.
     */
    std::vector<std::string> generate_words(
        std::size_t min_words,
        std::size_t enough_words,
        std::size_t max_words) const;

    /**
     * \brief Attempts to generate between \p min_words and \p max_words words
     *
     * The generated phrase always contains \p prompt.
     *
     * Depending on the available data, it might end up with fewer words.
     * \p max_words is a hard upper limit.
     *
     * It will attempt to generate at least \p min_words.
     * After \p enough_words it will try to find a good ending spot.
     * After \p max_words it will stop even if it isn't on a terminating node.
     */
    std::vector<std::string> generate_words(
        const std::string& prompt,
        std::size_t min_words,
        std::size_t enough_words,
        std::size_t max_words) const;

    /**
     * \brief Attempts to generate between \p min_words and \p max_words words
     *
     * Depending on the available data, it might end up with fewer words.
     * \p max_words is a hard upper limit.
     */
    std::string generate_string(
        std::size_t min_words,
        std::size_t enough_words,
        std::size_t max_words) const
    {
        return join(generate_words(min_words, enough_words, max_words));
    }

    /**
     * \brief Attempts to generate between \p min_words and \p max_words words
     *
     * The generated phrase always contains \p prompt.
     *
     * Depending on the available data, it might end up with fewer words.
     * \p max_words is a hard upper limit.
     */
    std::string generate_string(
        const std::string& prompt,
        std::size_t min_words,
        std::size_t enough_words,
        std::size_t max_words) const
    {
        return join(generate_words(prompt, min_words, enough_words, max_words));
    }

    /**
     * \brief Removes all transitions in the Markov chain older than max_age
     */
    void cleanup();

    void set_max_size(std::size_t entries);

    void set_max_age(time::days days);

    std::size_t max_size() const
    {
        return _max_size;
    }

    time::days max_age() const
    {
        return _max_age;
    }

    /**
     * \brief Stores a representation of the internal data structure to a stream
     * \note This only includes text data, not max_age and the like
     */
    void store(std::ostream& output, StorageFormat format = StorageFormat::TextPlain) const;

    /**
     * \brief Loads a representation of the internal data structure from a stream
     * \note This only includes text data, not max_age and the like
     */
    void load(std::istream& input, StorageFormat format = StorageFormat::TextPlain);

    /**
     * \brief Returns some stats about the internal graph
     */
    Stats stats() const;

    /**
     * \brief whether the generator limits the number of known words
     */
    bool limit_size() const
    {
        return _max_size != std::numeric_limits<std::size_t>::max();
    }

protected:
    /**
     * \brief Input token
     */
    struct Token
    {
        std::string text; ///< Token contents
        bool is_start = false; ///< Whether it should mark the start of a phrase
        bool is_end = false; ///< Whether it should mark the end of a phrase

        /**
         * \brief Whether the token is valid
         *
         * (ie: \b text is not empty)
         */
        bool valid() const
        {
            return !text.empty();
        }
    };

    /**
     * \brief Shall read a Token from the input stream, marking is_start and
     * is_end when appropriate.
     *
     * When the first invalid (empty) token is returned, the rest of the input
     * is discarded and the last valid token is marked as an ending point.
     */
    virtual Token next_token(std::istream& input) const;

    /**
     * \brief Returns a normalized form for the input word, ensuring
     * two normalized values are equal iff the input strings are equivalent
     *
     * This is useful multiple strings may be considered as equivalent,
     * eg: to make this case-insensitive
     */
    virtual std::string normalize(const std::string& word) const;

    virtual std::string join(const std::vector<std::string>& words) const
    {
        return implode(" ", words);
    }

    virtual std::vector<std::string> split(const std::string& words) const;

private:
    void expand(
        Direction direction,
        std::vector<std::string>& words,
        std::size_t min_words,
        std::size_t enough_words,
        std::size_t cutoff
    ) const;

    void generate(
        NodeIterator iterator,
        std::vector<std::string>& words,
        std::size_t min_words,
        std::size_t enough_words,
        std::size_t cutoff
    ) const;

    Node* node_for_nocreate(const std::string& word) const;

    Node* node_for(const std::string& word);

    void mark_start(Node* node);

    /**
     * \see cleanup()
     */
    void cleanup_unlocked();

    std::vector<Node*> start;
    std::unordered_map<std::string, std::unique_ptr<Node>> words;
    std::size_t _max_size;
    time::days _max_age;
    Clock::time_point last_cleanup = Clock::time_point::min();
    BasicIdPool<NodeId> id_pool;
    mutable std::mutex mutex;
};

/**
 * \brief Markov word generator
 */
class WordGenerator : public TextGenerator
{
public:
    WordGenerator(
        std::size_t chunk_size = 1,
        std::size_t max_size = 65535,
        time::days max_age = time::days(30))
    : TextGenerator(max_size, max_age),
      _chunk_size(chunk_size)
    {}

    std::size_t chunk_size() const
    {
        return _chunk_size;
    }

    void set_chunk_size(std::size_t chunk_size)
    {
        _chunk_size = chunk_size;
    }

protected:
    Token next_token(std::istream& input) const override
    {
        Token token;

        while ( token.text.size() < _chunk_size )
        {
            int ch = input.get();

            if ( !input )
                break;

            if ( ascii::is_space(ch) )
            {
                token.text += ' ';
                do
                    ch = input.get();
                while ( ascii::is_space(ch) );
                if ( input )
                    input.unget();
            }
            else
            {
                token.text += ch;
            }
        }
        return token;
    }

    std::string normalize(const std::string& word) const override
    {
        return word;
    }

    std::string join(const std::vector<std::string>& words) const override
    {
        return std::accumulate(words.begin(), words.end(), std::string{});
    }

    std::vector<std::string> split(const std::string& words) const override
    {
        std::vector<std::string> result;
        if ( ! words.empty() )
        {
            result.reserve(words.size() / _chunk_size);
            std::size_t i;
            for ( i = 0; i + _chunk_size < words.size(); i += _chunk_size )
                result.push_back(words.substr(i, _chunk_size));
            result.push_back(words.substr(i));
        }
        return result;
    }

private:
    std::size_t _chunk_size;
};

} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_TEXT_GENERATOR_HPP
