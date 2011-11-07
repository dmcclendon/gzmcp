/*
 *  Guitar-ZyX(tm)::MasterControlProgram - portable guitar F/X controller
 *  Copyright (C) 2009  Douglas McClendon
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CARD_H
#define CARD_H

#include "disc_io.h"
#include "io_dldi.h"

static inline bool CARD_StartUp (void)
{
	return _io_dldi.fn_startup();
}

static inline bool CARD_IsInserted (void)
{
	return _io_dldi.fn_isInserted();
}

static inline bool CARD_ReadSector (u32 sector, void *buffer)
{
	return _io_dldi.fn_readSectors(sector, 1, buffer);
}

#endif // CARD_H
