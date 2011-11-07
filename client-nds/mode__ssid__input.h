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
# gzmcpc::mode__ssid__input: mode__ssid__input header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_MODE__SSID__INPUT_H
#define _NDS_CLIENT_MODE__SSID__INPUT_H



#define SSID__INPUT__TOP_BG_FADE_IN_START_MS 0
#define SSID__INPUT__TOP_BG_FADE_IN_DURATION_MS 1000
#define SSID__INPUT__TOP_BG_HOLD_DURATION_MS 1500
#define SSID__INPUT__TOP_BG_FADE_OUT_DURATION_MS 1000

#define SSID__INPUT__BOT_BG_FADE_IN_START_MS 0
#define SSID__INPUT__BOT_BG_FADE_IN_DURATION_MS 1000
#define SSID__INPUT__BOT_BG_HOLD_DURATION_MS 1500
#define SSID__INPUT__BOT_BG_FADE_OUT_DURATION_MS 1000

#define SSID__INPUT__BOT_TXT_FADE_IN_START_MS 1000
#define SSID__INPUT__BOT_TXT_FADE_IN_DURATION_MS 500
#define SSID__INPUT__BOT_TXT_HOLD_DURATION_MS 500
#define SSID__INPUT__BOT_TXT_FADE_OUT_DURATION_MS 500












void mode__ssid__input___init(void);
void mode__ssid__input___top_renderer(void);
void mode__ssid__input___bot_renderer(void);
void mode__ssid__input___input_handler(void);
void mode__ssid__input___idle(void);
void mode__ssid__input___exit(void);






#endif // _NDS_CLIENT_MODE__SSID__INPUT_H
