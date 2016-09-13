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
#ifndef MELANOLIB_ID_POOL_HPP
#define MELANOLIB_ID_POOL_HPP

#include <cstddef>
#include <limits>
#include "melanolib/utils/c++-compat.hpp"

namespace melanolib {

template<class IdType, IdType Max = std::numeric_limits<IdType>::max()>
class BasicIdPool
{
public:
    using Id = IdType;
    static constexpr IdType max = Max;

    BasicIdPool() = default;

    BasicIdPool(BasicIdPool&& oth)
        : first(0, oth.first.last, std::move(oth.first.next))
    {}

    BasicIdPool& operator=(BasicIdPool&& oth)
    {
        std::swap(first.last, oth.first.last);
        std::swap(first.next, oth.first.next);
    }

    Id get_id()
    {
        for ( IdSegment* segment = &first; segment; segment = segment->next.get() )
        {
            if ( segment->last < max )
            {
                segment->last += 1;
                Id id = segment->last;
                segment->check_merge();
                return id;
            }
        }
        return 0;
    }

    void mark_id(Id id)
    {
        IdSegment* previous = &first;
        for ( IdSegment* segment = &first; segment; segment = segment->next.get() )
        {
            if ( id < segment->first )
            {
                if ( id + 1 == segment->first )
                {
                    segment->first -= 1;
                    return;
                }
                break;
            }
            else if ( id >= segment->first && id <= segment->last )
            {
                return;
            }
            else if ( id > segment->last && segment->last + 1 == id )
            {
                segment->last += 1;
                segment->check_merge();
                return;
            }

            previous = segment;
        }

        previous->next = New<IdSegment>(id, id, std::move(previous->next));
    }

private:
    struct IdSegment
    {
        IdSegment(Id first, Id last, std::unique_ptr<IdSegment> next = {})
            : first(first),
            last(last),
            next(std::move(next))
        {}

        Id first;
        Id last;
        std::unique_ptr<IdSegment> next;

        void check_merge()
        {
            while ( next && last >= next->first - 1 )
            {
                last = next->last;
                next = std::move(next->next);
            }
        }
    };

    IdSegment first = IdSegment(0, 0);
};

using IdPool = BasicIdPool<std::size_t>;

} // namespace melanolib
#endif // MELANOLIB_ID_POOL_HPP
