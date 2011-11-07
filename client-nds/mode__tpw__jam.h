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
# gzmcpc::mode__tpw__jam: mode__tpw__jam header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_MODE__TPW__JAM_H
#define _NDS_CLIENT_MODE__TPW__JAM_H




#define TPW__JAM__TOP_BG_FADE_IN_START_MS 224
#define TPW__JAM__TOP_BG_FADE_IN_DURATION_MS 2000
#define TPW__JAM__TOP_BG_FADE_OUT_DURATION_MS 2000

#define TPW__JAM__BOT_BG_FADE_IN_START_MS 224
#define TPW__JAM__BOT_BG_FADE_IN_DURATION_MS 4000
#define TPW__JAM__BOT_BG_FADE_OUT_DURATION_MS 2000

#define TPW__JAM__BOT_TXT_FADE_IN_START_MS 2224
#define TPW__JAM__BOT_TXT_FADE_IN_DURATION_MS 6996
#define TPW__JAM__BOT_TXT_FADE_OUT_DURATION_MS 1000

#define TPW__JAM__BOT_3D_FADE_IN_START_MS 500
#define TPW__JAM__BOT_3D_FADE_IN_DURATION_MS 6996
#define TPW__JAM__BOT_3D_FADE_OUT_DURATION_MS 1000

#define TPW_3D_MAX_INTENSITY 255

#define INTRO_ANIM_3DSPINZOOM_START 500
#define INTRO_ANIM_3DSPINZOOM_DURATION 9000
#define INTRO_ANIM_3DSPINZOOM_SCALE_FACTOR 42


#define DEFAULT_X_PARM 12

#define DEFAULT_Y_PARM 7



#define CM_PRESET_A_NUM 10


#define CM_PRESET_X_NUM 76

#define CM_PRESET_B_NUM 35


#define CM_PRESET_Y_NUM 59

#define VSTROBE_DEFAULT_BPM 240



#include <nds.h>

#include "graphics.h"







void mode__tpw__jam___init(void);
void mode__tpw__jam___top_renderer(void);
void mode__tpw__jam___bot_renderer(void);
void mode__tpw__jam___input_handler(void);
void mode__tpw__jam___idle(void);
void mode__tpw__jam___exit(void);




extern int touch_whammy_enabled;
extern int touch_whammy_x_midi_parm;
extern int touch_whammy_y_midi_parm;

extern int current_preset;

extern int button_x_preset_num;
extern int button_y_preset_num;
extern int button_a_preset_num;
extern int button_b_preset_num;


extern uint8 comb_charge[COMB_WIDTH][COMB_HEIGHT];

extern unsigned char x3d_intensity;

extern v16 model_xcenter;
extern v16 model_ycenter;
extern v16 model_zcenter;
extern v16 model_width;
extern v16 model_height;
extern v16 model_depth;

extern int vstrobe_bpm;
extern int vstrobe_enabled;
extern int user_vstrobe_enabled;


#endif // _NDS_CLIENT_MODE__TPW__JAM_H
