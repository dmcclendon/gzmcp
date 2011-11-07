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
## 
## gzmcp::libmcp: the mcp's meta-ds library (how I wish libnds would look)
##
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/




#include <nds.h>

#include <nds/arm9/console.h>

#include <string.h>

#include <stdlib.h>

#include <stdio.h>

#include <sys/iosupport.h>

#include "debug.h"

#include "mcp.h"




extern bool bgIsTextLut[8];
extern bool checkIfText(int id);
extern ssize_t con_write(struct _reent *, 
			 int,
			 const char *,
			 size_t);
extern PrintConsole *currentConsole;
extern PrintConsole defaultConsole;
extern void consoleLoadFont(PrintConsole *);
extern void consolePrintChar(char);





int mcp_blend_level_main = MCP_MAX_BLEND_LEVEL;
int mcp_blend_level_sub = MCP_MAX_BLEND_LEVEL;

static const devoptab_t dotab_stdout = {
  "con",
  0,
  NULL,
  NULL,
  con_write,
  NULL,
  NULL,
  NULL,
};  

u16 fadestate[8];
void (*fadestate_fire_func[8])(void);







void mcp_basename(char *dest, 
		  char *path) {

  char *new_result;
  char *last_result;


  // look for the last '/'
  last_result = path;
  while ((new_result = strstr(last_result, "/")) != NULL) {
    last_result = new_result + 1;
  }

  // copy to dest, everything after the last '/'
  strncpy(dest, last_result, PATH_MAXLEN);

  // guarantee null terminated destination string
  dest[PATH_MAXLEN] = 0;

  return;
}



void mcp_dirname(char *dest, 
		 char *path) {

  char *new_result;
  char *last_result;
  int dirname_length;

  char *trimmed_path;


  // first check for dirname('/') = '/'
  if (strncmp(path, "/", 2) == 0) {
    strncpy(dest, path, 2);
    return;
  }

  if ((trimmed_path = malloc(PATH_MAXLEN)) == NULL) die();

  strncpy(trimmed_path, path, PATH_MAXLEN);
  if (trimmed_path[strlen(trimmed_path) - 1] == '/') {
    
    trimmed_path[strlen(trimmed_path) - 1] = '\0';
  }

  // look for the last '/'
  last_result = trimmed_path;
  while ((new_result = strstr(last_result, "/")) != NULL) {
    last_result = new_result + 1;
  }

  // if no / found, dirname is .
  if (last_result == trimmed_path) {
    // edunote: normally I go with n of strlen(string), but this from the
    //          strncpy manpage worries me-
    // "If there is no terminating null byte in the first n characters of 
    //  src, strncpy() produces an unterminated string in dest."
    strncpy(dest, ".", 2);
    free(trimmed_path);
    return;
  }

  // calculdate the length of the result we want
  dirname_length = strlen(trimmed_path) - strlen(last_result) - 1;

  if (dirname_length > PATH_MAXLEN) {
    //    die("dirname() result greater than PATH_MAXLEN - %d\n",
    //	PATH_MAXLEN);
    die();
  }

  // special case of dirname("/XYZ") == "/" instead of ""
  if (!dirname_length) {
    strncpy(dest, "/", 2);
  } else {

    // copy to dest, everything up to last '/'
    strncpy(dest, 
	    trimmed_path, 
	    dirname_length);
    
    // guarantee null terminated destination string
    dest[dirname_length] = 0;
    dest[PATH_MAXLEN] = 0;
    
  }

  free(trimmed_path);
  return;

}



void mcp_set_blend(int main, 
		   int level) {
  if (main) {
    mcp_blend_level_main = level;
    REG_BLDY = mcp_blend_level_main;
  } else {
  mcp_blend_level_sub = level;
  REG_BLDY_SUB = mcp_blend_level_sub;
  }
}
int mcp_get_blend(int main) {
  if (main) {
    return mcp_blend_level_main;
  } else {
    return mcp_blend_level_sub;
  }
}



