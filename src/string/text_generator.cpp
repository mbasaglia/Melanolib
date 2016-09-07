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

namespace melanolib {
namespace string {


void TextGenerator::add_text(std::istream& stream)
{
    Prefix prefix(prefix_size);
    std::string word;
    Chain::iterator last = chain.end();
    while ( stream.good() )
    {
        int ch = ' ';
        while ( ascii::is_space(ch) && stream.good() )
        {
            ch = stream.get();
            if ( ch == '\n' )
            {
                prefix = Prefix(prefix_size);
                if ( last != chain.end() )
                    last->second.end = true;
            }
        }
        if ( !stream.good() )
            break;
        stream.unget();

        if ( stream >> word )
        {
            bool end = word.back() == '.' || stream.eof();
            last = chain.insert({prefix, {word, end, Clock::now()}});
            prefix.shift(word);

            if ( chain.size() > max_size )
                cleanup();
        }
    }

    if ( last != chain.end() )
        last->second.end = true;
}

std::vector<std::string> TextGenerator::generate_words(
    std::size_t min_words,
    std::size_t max_words) const
{
    std::vector<std::string> words;
    words.reserve(min_words);
    generate_words(Prefix(prefix_size), min_words, max_words, words);
    return words;
}

std::vector<std::string> TextGenerator::generate_words(
    const std::string& prompt,
    std::size_t min_words,
    std::size_t max_words) const
{

    std::vector<std::string> words;
    words.reserve(min_words);
    words.push_back(prompt);
    Prefix prefix = walk_back(max_words/2, words);
    generate_words(prefix, min_words, max_words, words);
    return words;
}

void TextGenerator::cleanup()
{
    auto now = Clock::now();
    auto death = Clock::now() - max_age;

    if ( last_cleanup < death )
    {
        for ( Chain::iterator iter = chain.begin(); iter != chain.end(); )
        {
            if ( iter->second.creation < death )
                iter = chain.erase(iter);
            else
                ++iter;
        }
    }

    while ( chain.size() > max_size )
    {
        Chain::iterator iter = chain.begin();
        std::advance(iter, math::random(chain.size() - 1));
        chain.erase(iter);
    }

    last_cleanup = now;
}

void TextGenerator::generate_words(Prefix prefix,
                                   std::size_t min_words,
                                   std::size_t max_words,
                                   std::vector<std::string>& words) const
{
    while ( words.size() < max_words )
    {
        auto iter = random_suffix(prefix);
        if ( iter == chain.end() )
            return;
        words.push_back(iter->second.text);
        if ( words.size() >= min_words && iter->second.end )
            break;
        prefix.shift(words.back());
    }
}


TextGenerator::Prefix TextGenerator::walk_back(
    std::size_t max_words,
    std::vector<std::string>& words) const
{
    if ( words.empty() )
        return Prefix(prefix_size);
    while ( words.size() < max_words )
    {
        std::vector<Chain::const_iterator> prefixes;
        bool single_word = words.size() == 1 || prefix_size == 1;
        for ( auto iter = chain.begin(); iter != chain.end(); ++iter )
        {
            if ( single_word )
            {
                if ( icase_equal(iter->second.text, words.front()) )
                    prefixes.push_back(iter);
            }
            else if ( icase_equal(iter->first.back(), words[0]) &&
                        icase_equal(iter->second.text, words[1]) )
            {
                prefixes.push_back(iter);
            }
        }
        if ( prefixes.empty() )
            break;

        auto random_iter = prefixes[math::random(prefixes.size() - 1)];

        if ( random_iter->second.end && words.size() > 1 )
        {
            words.erase(words.begin());
            if ( !single_word )
                words.erase(words.begin());
            break;
        }

        auto& random_prefix = random_iter->first;
        auto prefix_end = random_prefix.end();
        if ( !single_word && random_prefix.begin() != prefix_end  )
            prefix_end--;
        auto prefix_begin = std::find_if(random_prefix.begin(), random_prefix.end(),
            [](const std::string& str) { return !str.empty(); });
        words.insert(words.begin(), prefix_begin, prefix_end);

        if ( random_prefix.front().empty() )
            break;
    }
    Prefix last_prefix(prefix_size);
    std::size_t copy_size = math::min(prefix_size, words.size());
    for ( std::size_t i = 0; i < copy_size; i++ )
        last_prefix[prefix_size - copy_size + i] = words[words.size() - copy_size + i];
    return last_prefix;
}

} // namespace string
} // namespace melanolib
