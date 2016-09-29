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
 *
 */
#ifndef MELANOLIB_STRING_ENCODING_HPP
#define MELANOLIB_STRING_ENCODING_HPP

#include "melanolib/string/quickstream.hpp"

namespace melanolib {
namespace string {

/**
 * \brief Unicode point
 */
class Unicode
{
public:
    Unicode(std::string utf8, uint32_t point)
        : utf8_(std::move(utf8)), point_(point) {}

    explicit Unicode(uint32_t point);

    /**
     * \brief Returns the UTF-8 representation
     */
    const std::string& utf8() const { return utf8_; }
    /**
     * \brief Returns the Unicode code point
     */
    uint32_t point() const { return point_; }

    bool is_ascii() const { return point_ < 128; }

    bool valid() const { return !utf8_.empty(); }

private:
    std::string utf8_;
    uint32_t    point_;
};

/**
 * \brief Class used to parse and convert UTF-8
 */
class Utf8Parser
{
public:
    using Byte = uint8_t;

    std::function<void(uint8_t)>                     callback_ascii;
    std::function<void(uint32_t, const std::string&)>callback_utf8;
    std::function<void(const std::string&)>          callback_invalid;
    std::function<void()>                            callback_end;

    melanolib::string::QuickStream input;

    /**
     * \brief Parse the string completely, calling the right callbacks
     */
    void parse(const std::string& string);

    /**
     * \brief Whether the end of the string has been reached
     */
    bool finished() const { return !input; }


    /**
     * \brief Start manual parsing
     */
    void start_parsing(const std::string& string);

    /**
     * \brief Get the next unicode point
     * \pre !finished()
     */
    Unicode next();

    /**
     * \brief Encode a unicode value to UTF-8
     */
    static std::string encode(uint32_t value);

    /**
     * \brief Whether a byte is a valid ascii character
     */
    static bool is_ascii(Byte b) { return b < 128; }

    /**
     * \brief Transliterate a single character to ascii
     */
    static char to_ascii(uint32_t unicode);
    /**
     * \brief Transliterate a single character to ascii
     */
    static char to_ascii(const std::string& utf8_char);

    /**
     * \brief Whether the library supports fancy character conversions
     */
    static bool has_iconv();

private:
    enum class ByteType
    {
        ASCII,
        MultiHead,
        MultiTail,
    };

    std::string           utf8;         ///< Multibyte string
    uint32_t              unicode;      ///< Multibyte value
    unsigned              length = 0;   ///< Multibyte length

    /**
     * \brief Handles an invalid/incomplete sequence
     */
    void check_valid();

    static constexpr ByteType byte_type(uint8_t byte)
    {
        byte >>= 6;
        if ( (byte & 0b10) == 0 )
            return Utf8Parser::ByteType::ASCII;
        if ( (byte & 0b11) == 0b11 )
            return Utf8Parser::ByteType::MultiHead;
        return Utf8Parser::ByteType::MultiTail;
    }

    static constexpr uint8_t tail_value(uint8_t byte)
    {
        return byte & 0b0011'1111;
    }

    static constexpr std::pair<unsigned, uint32_t> head_length_value(uint8_t byte)
    {
        unsigned length = 0;
        // extract number of leading 1s
        while ( byte & 0b1000'0000 )
        {
            length++;
            byte <<= 1;
        }

        // Restore byte (leading 1s have been eaten off)
        return {length, byte >> length};
    }
};


} // namespace string
} // namespace melanolib
#endif // MELANOLIB_STRING_ENCODING_HPP
