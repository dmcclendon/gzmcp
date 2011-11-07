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
# gzmcpc::graphics: visual output rendering
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_GRAPHICS_H
#define _NDS_CLIENT_GRAPHICS_H




#define MODEL_XMIN 0
#define MODEL_XMAX (v16)(4 << 8)
#define MODEL_YMIN 0
#define MODEL_YMAX (v16)(3 << 8)
#define MODEL_ZMIN floattov16(-1.0f)
#define MODEL_ZMAX floattov16(1.0f)


#define COMB_WIDTH 17
#define COMB_HEIGHT 15
#define COMB_XBORDER 0
#define COMB_YBORDER 0




#define DEFAULT_TOP_RENDER_RATE 24

#define DEFAULT_BOTTOM_RENDER_RATE 30

#define MAX_TEXT_OUTPUT_LINE_SIZE 33


#define TS_MODE_ROW 7
#define TS_STATUS_ROW 9
#define TS_NETWORK_STATUS_ROW 11
#define TS_AESTHETIC_ROW 13
#define TS_DEBUG_ROW 14
#define TS_FX_MODE_ROW 15
#define TS_TOUCH_VAL_ROW 17
#define TS_ERROR_ROW 18
#define TS_TOUCH_VALXP_ROW 19
#define TS_TOUCH_VALYP_ROW 21


#define DEFAULT_CURSOR_INTESITY 0.5f
#define DEFAULT_FONT_INTENSITY 27

#define FONT_INTENSITY_MAX 30
#define FONT_INTENSITY_MIN 7


#define CURSOR_RADIUS (v16)((MODEL_XMAX - MODEL_XMIN) / 16)

#define M_PIl 3.1415926535897932384626433832795029L


#ifndef REG_BLDCNT
#define REG_BLDCNT BLEND_CR
#endif

#ifndef REG_BLDCNT_SUB
#define REG_BLDCNT_SUB SUB_BLEND_CR
#endif

#ifndef REG_BLDY
#define REG_BLDY BLEND_Y
#endif

#ifndef REG_BLDY_SUB
#define REG_BLDY_SUB SUB_BLEND_Y
#endif








#include <nds/arm9/videoGL.h>


#include "time.h"





typedef enum {
  TLT_AESTHETIC,
  TLT_MODE,
  TLT_STATUS,
  TLT_TOUCH_VAL,
  TLT_TOUCH_VALXP,
  TLT_TOUCH_VALYP,
  TLT_FX_MODE,
  TLT_ERROR,
  TLT_NETWORK_STATUS,
  TLT_DEBUG,
  TLT_NODEBUG,
  TLT_NUM_TEXT_OUTPUT_LINE_TYPES,
} text_output_line_type;





void gzmcpc_init_3d(void);


void gzmcpc_display_main_splash_text(void);

void textout(text_output_line_type linetype, const char *fmt, ...);

void render_textout(void);

void gzmcpc_render_touchwhammy_vars(void);

void gzmcpc_increment_animations(void);


void init_modelview_matrix(void);

void draw_hex(v16 centerx, v16 centery, v16 z, 
	      v16 radius, float rotation,
	      uint8 centerr, uint8 centerg, uint8 centerb, uint8 centera,
	      uint8 outterr, uint8 outterg, uint8 outterb, uint8 outtera,
	      uint8 rimr, uint8 rimg, uint8 rimb, uint8 rima,
	      u16 intensity);

void draw_simple_hex_grid(int width,
			  int height,
			  u16 intensity);


void gzmcpc_flush_frame(void);



extern char text_output_line[TLT_NUM_TEXT_OUTPUT_LINE_TYPES][MAX_TEXT_OUTPUT_LINE_SIZE];

extern float cursor_rotation;

extern float cursor_intensity;

extern v16 touch_pos_x;
extern v16 touch_pos_y;

extern int cursor_rotating;

extern unsigned char font_intensity;

extern PrintConsole top_screen;
extern PrintConsole bottom_screen;
extern PrintConsole top_screen_x;
extern PrintConsole bottom_screen_x;

extern Keyboard keyboard;

extern float sqrt_three;

extern unsigned char top_render_rate;
extern unsigned char bottom_render_rate;
extern time_val next_top_render;
extern time_val next_bottom_render;

extern int bg0, bg1, bg2, bg3;
extern int bgs0, bgs1, bgs2, bgs3;

#endif // _NDS_CLIENT_GRAPHICS_H
