/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2015-2016 Mattia Basaglia
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
#ifndef MELANOLIB_STRING_UTILS_HPP
#define MELANOLIB_STRING_UTILS_HPP

#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "trie.hpp"
#include "simple_stringutils.hpp"

namespace melanolib {
namespace string {

/**
 * \brief If the string is longer than \c length,
 * truncates to the last word and adds an ellipsis
 */
std::string elide ( std::string text, int length );


/**
 * \brief Escape all occurrences of \c characters with slashes
 */
std::string add_slashes ( const std::string& input, const std::string& characters );
/**
 * \brief Escapes \c input to be inserted in a regex
 */
inline std::string regex_escape( const std::string& input )
{
    return add_slashes(input, "^$\\.*+?()[]{}|");
}

/**
 * \brief Replace all occurrences of \c from in \c text to \c to
 */
std::string replace(const std::string& input, const std::string& from, const std::string& to);

/**
 * \brief Replaces the keys of \c map to the respective values in \c subject
 * \param subject The string to be searched in
 * \param map     Term/replacement map
 * \param prefix  (Optional) prefix to prepend to all terms
 */
std::string replace(const std::string& subject,
                    const std::unordered_map<std::string, std::string>& map,
                    const std::string& prefix = {});

/**
 * \brief Replaces the keys of \c trie to the respective values in \c subject
 * \param subject The string to be searched in
 * \param trie    Term/replacement map
 */
std::string replace(const std::string& subject, const StringTrie& trie);

/**
 * \brief Checks if \c text matches the wildcard \c pattern
 *
 * \c * matches any sequence of characters, all other characters match themselves
 */
inline bool simple_wildcard(const std::string& text, const std::string& pattern)
{
    std::regex regex_pattern (
        "^"+replace(add_slashes(pattern, "^$\\.+?()[]{}|"), "*", ".*")+"$" );
    return std::regex_match(text, regex_pattern);
}

/**
 * \brief Checks if any of the elements in \c input matches the wildcard \c pattern
 *
 * \c * matches any sequence of characters, all other characters match themselves
 * \tparam Container A container of \c std::string
 */
template <class Container, class = std::enable_if_t<!std::is_convertible<Container, std::string>::value>>
    bool simple_wildcard(const Container& input, const std::string& pattern)
    {
        static_assert(std::is_convertible<typename Container::value_type, std::string>::value,
            "simple_wildcard requires a string container"
        );
        return std::any_of(input.begin(), input.end(),
            [pattern](const std::string& t) { return simple_wildcard(t, pattern); });
    }


/**
 * \brief Separate the string into components separated by \c pattern
 */
std::vector<std::string> regex_split(const std::string& input,
                                     const std::regex& pattern,
                                     bool skip_empty = true );

inline std::vector<std::string> regex_split(const std::string& input,
                                            const std::string& pattern,
                                            bool skip_empty = true )
{
    return regex_split ( input, std::regex(pattern), skip_empty );
}

/**
 * \brief Separate the string into components separated by \c separator
 */
std::vector<std::string> char_split(const std::string& input,
                                    char separator,
                                    bool skip_empty = true);


/**
 * \brief Split a string of element separated by commas and spaces
 */
inline std::vector<std::string> comma_split(const std::string& input, bool skip_empty = true)
{
    static std::regex regex_commaspace ( "(,\\s*)|(\\s+)",
        std::regex_constants::optimize |
        std::regex_constants::ECMAScript );
    return string::regex_split(input, regex_commaspace, skip_empty);
}

/**
 * \brief Returns a value representing how similar the two strings are
 */
std::string::size_type similarity(const std::string& s1, const std::string& s2);

/**
 * \brief Converts a number of bytes into a human-readable format
 */
std::string pretty_bytes(uint64_t bytes);


} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_UTILS_HPP
