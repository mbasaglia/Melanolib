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
        // skip whitespaces
        int ch = ' ';
        while ( ascii::is_space(ch) && stream.good() )
        {
            ch = stream.get();
            // On newline, mark the last item as a good ending point
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

        // Read a word from the stream and add it to the chain
        if ( stream >> word )
        {
            // If the current word ends with a period, is a good ending point
            bool end = word.back() == '.' || stream.eof();
            last = chain.insert({prefix, {word, end, Clock::now()}});
            prefix.shift(word);

            // Ensure we don't exceed the maximum size
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

    // If the last cleanup was recent, there's no point in trying to erase
    // old transitions as they would already been removed
    if ( last_cleanup < death )
    {
        // Remove all entries that are too old
        for ( Chain::iterator iter = chain.begin(); iter != chain.end(); )
        {
            if ( iter->second.creation < death )
                iter = chain.erase(iter);
            else
                ++iter;
        }
    }

    // Remove random transitions until we get to the maximum allowed size
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
        // Select a random suffix for the current prefix
        auto iter = random_suffix(prefix);
        // If we can't continue, terminate here
        if ( iter == chain.end() )
            break;
        words.push_back(iter->second.text);
        // If we have enough words and we can end, we terminate
        if ( words.size() >= min_words && iter->second.end )
            break;
        prefix.shift(words.back());
    }
}


TextGenerator::Prefix TextGenerator::walk_back(
    std::size_t max_words,
    std::vector<std::string>& words) const
{
    // Nothing to do
    if ( words.empty() )
        return Prefix(prefix_size);

    // Keep going back until we hit the word limit
    while ( words.size() < max_words )
    {
        std::vector<Chain::const_iterator> prefixes;

        // single_word is for when we are only supposed to check one word in lookback
        bool single_word = words.size() == 1 || prefix_size == 1;

        // Search through the map for matching transitions
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
        // No match, bail out
        if ( prefixes.empty() )
            break;

        // Select a random match
        auto random_iter = prefixes[math::random(prefixes.size() - 1)];

        // If it looks like an ending point, exit
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
        // Skip empties
        auto prefix_begin = std::find_if(random_prefix.begin(), random_prefix.end(),
            [](const std::string& str) { return !str.empty(); });
        // Append the prefix
        words.insert(words.begin(), prefix_begin, prefix_end);

        // Starting prefix, can exit
        if ( random_prefix.front().empty() )
            break;
    }
    // Determine the prefix obtained from the end of words
    Prefix last_prefix(prefix_size);
    std::size_t copy_size = math::min(prefix_size, words.size());
    for ( std::size_t i = 0; i < copy_size; i++ )
        last_prefix[prefix_size - copy_size + i] = words[words.size() - copy_size + i];
    return last_prefix;
}

} // namespace string
} // namespace melanolib
