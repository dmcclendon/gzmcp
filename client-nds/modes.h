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
# gzmcpc::modes: modes header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_MODES_H
#define _NDS_CLIENT_MODES_H







#include "time.h"



typedef struct mode_def_tag {
  // a little fun
  void (*hello)(void);
  void (*icandy_top_reindeer)(void);
  void (*icandy_bot_reindeer)(void);
  void (*input_processor)(void);
  void (*slacker_funk)(void);
  void (*goodbye)(void);
} mode_def;


typedef enum {
  MODE_INTRO__MAIN,
  MODE_INTRO__CREDITS,
  MODE_MAIN_MENU,
  MODE_GET_UPDATE,
  MODE_SSID__INPUT,
  MODE_TPW__JAM,
  //  SERVERCONSOLE,
  //  SCROLLMENU,
  // 1st use, ssid input, use/displaystrings defined with globals
  //  STRINGINPUT,
  // single slider, 
  //   with miniscroll numerical inputs 
  //   (or slider w/dragable tikmarks) 
  //   for min and max
  //   and reverse toggle
  //  SLIDERINPUT
  // one for each rakmod
  //  ONOFFINPUT
  // 10/18 radio buttons
  //  ACTIVERAKMODS
  //  RECCONTROL
  //  TUNERFEEDBACK
  MODE_NUM_MODES,
} gzmcp_mode;




void system_xmode_new(gzmcp_mode next_mode);

void system_xmode_reinit(void);

void system_xmode_real(void);

void got_no_funk(void);




extern gzmcp_mode last_mode;
extern gzmcp_mode mode;
extern gzmcp_mode next_mode;
extern gzmcp_mode new_mode;

extern mode_def modes[];

extern time_val mode_start;

extern long mode_ms;
extern long exit_mode_ms;
#endif // _NDS_CLIENT_MODES_H
