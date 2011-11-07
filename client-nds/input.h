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
/*
#############################################################################
#############################################################################
# 
# gzmcpc::input: input header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_INPUT_H
#define _NDS_CLIENT_INPUT_H



#define DEFAULT_INPUT_POLL_RATE 60




#include <nds.h>


#include "time.h"




typedef enum {
  IH_NONE,
  IH_METROGNOME_INC,
  IH_METROGNOME_DEC,
  IH_WPX_INC,
  IH_WPX_DEC,
  IH_WPY_INC,
  IH_WPY_DEC,
  IH_PRESET_INC,
  IH_PRESET_DEC,
} input_held_type;




void gzmcpc_poll_input(void);

void global_input_processor(void);



extern touchPosition touchdata;
extern u16 heldkeys;
extern u16 upkeys;
extern u16 downkeys;

extern input_held_type heldover;
extern time_val heldover_sunset;

extern unsigned char input_poll_rate;
extern time_val next_input_poll;


#endif // _NDS_CLIENT_INPUT_H
