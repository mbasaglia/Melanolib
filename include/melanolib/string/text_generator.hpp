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
     */
    std::vector<std::string> generate_words(
        std::size_t min_words,
        std::size_t max_words) const;

    /**
     * \brief Attempts to generate between \p min_words and \p max_words words
     *
     * The generated phrase always contains \p prompt.
     *
     * Depending on the available data, it might end up with fewer words.
     * \p max_words is a hard upper limit.
     */
    std::vector<std::string> generate_words(
        const std::string& prompt,
        std::size_t min_words,
        std::size_t max_words) const;

    /**
     * \brief Attempts to generate between \p min_words and \p max_words words
     *
     * Depending on the available data, it might end up with fewer words.
     * \p max_words is a hard upper limit.
     */
    std::string generate_string(
        std::size_t min_words,
        std::size_t max_words) const
    {
        return implode(" ", generate_words(min_words, max_words));
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
        std::size_t max_words) const
    {
        return implode(" ", generate_words(prompt, min_words, max_words));
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
    std::unordered_map<std::string, std::unique_ptr<Node>, ICaseHasher, ICaseComparator> words;
    std::size_t _max_size;
    time::days _max_age;
    Clock::time_point last_cleanup = Clock::time_point::min();
    BasicIdPool<NodeId> id_pool;
    mutable std::mutex mutex;
};

} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_TEXT_GENERATOR_HPP
