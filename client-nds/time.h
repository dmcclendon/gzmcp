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
# gzmcpc (nds.arm9): time header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_TIME_H
#define _NDS_CLIENT_TIME_H











typedef struct time_val_tag {
  long s;
  long ms;
} time_val;




int time_val_compare(time_val time_a, time_val time_b);

time_val time_val_add_ms(time_val time_a, int ms);

long ms_since(time_val time);

void gzmcpc_init_time(void);

void ms_timer_handler(void);




extern char timer_num_ticks;

extern time_val num_ticks;


#endif // _NDS_CLIENT_TIME_H