int mcp_bg_init(int main,
		int layer,
		int hidden,
		BgType bg_type,
		BgSize bg_size,
		int map_base,
		int tile_base) {

  int layer_id;

  vu16 *bg_control_reg;


  // convert layer and main/sub to compact layer_id encoding
  layer_id = MCP_LAYER_ID(main, layer);

  // get a pointer to the appropriate control register
  bg_control_reg = (main ? &(BGCTRL[layer]) : &(BGCTRL_SUB[layer]));

  // will this compile?? (not)
  *bg_control_reg = (BG_MAP_BASE(map_base) |
		     BG_TILE_BASE(tile_base) |
		     bg_size |
		     ((bg_type == BgType_Text8bpp) ? BG_COLOR_256 : 0));

  // initialize global state structure array
  memset(&bgState[layer_id], 
	 0, 
	 sizeof(BgState));

  // initialize rotate and scale if a bitmap
  if ((bg_type != BgType_Text4bpp) && 
      (bg_type != BgType_Text8bpp)) {
    bgSetScale(layer_id,
	       intToFixed(1, 8),
	       intToFixed(1, 8));
    bgRotate(layer_id, 0);
  }

  // update global state structure array
  bgState[layer_id].type = bg_type;
  bgState[layer_id].size = bg_size;

  // *DON'T* show(unhide) layer automatically
  //  if (main) {
  //    videoBgEnable(layer);
  //  } else {
  //    videoBgEnableSub(layer);
  //  }

  // update more global state
  bgIsTextLut[layer_id] = checkIfText(layer_id);

  // set layer state to dirty
  bgState[layer_id].dirty = true;

  // things are set up, let bgUpdate process the new state
  bgUpdate();

  // be seeing you...
  return layer_id;

}



void mcp_bg_show(int main,
		 int layer) {
  bgShow(MCP_LAYER_ID(main, layer));
}

void mcp_bg_hide(int main,
		 int layer) {
  bgHide(MCP_LAYER_ID(main, layer));
}



void mcp_console_init(PrintConsole *pconsole,
		      int main,
		      int layer,
		      int hidden,
		      int loadfont,
		      BgType bg_type,
		      BgSize bg_size,
		      int map_base,
		      int tile_base) {

  static int do_first_init = 1;
  int i;

  
  if (do_first_init) {
    devoptab_list[STD_OUT] = &dotab_stdout;
    setvbuf(stdout,
	    NULL,
	    _IONBF,
	    0);
    do_first_init = 0;
  }

  currentConsole = pconsole;

  // initialize the console struct
  // note: that the contents of console prior thus do not matter
  *currentConsole = defaultConsole;

  // initialize the background layer
  pconsole->bgId = MCP_LAYER_ID(main, layer);
  mcp_bg_init(main, 
	      layer,
	      1,
	      bg_type,
	      bg_size,
	      map_base,
	      tile_base);

  // set console gfx pointers
  pconsole->fontBgMap = (u16*)bgGetMapPtr(pconsole->bgId);
  pconsole->fontBgGfx = (u16*)bgGetGfxPtr(pconsole->bgId);

  pconsole->consoleInitialised = 1;

  // clear the console
  pconsole->cursorX = 0;
  pconsole->cursorY = 0;
  for (i = 0; i < pconsole->windowHeight * pconsole->windowWidth; i++) {
    consolePrintChar(' ');
  }

  // load the font if requested
  if (loadfont) consoleLoadFont(pconsole);

  // show the background layer if requested (that it not be hidden)
  if (!hidden) mcp_bg_show(main, layer);

}



void mcp_delay(int frames) {
  int i;
  for (i = 0 ; i < frames ; i++) swiWaitForVBlank();
}


void mcp_fade_set_enabled(int screen, 
			  int layer,
			  int fading_enable) {
  // todo? a ^= more readable logic equivalent?
  fadestate[MCP_LAYER_ID(screen, layer)] &= ~(1 << 13);
  fadestate[MCP_LAYER_ID(screen, layer)] |= (fading_enable << 13);
}


int mcp_fade_get_enabled(int screen, 
			 int layer) {
  return ((fadestate[MCP_LAYER_ID(screen, layer)] & 0x2000) >> 13);
}


void mcp_fade_set_level(int screen, 
			int layer,
			int level) {
  fadestate[MCP_LAYER_ID(screen, layer)] &= ~((u16)63 << 0);
  fadestate[MCP_LAYER_ID(screen, layer)] |= (level << 0);
}


int mcp_fade_get_level(int screen, 
		       int layer) {
  return ((fadestate[MCP_LAYER_ID(screen, layer)] & 0x003F) >> 0);
}


void mcp_fade_set_rate(int screen, 
		       int layer,
		       int rate) {
  fadestate[MCP_LAYER_ID(screen, layer)] &= ~((u16)63 << 6);
  fadestate[MCP_LAYER_ID(screen, layer)] |= (rate << 6);
}


int mcp_fade_get_rate(int screen, 
		      int layer) {
  return ((fadestate[MCP_LAYER_ID(screen, layer)] & 0x0FC0) >> 6);
}


