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
# gzmcpc::template: template header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _LIB_MCP_H
#define _LIB_MCP_H



#define PATH_MAXLEN 1023

#define MCP_MAIN_SCREEN 1
#define MCP_SUB_SCREEN 0

#define MCP_BG_HIDE 1
#define MCP_BG_SHOW 0

#define MCP_MAX_BLEND_LEVEL 31
#define MCP_MIN_BLEND_LEVEL 0

#define MCP_LAYER_ID(screen, layer) (screen ? layer : (layer + 4))

#define RGB15_TO_R5(color) (color & 0x001f)
#define RGB15_TO_G5(color) ((color & 0x03e0) >> 5)
#define RGB15_TO_B5(color) ((color & 0x7c00) >> 10)
#define RGB15_TO_A1(color) ((color & 0x8000) >> 15)

#define MCP_FADE_MAXLEVEL 63
#define MCP_FADE_THROTTLE 4













void mcp_basename(char *dest, 
		  char *path);
void mcp_dirname(char *dest, 
		 char *path);

void mcp_set_blend(int main,
		   int level);
int mcp_get_blend(int main);

int mcp_bg_init(int main,
		int layer,
		int hidden,
		BgType bg_type,
		BgSize bg_size,
		int map_base,
		int tile_base);

void mcp_bg_show(int main,
		 int layer);
void mcp_bg_hide(int main,
		 int layer);


void mcp_console_init(PrintConsole *pconsole,
		      int main,
		      int layer,
		      int hidden,
		      int loadfont,
		      BgType bg_type,
		      BgSize bg_size,
		      int map_base,
		      int tile_base);

void mcp_delay(int frames);
		      

void mcp_fade_set_enabled(int screen, 
			  int layer,
			  int fading_enable);
int mcp_fade_get_enabled(int screen, 
			 int layer);

void mcp_fade_set_level(int screen, 
			int layer, 
			int level);
int mcp_fade_get_level(int screen, 
		       int layer);

void mcp_fade_set_rate(int screen, 
		       int layer, 
		       int level);
int mcp_fade_get_rate(int screen, 
		      int layer);

void mcp_fade_set_fwd(int screen, 
		      int layer,
		      int fwd);
int mcp_fade_get_fwd(int screen, 
		     int layer);

void mcp_fade_set_func(int screen,
		       int layer,
		       void (*func)(void));
void mcp_fade_fire_func(int screen,
			int layer);

void mcp_fade_init(void);

void mcp_fade_start(int screen,
		    int layer,
		    int fwd,
		    int duration_ms,
		    int update_period_hz,
		    void (*func)(void));

void mcp_fade_update(void);





#endif // _LIB_MCP_H
