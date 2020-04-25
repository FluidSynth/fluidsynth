/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2019  Tom Moebert and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "fluid_seqbind_notes.h"

#include <set>

typedef std::set<fluid_note_id_t> note_container_t;

fluid_note_id_t compute_id(int chan, short key)
{
    return 128 * chan + key;
}

void* new_fluid_note_container()
{
    try
    {
        note_container_t* cont = new note_container_t;
        return cont;
    }
    catch(...)
    {
        return 0;
    }
}

void delete_fluid_note_container(void *cont)
{
    delete static_cast<note_container_t*>(cont);
}

int fluid_note_container_insert(void* cont, fluid_note_id_t id)
{
    try
    {
        std::pair<note_container_t::iterator, bool> res = static_cast<note_container_t*>(cont)->insert(id);
        // res.second tells us whether the element was inserted
        // by inverting it, we know whether it contained the element previously
        return !res.second;
    }
    catch(...)
    {
        return FLUID_FAILED;
    }
}

void fluid_note_container_remove(void* cont, fluid_note_id_t id)
{
    try
    {
        static_cast<note_container_t*>(cont)->erase(id);
    }
    catch(...)
    {
        // should never happen
    }
}

// empties the entire collection, e.g. in case of a AllNotesOff event
void fluid_note_container_clear(void* cont)
{
    static_cast<note_container_t*>(cont)->clear();
}