void mcp_fade_set_fwd(int screen, 
		      int layer,
		      int fwd) {
  fadestate[MCP_LAYER_ID(screen, layer)] &= ~(1 << 12);
  fadestate[MCP_LAYER_ID(screen, layer)] |= (fwd << 12);
}


int mcp_fade_get_fwd(int screen, 
		     int layer) {
  return ((fadestate[MCP_LAYER_ID(screen, layer)] & 0x1000) >> 12);
}


void mcp_fade_set_func(int screen, 
		       int layer,
		       void (*func)(void)) {
  fadestate_fire_func[MCP_LAYER_ID(screen, layer)] = func;
}


void mcp_fade_fire_func(int screen, 
			int layer) {

  void (*tfunc)(void);


  // get a pointer to the function to fire off
  tfunc = fadestate_fire_func[MCP_LAYER_ID(screen, layer)];
  // reinitialize func to NULL (so perhaps tfunc can set it anew)
  mcp_fade_set_func(screen, layer, NULL);
  // fire off the function
  if (tfunc != NULL) (*tfunc)();

}


void mcp_fade_init(void) {

  int screen;
  int layer;


  for (screen = MCP_SUB_SCREEN ; screen <= MCP_MAIN_SCREEN ; screen++) {
    for (layer = 0; layer < 4 ; layer++) {
      mcp_fade_set_enabled(screen, layer, 0);
      mcp_fade_set_fwd(screen, layer, 0);
      mcp_fade_set_rate(screen, layer, 0);
      mcp_fade_set_level(screen, layer, 0);
      mcp_fade_set_func(screen, layer, NULL);
    }
  }

}


void mcp_fade_start(int screen,
		    int layer,
		    int fwd,
		    int duration_ms,
		    int update_period_hz,
		    void (*func)(void)) {

  mcp_fade_set_enabled(screen, layer, 1);
  mcp_fade_set_fwd(screen, layer, fwd);
  mcp_fade_set_func(screen, layer, func);
  mcp_fade_set_rate(screen, layer, 
		    (MCP_FADE_MAXLEVEL /
		     (duration_ms /
		      (1000 / (update_period_hz / MCP_FADE_THROTTLE)))));
}


void mcp_fade_update(void) {
  
  int screen;
  int layer;

  static int fade_throttle = 0;


  fade_throttle++;
  if (fade_throttle % MCP_FADE_THROTTLE) {
    return;
  }

  for (screen = MCP_SUB_SCREEN ; screen <= MCP_MAIN_SCREEN ; screen++) {
    for (layer = 0; layer < 4 ; layer++) {
      if (mcp_fade_get_enabled(screen, layer)) {
	if (mcp_fade_get_fwd(screen, layer)) {
	  // fading in
	  if (mcp_fade_get_level(screen, layer) == MCP_FADE_MAXLEVEL) {
	    // disable first, in case fired func wants to start a new fade
	    mcp_fade_set_enabled(screen, layer, 0);
	    mcp_fade_fire_func(screen, layer);
	  } else if ((mcp_fade_get_level(screen, layer) + 1 + 
		      mcp_fade_get_rate(screen, layer)) >=
	      MCP_FADE_MAXLEVEL) {
	    // finished fading in
	    mcp_fade_set_level(screen, layer,
			   MCP_FADE_MAXLEVEL);
	  } else {
	    // fade in progress
	    // note: +1 ensures that a level of 0 is still progressing
	    mcp_fade_set_level(screen, layer,
			   (mcp_fade_get_level(screen, layer) + 1 +
			    mcp_fade_get_rate(screen, layer)));
	  }
	} else {
	  // fading out
	  if (mcp_fade_get_level(screen, layer) == 0) {
	    // disable first, in case fired func wants to start a new fade
	    mcp_fade_set_enabled(screen, layer, 0);
	    mcp_fade_fire_func(screen, layer);
	  } else if ((mcp_fade_get_level(screen, layer) - 1
		      - mcp_fade_get_rate(screen, layer)) <= 
	      0) {
	    // finished fading in
	    mcp_fade_set_level(screen, layer, 0);
	  } else {
	    // in progress
	    mcp_fade_set_level(screen, layer,
			   (mcp_fade_get_level(screen, layer) - 1 -
			    mcp_fade_get_rate(screen, layer)));
	  }
	} // end if fading in/else/out 
      } // end if fading
    } // end layer iteration
  } // end screen iteration

}



  
