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
# gzmcpc::mode__intro__main: mode__intro__main header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_MODE__INTRO__MAIN_H
#define _NDS_CLIENT_MODE__INTRO__MAIN_H



#define INTRO__MAIN__TOP_BG_FADE_IN_START_MS 224
#define INTRO__MAIN__TOP_BG_FADE_IN_DURATION_MS 2000
#define INTRO__MAIN__TOP_BG_HOLD_DURATION_MS 3000
#define INTRO__MAIN__TOP_BG_FADE_OUT_DURATION_MS 2000

#define INTRO__MAIN__BOT_BG_FADE_IN_START_MS 224
#define INTRO__MAIN__BOT_BG_FADE_IN_DURATION_MS 2000
#define INTRO__MAIN__BOT_BG_HOLD_DURATION_MS 3000
#define INTRO__MAIN__BOT_BG_FADE_OUT_DURATION_MS 2000

#define INTRO__MAIN__BOT_TXT_FADE_IN_START_MS 2224
#define INTRO__MAIN__BOT_TXT_FADE_IN_DURATION_MS 1000
#define INTRO__MAIN__BOT_TXT_HOLD_DURATION_MS 2000
#define INTRO__MAIN__BOT_TXT_FADE_OUT_DURATION_MS 1000












void mode__intro__main___init(void);
void mode__intro__main___top_renderer(void);
void mode__intro__main___bot_renderer(void);
void mode__intro__main___input_handler(void);
void mode__intro__main___idle(void);
void mode__intro__main___exit(void);





#endif // _NDS_CLIENT_MODE__INTRO__MAIN_H
