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

#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <mutex>

#include "melanolib/math/random.hpp"
#include "melanolib/math/math.hpp"
#include "melanolib/string/simple_stringutils.hpp"
#include "melanolib/time/units.hpp"

namespace melanolib {
namespace string {

/**
 * \brief Markov text generator
 */
class TextGenerator
{
    struct Prefix
    {
        Prefix(){}

        void shift(const std::string& word)
        {
            first = std::move(second);
            second = word;
        }

        bool operator==(const Prefix& p) const
        {
            return icase_equal(first, p.first) && icase_equal(second, p.second);
        }

        std::string first;
        std::string second;
    };

    struct PrefixHasher
    {
        size_t operator()(const Prefix& prefix) const
        {
            std::hash<std::string> hasher;
            std::size_t hash = hasher(strtolower(prefix.first));
            hash ^= hasher(strtolower(prefix.second)) + 0x9e3779b9 + (hash<<6) + (hash>>2);
            return hash;
        }
    };

    using Clock = std::chrono::steady_clock;

    struct Suffix
    {
        std::string text;
        bool end;
        Clock::time_point creation;
    };

    using Chain = std::unordered_multimap<Prefix, Suffix, PrefixHasher>;
public:

    /**
     * \brief Creates a TextGenerator
     * \param max_size      Maximum number of transition in the Markov chain
     * \param max_age       Transition age cutoff
     */
    explicit TextGenerator(std::size_t max_size = 65535,
                           time::days max_age = time::days(30))
        : max_size(max_size), max_age(max_age)
    {}

    /**
     * \brief Parses the contents of \p stream and adds the required state to
     *        the model graph.
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
     * \brief Generates between \p min_words and \p max_words random words
     */
    std::vector<std::string> generate_words(std::size_t min_words,
                                            std::size_t max_words) const;

    /**
     * \brief Generates between \p min_words and \p max_words random words,
     *        one of which must be \p prompt
     * \note if \p prompt is not in the model, the result might contain less
     *       than \p min_words words
     */
    std::vector<std::string> generate_words(const std::string& prompt,
                                            std::size_t min_words,
                                            std::size_t max_words) const;

    /**
     * \brief Generates between \p min_words and \p max_words random words
     */
    std::string generate_string(std::size_t min_words, std::size_t max_words) const
    {
        return implode(" ", generate_words(min_words, max_words));
    }

    /**
     * \brief Generates between \p min_words and \p max_words random words,
     *        one of which must be \p prompt
     * \note if \p prompt is not in the model, the result might contain less
     *       than \p min_words words
     */
    std::string generate_string(const std::string& prompt,
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

private:

    /**
     * \brief Uses \p prefix to generate words into \p words
     */
    void generate_words_unlocked(Prefix prefix,
                                 std::size_t min_words,
                                 std::size_t max_words,
                                 std::vector<std::string>& words) const;

    /**
     * \brief Selects an iterator matching \p prefix
     */
    Chain::const_iterator random_suffix(const Prefix& prefix) const
    {
        auto choices = chain.equal_range(prefix);

        if ( choices.first == choices.second )
            return chain.end();

        auto count = std::distance(choices.first, choices.second);
        std::advance(choices.first, math::random(count-1));
        return choices.first;
    }

    /**
     * \see cleanup()
     */
    void cleanup_unlocked();

    /**
     * \brief Looks at the beginning of \p words and tracks back until
     *        \p max_words or a delimiter is found
     */
    Prefix walk_back(std::size_t max_words, std::vector<std::string>& words) const;

    Chain chain;
    std::size_t max_size;
    time::days max_age;
    Clock::time_point last_cleanup = Clock::time_point::min();
    mutable std::mutex mutex;
};

} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_TEXT_GENERATOR_HPP
