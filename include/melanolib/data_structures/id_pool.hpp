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

/**
 * \brief Class used to generate locally unique incremental ids
 * \tparam IdType An integral type
 * \tparam Max Maximum allowed value
 *
 * The generated ids will be in [1, max]. 0 is used as a special value.
 */
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

    /**
     * \brief Allocates the lowest available id.
     * \returns A valid id or 0 if they are all allocated
     */
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

    /**
     * \brief Marks \p id as allocated
     */
    void mark_id(Id id)
    {
        if ( id <= 0 || id > max )
            return;

        IdSegment* previous = &first;
        for ( IdSegment* segment = &first; segment; segment = segment->next.get() )
        {
            if ( id < segment->first )
            {
                if ( id + 1 == segment->first )
                {
                    // The id falls immediatly before the start of the segment
                    // Note: we don't need to merge because the id is not the
                    //       id following the last of the previous segment
                    //       (Otherwise it would already been handled)
                    segment->first -= 1;
                    return;
                }
                // else, falls somewhere between this and the previous segment,
                // which is handled outside the loop
                break;
            }
            else if ( id >= segment->first && id <= segment->last )
            {
                // Falls inside an already allocated segment, nothing to do
                return;
            }
            else if ( id > segment->last && segment->last + 1 == id )
            {
                // The id falls immediatly after an allocated segment,
                // so we need to expand it and possibly merge it
                segment->last += 1;
                segment->check_merge();
                return;
            }

            previous = segment;
        }

        // The id falls between segments or after the last segment
        // Either way, we shall create a new segment and add it on the right
        // position in the list
        // Note: "previous" is always the latest segment with "last" less than id
        previous->next = New<IdSegment>(id, id, std::move(previous->next));
    }

private:
    /**
     * \brief Linked list node representing a range of allocated ids
     * \invariant first <= last && last <= max && (!next || next->first > last + 1)
     */
    struct IdSegment
    {
        IdSegment(Id first, Id last, std::unique_ptr<IdSegment> next = {})
            : first(first),
            last(last),
            next(std::move(next))
        {}

        Id first; ///< First allocated id in the segment
        Id last; ///< Last allocated id in the segment
        std::unique_ptr<IdSegment> next; ///< Next node in the list

        /**
         * \brief Merges with following list nodes to preserve the class invariant
         */
        void check_merge()
        {
            while ( next && last >= next->first - 1 )
            {
                last = next->last;
                next = std::move(next->next);
            }
        }
    };

    IdSegment first = IdSegment(0, 0); ///< Initial node, starts allocating the invalid value 0
};

using IdPool = BasicIdPool<std::size_t>;

} // namespace melanolib
#endif // MELANOLIB_ID_POOL_HPP
