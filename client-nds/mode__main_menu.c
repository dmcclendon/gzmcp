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
## gzmcpc::mode__main_menu: the main menu(/command-tree) system
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


#include <nds/bios.h>

#include <stdio.h>

#include <string.h>

#include <dirent.h>


#include "cloader/arm9_loader.h"

#include "dmc.h"

#include "debug.h"

#include "graphics.h"

#include "input.h"

#include "main.h"

#include "mcp.h"

#include "modes.h"

#include "mode__intro__main.h"

#include "mode__get_update.h"

#include "mode__main_menu.h"

#include "network.h"

#include "sound.h"


#include "sounds.h"

#include "sounds_bin.h"

#include "resources/bitmaps/guitar-zyx.splash.main.h"

#include "resources/bitmaps/dlava.h"

#include "resources/bitmaps/mcpfont.h"





gzmcp_menu_node *mm_list = NULL;
gzmcp_menu_node *mm_hist = NULL;



ConsoleFont mcpfont;

char pwd[FS_PATH_MAXLEN + 1];
char t_scratch_path[FS_PATH_MAXLEN + 1];

int selection_index = 0;
int num_selections = 0;

int hilite_offset = 0;

int mm_doublemode = 0;
int mm_dm_height_lines = 0;
int mm_dm_scroll_pad = 0;
int mm_dm_repeat_rate = 0;
int mm_dm_width = 0;
int mm_dm_xscl = 0;
int mm_dm_yscl = 0;


int mm_text_offset_target = 0;
int mm_text_offset_actual = 0;
int mm_hilite_offset_relative = 0;

u16 mm_txt_fontcolor = MM_TXT_DEFAULTCOLOR;
u16 mm_hilitecolor = MM_HILITE_DEFAULTCOLOR;

int mm_txt_firstfade_done;
int mm_txt_render_hilite;





void mode__main_menu___init(void) {


  // initialize fading system
  mcp_fade_init();

  // initialize main screen
  videoSetMode(MODE_5_2D);

  // map main screen background fourth (128k) region to vram bank A
  vramSetBankA(VRAM_A_MAIN_BG_0x06060000);
  
  // set the secondary/sub screen for text and a background
  videoSetModeSub(MODE_5_2D);

  // map sub screen background (only? 1/4?) to vram bank C
  vramSetBankC(VRAM_C_SUB_BG);

  // unfaded
  mcp_set_blend(MCP_MAIN_SCREEN,
		MCP_MAX_BLEND_LEVEL);
  mcp_set_blend(MCP_SUB_SCREEN,
		MCP_MAX_BLEND_LEVEL);

  // fade the mainscreen background to/from black, layer 3
  REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG3;
  // fade the lava background to/from black, layer 2
  REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG2;

  // visible fonts
  BG_PALETTE[255] = RGB15(0, 0, 0);
  BG_PALETTE_SUB[255] = RGB15(0, 0, 0);

  // init subscreen layer/background 3 
  // the mapbase offset of 24 here means 24*16k which means utilizing
  // the 4th of the possible main background memory regions that vram
  // bank A can be mapped to.  I.e. above we mapped to the 4th.  Had
  // we mapped to the 1st, we would have used offset 0.  
  // note: vram bank A is 128k, i.e. 8 * 16k.
  // note: *16k is because of bitmap type, else would be *2k
  //  bg3 = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 24, 0);
  bg3 = mcp_bg_init(MCP_MAIN_SCREEN,
		    3, 
		    MCP_BG_HIDE,
		    BgType_Bmp16, 
		    BgSize_B16_256x256, 
		    24, 
		    0);
  // its initial priority, lowest (to emphasize lack of other enabled layers)
  // priorities 0..3, 0 highest priority
  bgSetPriority(bg3, 3);

  // load main splash screen into screen/background memory (bgs3)
  // note: if I wanted to do this quickly/perfectly/doublebufferred somehow
  //       I'm not sure if I'd need to tweak the bgInitSub offset or pointer
  //       here or some such.  As it is, the fade to black ensures no tear
  //       as this new image gets loaded into display memory.
  decompress(guitar_zyx_splash_mainBitmap, 
	     (u16*)bgGetGfxPtr(bg3),  
	     LZ77Vram);
  
  bgShow(bg3);

  // note: using offset=4, because 4 will be 64k offset, where 31 above is 62k
  // (thus above using only 2k? seems plausible with tiles for console text chars
  //  bgs2 = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 4, 0);
  bgs2 = mcp_bg_init(MCP_SUB_SCREEN,
		     2,
		     MCP_BG_HIDE,
		     BgType_Bmp8, 
		     BgSize_B8_256x256, 
		     4, 
		     0);
  // its initial priority, lowest (to emphasize lack of other enabled layers)
  // priorities 0..3, 0 highest priority
  bgSetPriority(bgs2, 2);

  // as per libnds doc on dma
  DC_FlushRange(MM_BG_BMP, 256*256);
  dmaCopy(MM_BG_BMP, bgGetGfxPtr(bgs2), 256*256);
  DC_FlushRange(MM_BG_PAL, 256*2);
  dmaCopy(MM_BG_PAL, BG_PALETTE_SUB, 256*2);
  
  bgShow(bgs2);

  // edunote: consoleInit/mcp_console_init overwrite completely 
  //          the PrintConsole arg

  // was 31, 0 for normal non 8bpp font, 
  // 20, 0 for 8bpp font
  // ,1 is to to avoid corruption

  // note: no need to load graphics
  mcp_console_init(&bottom_screen,
		   MCP_SUB_SCREEN,
		   3,
		   1,
		   1,
		   BgType_ExRotation,
		   BgSize_ER_256x256,
		   31, 
		   1);

  // set printf sink
  consoleSelect(&bottom_screen);

  // custom 8bpp font
  mcpfont.asciiOffset = 32;
  mcpfont.bpp = 8;
  mcpfont.convertSingleColor = false;
  mcpfont.gfx = (u16*)mcpfontTiles;
  mcpfont.numChars = 95;
  mcpfont.numColors =  mcpfontPalLen / 2;
  mcpfont.pal = (u16*)mcpfontPal;
  consoleSetFont(&bottom_screen, &mcpfont);

  // set console background layer to top priority
  bgSetPriority(bottom_screen.bgId, 0);

  // this triggers the initial mode fade in
  mm_txt_firstfade_done = 0;
  mm_txt_render_hilite = 0;

  // initialize to black text
  BG_PALETTE_SUB[MM_FONT_COLOR_INDEX] = RGB15(0, 0, 0);

  // initialize pwd
  // just using sn out of habit
  snprintf(pwd, FS_PATH_MAXLEN, "/");

  //
  // initialize the menu
  //
  //  mm_hist_push("WhammyPad!");
  //  mm_create_root();
  mm_create_last();

}


void mode__main_menu___top_renderer(void) {

  int t_blend;

  // 
  // initialize to unfaded values
  // 
  t_blend = 0;


  // 
  // do top background fade-in
  // 
  if (mode_ms < MAIN_MENU__TOP_BG_FADE_IN_START_MS) {
    t_blend = 31;
  } else if (mode_ms < (MAIN_MENU__TOP_BG_FADE_IN_START_MS + 
			MAIN_MENU__TOP_BG_FADE_IN_DURATION_MS)) {
    // main screen bg fade
    t_blend = 31 - ((mode_ms - MAIN_MENU__TOP_BG_FADE_IN_START_MS) * 31 / 
		     MAIN_MENU__TOP_BG_FADE_IN_DURATION_MS);

  } 

  //
  // handle fadeout
  //
  if (mode != next_mode) {
    if ((mode_ms - exit_mode_ms) < MAIN_MENU__TOP_BG_FADE_OUT_DURATION_MS) {
      t_blend = ((mode_ms - exit_mode_ms) * 
		 31 / MAIN_MENU__TOP_BG_FADE_OUT_DURATION_MS);
    } else {
      t_blend = 31;
    }
    // fadeouts should only be fading out
    // i.e. this code may be executing right after an aborted fadein
    //    t_blend = MAX(REG_BLDY, t_blend);
    t_blend = MAX(t_blend, mcp_get_blend(MCP_MAIN_SCREEN));
  } // end handle fadeout

  // actually set the blend register value
  mcp_set_blend(MCP_MAIN_SCREEN,
		t_blend);

}


void mode__main_menu___bot_renderer(void) {

  int t_blend;

  if (mode_ms < MAIN_MENU__BOT_TXT_FADE_IN_START_MS) {
  } else {
    if (!mm_txt_firstfade_done) {
      mm_txt_render_hilite = 1;
      BG_PALETTE_SUB[MM_FONT_COLOR_INDEX] = RGB15(0, 0, 0);
      mcp_fade_set_level(MCP_SUB_SCREEN, 3, 0);
      mcp_bg_show(MCP_SUB_SCREEN, 3);
      mcp_fade_start(MCP_SUB_SCREEN, 
		     3, 
		     1,
		     MAIN_MENU__BOT_TXT_FADE_IN_DURATION_MS,
		     idle_rate,
		     NULL);
      mm_txt_firstfade_done = 1;
    }
  }

  // clear the console text
  consoleClear();
  // and hilite
  render_hilite(bgs2, 0, 0, 0, 0, 0, 1);

  // set console variable parameters
  // note: -128, -128 is doubling
  bgSetRotateScale(bottom_screen.bgId, 
		   0,
		   intToFixed(1,8) + mm_dm_xscl,
		   intToFixed(1,8) + mm_dm_yscl);

  // TODO: verbosely document this ugly crap
  // the -4 is down half an 8 pixel char (that has scaled to 16pix)
  // the other term is another optional same amount
  bgScroll(bottom_screen.bgId, 
  	   0, 
	   -4 - (mm_text_offset_target & 15));
  bgUpdate();


  // 
  // initialize to unfaded values
  // 
  t_blend = 0;

  // 
  // do bg fade-in
  // 
  if (mode_ms < MAIN_MENU__BOT_BG_FADE_IN_START_MS) {
    // pre fade-in
    t_blend = 31;
  } else if (mode_ms < (MAIN_MENU__BOT_BG_FADE_IN_START_MS + 
			MAIN_MENU__BOT_BG_FADE_IN_DURATION_MS)) {
    // main screen bg fade-in
    t_blend = 31 - ((mode_ms - MAIN_MENU__BOT_BG_FADE_IN_START_MS) * 31 / 
		    MAIN_MENU__BOT_BG_FADE_IN_DURATION_MS);

  } 


  //
  // do fadeout, possibly overriding above
  //
  if (mode != next_mode) {
    // start a fade-out if a fade isn't in progress, or is fading in,
    if (!mcp_fade_get_enabled(MCP_SUB_SCREEN, 3) ||
	mcp_fade_get_fwd(MCP_SUB_SCREEN, 3)) {
      mcp_fade_start(MCP_SUB_SCREEN, 
		     3, 
		     0,
		     MAIN_MENU__BOT_TXT_FADE_OUT_DURATION_MS,
		     idle_rate,
		     NULL);
    }

    // fade-out
    if ((mode_ms - exit_mode_ms) < MAIN_MENU__BOT_BG_FADE_OUT_DURATION_MS) {
      t_blend = ((mode_ms - exit_mode_ms) * 
		      31 / MAIN_MENU__BOT_BG_FADE_OUT_DURATION_MS);
    } else {
      t_blend = 31;
    }
    // only fadeout
    t_blend = MAX(t_blend, mcp_get_blend(MCP_SUB_SCREEN));

  }

  mcp_set_blend(MCP_SUB_SCREEN,
		t_blend);

  /*
  // clear the console text
  //  consoleClear();
  // show the entire font
  int i, x, y;
  // 32 + 95, now 60 + 9
  for (i = 32; i < (32 + 95); i++) {
    y = (i - 32) / 12;
    x = (i - 32) % 12;
    printf("\x1b[%02d;%02dH%c", 
	   y, x, i);
  }
  */

  // print/render the main menu labels
  mm_print();

  if (mm_txt_render_hilite) {
    render_hilite(bgs2,
		  RGB15(RGB15_TO_R5(mm_hilitecolor) * mcp_fade_get_level(MCP_SUB_SCREEN, 3) / MCP_FADE_MAXLEVEL,
			RGB15_TO_G5(mm_hilitecolor) * mcp_fade_get_level(MCP_SUB_SCREEN, 3) / MCP_FADE_MAXLEVEL,
			RGB15_TO_B5(mm_hilitecolor) * mcp_fade_get_level(MCP_SUB_SCREEN, 3) / MCP_FADE_MAXLEVEL),
		  hilite_offset,
		  SCREEN_WIDTH * mm_dm_width / 32,
		  SCREEN_HEIGHT * 3 / 2 / mm_dm_height_lines,
		  SCREEN_HEIGHT / mm_dm_height_lines / 2,
		  0);
  }

  // set the fade by changing the font's palette entry
  BG_PALETTE_SUB[MM_FONT_COLOR_INDEX] = 
    RGB15(RGB15_TO_R5(mm_txt_fontcolor) * mcp_fade_get_level(MCP_SUB_SCREEN, 3) / MCP_FADE_MAXLEVEL,
	  RGB15_TO_G5(mm_txt_fontcolor) * mcp_fade_get_level(MCP_SUB_SCREEN, 3) / MCP_FADE_MAXLEVEL,
	  RGB15_TO_B5(mm_txt_fontcolor) * mcp_fade_get_level(MCP_SUB_SCREEN, 3) / MCP_FADE_MAXLEVEL);

}


void mode__main_menu___input_handler(void) {

  //
  // prepare input, i.e. transmogrify special held situations 
  // into downkeys
  //

  // only bother if sunset has expired
  if (time_val_compare(num_ticks, heldover_sunset) < 0) {
    
    if ((heldkeys & KEY_L) && (heldkeys & KEY_R)) {
      //
      // held(L)+held(R)+something
      //
      
      if (heldkeys & KEY_LEFT) {
	downkeys |= KEY_LEFT;
      }
      
      if (heldkeys & KEY_RIGHT) {
	downkeys |= KEY_RIGHT;
      }
      
      if (heldkeys & KEY_UP) {
	downkeys |= KEY_UP;
      }
      
      if (heldkeys & KEY_DOWN) {
	downkeys |= KEY_DOWN;
      }
      
    } else if (heldkeys & KEY_L) {
      
      if (heldkeys & KEY_A) {
	downkeys |= KEY_A;
      }
      
      if (heldkeys & KEY_B) {
	downkeys |= KEY_B;
      }
      
      if (heldkeys & KEY_X) {
	downkeys |= KEY_X;
      }
      
      if (heldkeys & KEY_Y) {
	downkeys |= KEY_Y;
      }
      
      if (heldkeys & KEY_LEFT) {
	downkeys |= KEY_LEFT;
      }
      
      if (heldkeys & KEY_RIGHT) {
	downkeys |= KEY_RIGHT;
      }
      
      if (heldkeys & KEY_UP) {
	downkeys |= KEY_UP;
      }
      
      if (heldkeys & KEY_DOWN) {
	downkeys |= KEY_DOWN;
      }
      
    } else if (heldkeys & KEY_R) {
      
      if (heldkeys & KEY_A) {
	downkeys |= KEY_A;
      }
      
      if (heldkeys & KEY_B) {
	downkeys |= KEY_B;
      }
      
      if (heldkeys & KEY_X) {
	downkeys |= KEY_X;
      }
      
      if (heldkeys & KEY_Y) {
	downkeys |= KEY_Y;
      }

      if (heldkeys & KEY_LEFT) {
	downkeys |= KEY_LEFT;
      }
      
      if (heldkeys & KEY_RIGHT) {
	downkeys |= KEY_RIGHT;
      }
      
      if (heldkeys & KEY_UP) {
	downkeys |= KEY_UP;
      }
      
      if (heldkeys & KEY_DOWN) {
	downkeys |= KEY_DOWN;
      }
      
    } else  {
      
      if (heldkeys & KEY_LEFT) {
	downkeys |= KEY_LEFT;
      }
      
      if (heldkeys & KEY_RIGHT) {
	downkeys |= KEY_RIGHT;
      }
      
      if (heldkeys & KEY_UP) {
	downkeys |= KEY_UP;
      }
      
      if (heldkeys & KEY_DOWN) {
	downkeys |= KEY_DOWN;
      }
      
    }
    
  } // end if heldover_sunset expired

  //
  // handle input
  //
  if ((heldkeys & KEY_L) && (heldkeys & KEY_R)) {
    //
    // held(L)+held(R)+something
    //

  } else if (heldkeys & KEY_L) {
    //
    // held(L)+something
    //

    if (downkeys & KEY_X) {
    }

    if (downkeys & KEY_Y) {
    }

    if (downkeys & KEY_A) {
    }

    if (downkeys & KEY_B) {
    }

    if (downkeys & KEY_UP) {
    }

    if (downkeys & KEY_DOWN) {
    }

    if (downkeys & KEY_START) {
    }

  } else if (heldkeys & KEY_R) {
    //
    // held(R)+something
    //

    if (downkeys & KEY_UP) {
    }

    if (downkeys & KEY_DOWN) {
    }

    if (downkeys & KEY_LEFT) {
    }

    if (downkeys & KEY_RIGHT) {
    }

  } else {
    //
    // no interesting modifer keys held
    //

    if (downkeys & KEY_START) {
      if (last_mode == MODE_NUM_MODES) {
	system_xmode_new(MODE_MAIN_MENU);
      } else {
	system_xmode_new(last_mode);
      }
    }

    if (downkeys & KEY_SELECT) {
    }

    if (downkeys & KEY_UP) {
      mm_set_selection_by_index(selection_index - 1);
      heldover_sunset = time_val_add_ms(num_ticks, mm_dm_repeat_rate);
    }

    if (downkeys & KEY_DOWN) {
      mm_set_selection_by_index(selection_index + 1);
      heldover_sunset = time_val_add_ms(num_ticks, mm_dm_repeat_rate);
    }

    if (downkeys & KEY_LEFT) {
    }

    if (downkeys & KEY_RIGHT) {
    }

    if (downkeys & KEY_X) {
    }

    if (downkeys & KEY_Y) {
    }

    if (downkeys & KEY_A) {
      // run selected select_cb
      if (mm_get_node_by_index(selection_index) != NULL) {
	// wow this is pretty gross
	// don't add *(go back)* to the stack, as our selection of it
	// always means we'd prefer whatever is already on the 
	// top of the stack.
	if (strstr(mm_get_node_by_index(selection_index)->label,
		   "(go back)") == NULL) {
	  mm_hist_push(mm_get_node_by_index(selection_index)->label);
	}
	if (mm_get_node_by_index(selection_index)->select_cb != NULL) {
	  mcp_snd_click();
	  (*((mm_get_node_by_index(selection_index))->select_cb))();
	}
      }
    }

    if (downkeys & KEY_B) {
    }

  }


  // touchpad handling is independent of modifiers (at the moment)
  if ((downkeys & KEY_TOUCH) || (heldkeys & KEY_TOUCH)) {

  }

}

void mode__main_menu___idle(void) {
  //
  // handle fadeout
  //
  if (mode != next_mode) {
    if (((mode_ms - exit_mode_ms) > MAIN_MENU__TOP_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > MAIN_MENU__BOT_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > MAIN_MENU__BOT_TXT_FADE_OUT_DURATION_MS)) {
      system_xmode_real();
    }
  }
}


void mode__main_menu___exit(void) {

  mm_free();

}


void mm_set_doublemode(int mode) {

  mm_doublemode = mode;
  if (mode) {
    mm_dm_width = 24;
    mm_dm_height_lines = 12;
    mm_dm_scroll_pad = 4;
    mm_dm_repeat_rate = 200;
    mm_dm_xscl = -128;
    mm_dm_yscl = -128;
  } else {
    mm_dm_width = 30;
    mm_dm_height_lines = 24;
    mm_dm_scroll_pad = 8;
    mm_dm_repeat_rate = 125;
    mm_dm_xscl = 0;
    mm_dm_yscl = 0;
  }

  // XXX if this works, make this a function shared with other copy
  // clear the console text
  consoleClear();
  // and the hilite
  render_hilite(bgs2, 0, 0, 0, 0, 0, 1);

  // set console variable parameters
  // note: -128, -128 is doubling
  bgSetRotateScale(bottom_screen.bgId, 
		   0,
		   intToFixed(1,8) + mm_dm_xscl,
		   intToFixed(1,8) + mm_dm_yscl);

  // TODO: verbosely document this ugly crap
  // the -4 is down half an 8 pixel char (that has scaled to 16pix)
  // the other term is another optional same amount
  bgScroll(bottom_screen.bgId, 
  	   0, 
	   -4 - (mm_text_offset_target & 15));
  bgUpdate();


}


void render_hilite(int bgid,
		   int color,
		   int y_offset,
		   int width,
		   int height,
		   int roundness,
		   int unrender) {

  // ooh, ahh, static local variables put to good use
  // edunote: in case I forget, static local variables in functions, which
  //          clearly I rarely use, are roughly equivalent to private globals,
  //          i.e. initialized once, and retaining value across multiple 
  //          invocations of the function
  static int y_offset_prev = 0;
  // note: the dmaCopy doesn't behave as I'd expect with a length of 0
  static int height_prev = 1;
  static int dirty = 0;

  int i;
  int rounded_width;

  
  // note: bgGetGfxPtr returns u16*, but this is 8bpp data

  // edunote: brief net search suggests there is no advantage to using
  //          bitshifts instead of *2^n, as compiler will optimize while
  //          the code will remain more readable.

  if (unrender || dirty) {
    // first, restore possible background damage from previous invocation
    DC_FlushRange(((const void*)MM_BG_BMP) + (y_offset_prev * SCREEN_WIDTH), 
		  height_prev * SCREEN_WIDTH);
    dmaCopy(((const void*)MM_BG_BMP) + (y_offset_prev * SCREEN_WIDTH), 
	    (void *)(bgGetGfxPtr(bgid)) + (y_offset_prev * SCREEN_WIDTH), 
	    height_prev * SCREEN_WIDTH);
    dirty = 0;
    // unrender means that is all that should be done
    if (unrender) return;
  }

  // set the hilite color 
  BG_PALETTE_SUB[MM_HILITE_COLOR_INDEX] = color;

  // render/copy the top rounded portion of the highlite
  for (i = y_offset ; i < (y_offset + roundness) ; i++) {
    rounded_width = width - ((roundness - (i - y_offset)) * 2);
    dmaFillHalfWords((MM_HILITE_COLOR_INDEX * SCREEN_WIDTH) + MM_HILITE_COLOR_INDEX, 
		     ((void *)bgGetGfxPtr(bgid)) + 
		     (i * SCREEN_WIDTH) + ((SCREEN_WIDTH - rounded_width) / 2),
		     rounded_width);
  }

  // render/copy the middle unrounded portion of the highlite
  for (i = (y_offset + roundness) ; 
       i < (y_offset + height - roundness) ; 
       i++) {
    dmaFillHalfWords((MM_HILITE_COLOR_INDEX * SCREEN_WIDTH) + MM_HILITE_COLOR_INDEX, 
		     ((void *)bgGetGfxPtr(bgid)) + 
		     (i * SCREEN_WIDTH) + ((SCREEN_WIDTH - width) / 2),
		     width);
  }

  // render/copy the bottom rounded portion of the highlite
  for (i = (y_offset + height - roundness) ; 
       i < (y_offset + height) ; 
       i++) {
    rounded_width = width - ((roundness - ((y_offset + height) - i)) * 2);
    dmaFillHalfWords((MM_HILITE_COLOR_INDEX * SCREEN_WIDTH) + MM_HILITE_COLOR_INDEX, 
		     ((void *)bgGetGfxPtr(bgid)) + 
		     (i * SCREEN_WIDTH) + ((SCREEN_WIDTH - rounded_width) / 2),
		     rounded_width);
  }

  // save the information about the part of the screen we just hilited,
  // so that it can be restored next time.
  y_offset_prev = y_offset;
  height_prev = height;

}

void mm_add_node(const char *label, 
		 int selectable,
		 void(*select_cb)(void)) {

  gzmcp_menu_node **p = &mm_list;


  // get a pointer to final NULL pointer in the llist
  while (*p != NULL) p = &((*p)->next);

  // allocate memory for a new node
  *p = malloc(sizeof(gzmcp_menu_node));
  if (*p == NULL) {
    die();
  }

  strncpy((*p)->label, label, MENU_LABEL_MAXLEN);
  (*p)->select_cb = select_cb;
  (*p)->selectable = selectable;
  (*p)->next = NULL;

  num_selections++;
}


gzmcp_menu_node *mm_get_node_by_index(int index) {

  int i;
  gzmcp_menu_node *result;

  result = mm_list;

  for (i = 1; i < index; i++) {
    if (result->next == NULL) {
      return NULL;
    } else {
      result = result->next;
    }
  }

  return result;

}


int mm_get_index_by_label(const char *label) {

  int i;
  int done = 0;
  gzmcp_menu_node *node;

  node = mm_list;
  i=1;

  while (!done) {
    if (strncmp(node->label, label, MENU_LABEL_MAXLEN) == 0) return i;
    i++;
    node = node->next;
    if (node == NULL) done = 1;
  }

  return -1;
}


void mm_alphasort(gzmcp_menu_node *head, 
		  int from_index, 
		  int to_index) {

  gzmcp_menu_node *t_mm_node_lower_parent;
  gzmcp_menu_node *t_mm_node_lower;
  gzmcp_menu_node *t_mm_node_upper_parent;
  gzmcp_menu_node *t_mm_node_upper;
  gzmcp_menu_node *t_mm_node_ptr;
  int t_cnt;
  int i, j, k;


  // current logic cannot change the first entry, which
  // by convention is '(go back)' anyway
  if (from_index <= 1) return;

  // note: for now, ignore selection, assuming that
  //       alphasort will be called after menu creation,
  //       but before selection initialization
  
  t_cnt = from_index;

  for (i = from_index ; i <= to_index ; i++) {

    for (j = from_index ; j < to_index  ; j++) {

      t_mm_node_lower_parent = mm_get_node_by_index(j - 1);
      t_mm_node_lower = mm_get_node_by_index(j);
      // these two are mainly to silence - possible uninitialized use warnings
      t_mm_node_upper_parent = mm_get_node_by_index(j);
      t_mm_node_upper = mm_get_node_by_index(j + 1);

      if (!(t_mm_node_lower->selectable)) continue;
      if (t_mm_node_lower->next == NULL) continue;

      for (k = (j + 1); k <= to_index ; k++) {
	t_mm_node_upper = mm_get_node_by_index(k);
	if (t_mm_node_upper->selectable) break;
      }

      if (t_mm_node_upper->selectable) {
	t_mm_node_upper_parent = mm_get_node_by_index(k - 1);
	// compare the node's labels
	if (strncmp(t_mm_node_lower->label, 
		    t_mm_node_upper->label, 
		    MENU_LABEL_MAXLEN) > 0) {
	  // swap
	  t_mm_node_ptr = t_mm_node_lower->next;
	  t_mm_node_lower->next = t_mm_node_upper->next;
	  t_mm_node_upper->next = t_mm_node_ptr;

	  t_mm_node_ptr = t_mm_node_lower_parent->next;
	  t_mm_node_lower_parent->next = t_mm_node_upper_parent->next;
	  t_mm_node_upper_parent->next = t_mm_node_ptr;

	} // end if need to actually do a bubble swap

      } // end if we have a selectable node to possibly bubble swap

    } // end of a single pass of bubbling iteration

  } // end outtermost bubble iteration

}


void mm_free(void) {

  gzmcp_menu_node *t_mm_node;


  // free the list entry by entry
  while (mm_list != NULL) {
    t_mm_node = mm_list->next;
    free(mm_list);
    mm_list = t_mm_node;
    num_selections--;
  }

  // initialize offsets
  mm_text_offset_target = (0 << 4) | 0;
  mm_text_offset_actual = (0 << 4) | 0;
  mm_hilite_offset_relative = (0 << 4) | 0;

  // and selection_index
  selection_index = 0;

}


void mm_set_selection_by_index(int index) {
  
  // for finding selectable index
  int found = 0;
  // for recording previous selected index
  int prev_index;
  // stack variables could be removed, but code would be less readable
  int offset_lines_fraction;
  int offset_lines;
  

  // record prior selection for rendering offset calculation below
  prev_index = selection_index;

  // do nothing if there is nothing to do
  if (num_selections < 1) return;

  // can't go out of bounds
  // assert candidate
  if ((index < 1) || (index > num_selections)) return;

#define PRV_OFFSET (mm_text_offset_target >> 4)
#define VIS_ENTRIES_MAX (mm_dm_height_lines - 1)
#define CUR_INNER_REGION_MAX PRV_OFFSET + VIS_ENTRIES_MAX - mm_dm_scroll_pad
#define CUR_INNER_REGION_MIN PRV_OFFSET + mm_dm_scroll_pad + 1

  //
  // set new index (skipping unselectable entries)
  //
  selection_index = index;
  if (selection_index < prev_index) {
    while (!found) {
      if (mm_get_node_by_index(selection_index)->selectable) {
	found = 1;
      } else if (selection_index == 1) {
	  // give up, no selectable entry found, use previous
	  selection_index = prev_index;
	  found = 1;
      } else {
	// try the next entry above
	selection_index--;
      }
    } // end while (!found)
  } else if (selection_index > prev_index) {
    while (!found) {
      if (mm_get_node_by_index(selection_index)->selectable) {
	found = 1;
      } else if (selection_index == num_selections) {
	  // give up, no selectable entry found, use previous
	  selection_index = prev_index;
	  found = 1;
      } else {
	// try the next entry above
	selection_index++;
      }
    } // end while (!found)
  } else {
    // no change in selection
    return;
  }

  //
  // handle many possible scenarios, the union of which is all possibilities
  //


  if (num_selections < VIS_ENTRIES_MAX) {
    // if the number of selections need not be scrolled, 
    // adjust so that the offset vertically centers the entries
    offset_lines_fraction = (num_selections % 2) * 8;
    // this logic needs to be rewritten and better understood
    offset_lines = ((VIS_ENTRIES_MAX - num_selections - 1) / -2);
  } else if ((selection_index <= CUR_INNER_REGION_MAX) &&
	     (selection_index >= CUR_INNER_REGION_MIN)) {
    // if the new index is in the existing visible region,
    // no adjustment is necessary
    offset_lines_fraction = mm_text_offset_target & 15;
    offset_lines = mm_text_offset_target >> 4;
  } else if (selection_index <= mm_dm_scroll_pad) {
    // handle case where scroll should be at the top
    offset_lines_fraction = 0;
    offset_lines = 0;
  } else if (selection_index >= (num_selections - mm_dm_scroll_pad)) {
    // handle case where scroll should be at the bottom
    offset_lines_fraction = 0;
    offset_lines = num_selections - VIS_ENTRIES_MAX;
  } else if (selection_index > prev_index) {
    // scroll down
    offset_lines_fraction = 0;
    offset_lines = selection_index -VIS_ENTRIES_MAX + mm_dm_scroll_pad;
  } else if (selection_index < prev_index) {
    // scroll up
    offset_lines_fraction = 0;
    offset_lines = selection_index - mm_dm_scroll_pad - 1;
  } else {
    // should be an assertion 
    offset_lines_fraction = 0;
    offset_lines = -42;
  }

  // set the new compact mm_text_offset_target from calculated values
  mm_text_offset_target = (offset_lines << 4) | offset_lines_fraction;

  //  int mm_text_offset_actual = (0 << 4) | 0;
  //  int mm_hilite_offset_relative = (0 << 4) | 0;

  hilite_offset = ((selection_index - offset_lines - 1) * 
		   ((SCREEN_HEIGHT) / mm_dm_height_lines)) + (SCREEN_HEIGHT / (mm_dm_height_lines * 4));
  hilite_offset += (offset_lines_fraction * ((SCREEN_HEIGHT / mm_dm_height_lines) / 8));

  // user audio feedback / click (quarter volume)
  if (prev_index && (selection_index != prev_index)) mcp_snd_click();

}


void mm_set_selection_by_label(const char *label) {

  // TODO: catch error/-1 from get_index_by_label
  if (mm_get_index_by_label(label) == -1) {
    mm_set_selection_by_index(1);
  } else {
    mm_set_selection_by_index(mm_get_index_by_label(label));
  }
    
}


void mm_set_selection_by_hist(void) {

  while (mm_get_index_by_label(mm_hist_peek()) == -1) {
    mm_hist_pop();
    if (mm_hist == NULL) break;
  }

  if (mm_hist == NULL) {
    mm_set_selection_by_index(1);
  } else {
    selection_index = 0;
    mm_set_selection_by_index(mm_get_index_by_label(mm_hist_peek()));
    mm_hist_pop();
  }
    
}


void mm_print(void) {

  gzmcp_menu_node *t_mm_node;
  int skip;
  int line_number = 0;


  t_mm_node = mm_list;
  skip = mm_text_offset_target >> 4;

  // first skip however lines mm_text_offset_target tells us to
  // note: this is for negative skip, positive skip is handled below
  while (skip < 0) {
    line_number++;
    skip++;
  }

  // iterate over the linked list of entry nodes, possibly rendering
  while (t_mm_node != NULL) {
    if (skip > 0) {
      skip--;
    } else {
      if (line_number < (mm_dm_height_lines - 1)) {
	// only print nonblank lines as not to overwrite a wrapped line
	if (strncmp(t_mm_node->label, "", 1) != 0) {
	  printf("\x1b[%02d;0H   %s", 
		 line_number,
		 t_mm_node->label);
	}
	line_number++;
      }
    }
    t_mm_node = t_mm_node->next;
  }

}


void mm_hist_push(const char *label) {

  gzmcp_menu_node *new_node;

  // allocate memory for a new node
  new_node = malloc(sizeof(gzmcp_menu_node));
  if (new_node == NULL) {
    die();
  }

  strncpy(new_node->label,
	  label,
	  MENU_LABEL_MAXLEN);
  // not used
  new_node->select_cb = NULL;
  // not used
  new_node->selectable = 0;
  new_node->next = mm_hist;

  mm_hist = new_node;

}


char *mm_hist_peek(void) {
  if (mm_hist == NULL) return NULL;
  return (mm_hist->label);
}


void mm_hist_pop(void) {

  gzmcp_menu_node *t_mm_node;

  if (mm_hist == NULL) return;

  t_mm_node = mm_hist->next;
  free(mm_hist);
  mm_hist = t_mm_node;
  
}

void mm_go_whammypad(void) {

  system_xmode_new(MODE_TPW__JAM);

}


void mm_go_settings(void) {

  // XXX: unimplemented

}


void mm_go_files(void) {

  mcp_snd_click();
  mm_hist_push("(go back)");
  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 0,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 &mm_create_files);

}


void mm_go_usermanual(void) {

  // XXX: unimplemented

}


void mm_go_misc(void) {

  mcp_snd_click();
  mm_hist_push("(go back)");
  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 0,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 &mm_create_misc);

}


void mm_go_shutdown(void) {

  // this isn't enough, netsearching seems to suggest it isn't possible,
  // or at least widely known how to accomplish this.

  // this gets to sleep mode, but thats it
  netmode_go_offline();
  powerOff(POWER_ALL | PM_SYSTEM_PWR);
  swiSetHaltCR(0x80);

}


void mm_go_get_update(void) {

  system_xmode_new(MODE_GET_UPDATE);

}


void mm_go_get_and_burn_update(void) {

  overwrite_self_with_update = 1;
  system_xmode_new(MODE_GET_UPDATE);

}


void mm_go_ssid_input(void) {

  mm_hist->select_cb = &mm_create_misc;
  system_xmode_new(MODE_SSID__INPUT);

}


void mm_go_show_credits(void) {

  mm_hist->select_cb = &mm_create_misc;
  system_xmode_new(MODE_INTRO__CREDITS);

}


void mm_go_show_ssid_list(void) {

  Wifi_AccessPoint ap;
  char ssidbuf[MENU_LABEL_MAXLEN + 1];
  int i, nn;

  mm_free();

  mm_set_doublemode(0);
  
  mm_add_node("/========================\\", 0, NULL);
  mm_add_node("|                        |", 0, NULL);
  mm_add_node("| Scanned WiFi Networks  |", 0, NULL);
  mm_add_node("|                        |", 0, NULL);
  mm_add_node("\\========================/", 0, NULL);
  mm_add_node("", 0, NULL);
  mm_add_node("(go back)",
	      1,
	      mm_go_back);
  mm_add_node("", 0, NULL);

  nn = Wifi_GetNumAP();

  for (i = 0 ; i < nn ; i++) {

    if (WIFI_RETURN_OK == Wifi_GetAPData(i,&ap)) {

      if (strncmp(ap.ssid, "", 2) != 0) {

	if (mm_get_index_by_label(ap.ssid) == -1) {

	  strncpy(ssidbuf, ap.ssid, MENU_LABEL_MAXLEN);
	  
	  mm_add_node("", 0, NULL);
	  mm_add_node(ssidbuf,
		      1,
		      NULL);

	} // end if not already in the list

      } // end if not an empty string

    } // end got data for ap       
                
  } // end iteration over networks 

  mm_alphasort(mm_list, 9, num_selections);

  mm_set_selection_by_hist();

  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 1,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 NULL);

}


void mm_go_back(void) {

  mcp_snd_click();
  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 0,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 &mm_create_last);

}


void mm_go_misc_back(void) {

  mcp_snd_click();
  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 0,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 &mm_create_root);

}


void mm_go_files_back(void) {

  // note: this made a big difference, I.e. I think was causing
  //       stack corruption of the 16k stack
  //  char tmpstring_dir[FS_PATH_MAXLEN];
  //  char tmpstring_base[FS_PATH_MAXLEN];
  char *tmpstring_dir;


  if ((tmpstring_dir = malloc(FS_PATH_MAXLEN)) == NULL ) die();

  if (strncmp(pwd, "/", FS_PATH_MAXLEN) == 0) {
    // if pwd is /, go back
    mcp_snd_click();
    mcp_fade_start(MCP_SUB_SCREEN, 
		   3, 
		   0,
		   MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		   idle_rate,
		   &mm_create_root);
  } else {
    // else cd .. and regenerate list
    // note: dirname manpage on linux.com, but not f10, says not to free
    //       the returned pointer.  Also warnings about passed string being
    //       modified.
    //    strncpy(tmpstring_dir, dirname(pwd), FS_PATH_MAXLEN);
    // ... but my dirname implementation is different at the moment
    mcp_dirname(tmpstring_dir, pwd);

    strncpy(pwd, tmpstring_dir, FS_PATH_MAXLEN);

    mcp_fade_start(MCP_SUB_SCREEN, 
		   3, 
		   0,
		   MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		   idle_rate,
		   &mm_create_files);
  }

  free(tmpstring_dir);

  return;
}


void mm_go_files_entry(void) {

  // stack->heap fodder
  struct stat t_statbuf;

  // use selection_index to retrieve entry string/filename
  // and then use pwd to craft full path
  if (strncmp(pwd, "/", 2) == 0) {
    snprintf(t_scratch_path, 
	     FS_PATH_MAXLEN, 
	     "/%s",
	     (mm_get_node_by_index(selection_index))->label);
  } else {
    snprintf(t_scratch_path, 
	     FS_PATH_MAXLEN, 
	     "%s/%s",
	     pwd,
	     (mm_get_node_by_index(selection_index))->label);
  }

  // TODO: catch error
  stat(t_scratch_path, &t_statbuf);

  if (S_ISDIR(t_statbuf.st_mode)) {
    // set new pwd
    strncpy(pwd, t_scratch_path, FS_PATH_MAXLEN);
    // lose a trailing slash if present
    // note: I really think I tried the same with t_scratch_path above,
    //       and it didn't make its way through to pwd??
    if (pwd[strlen(pwd) - 1] == '/') {
      pwd[strlen(pwd) - 1] = '\0';
    }
    mm_hist_push("../ (go back)");
    mcp_snd_click();
    mcp_fade_start(MCP_SUB_SCREEN, 
		   3, 
		   0,
		   MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		   idle_rate,
		   &mm_create_files);
  } else {
    execz(t_scratch_path, 0, NULL);
  }

  // TODO: add handlers for ogg, preverification of valid .nds, etc...

}


void mm_create(void (*create_func)(void)) {
  
  if (mm_hist != NULL) {
    mm_hist->select_cb = create_func;
  }

  (*create_func)();

}

void mm_create_last(void) {

  if (mm_hist != NULL) {
    if (mm_hist->select_cb != NULL) {
      (*(mm_hist->select_cb))();
      return;
    } else if (mm_hist->next != NULL) {
      if (mm_hist->next->select_cb != NULL) {
	(*(mm_hist->next->select_cb))();
	return;
      }
    }
  }

  // fallback to root
  mm_create_root();
}


void mm_create_root(void) {

  mm_free();

  mm_set_doublemode(1);
  
  mm_add_node("WhammyPad!",
	      1,
	      mm_go_whammypad);
  mm_add_node("", 0, NULL);
  mm_add_node(" settings",
	      1,
	      mm_go_settings);
  mm_add_node("", 0, NULL);
  mm_add_node("   help",
	      1,
  	      mm_go_usermanual);
  mm_add_node("", 0, NULL);
  mm_add_node(" advanced",
	      1,
	      mm_go_misc);
  mm_add_node("", 0, NULL);
  mm_add_node("filesystem",
	      1,
	      mm_go_files);

  mm_set_selection_by_hist();

  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 1,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 NULL);

}


void mm_create_misc(void) {
  
  mm_free();

  mm_set_doublemode(0);

  mm_add_node("/========================\\", 0, NULL);
  mm_add_node("|                        |", 0, NULL);
  mm_add_node("| MCP:: Advanced Options |", 0, NULL);
  mm_add_node("|                        |", 0, NULL);
  mm_add_node("\\========================/", 0, NULL);
  mm_add_node("", 0, NULL);
  mm_add_node("(go back)",
	      1,
	      mm_go_misc_back);
  mm_add_node("", 0, NULL);
  mm_add_node("get update",
	      1,
	      mm_go_get_update);
  mm_add_node("", 0, NULL);
  mm_add_node("get and burn-in update",
	      1,
	      mm_go_get_and_burn_update);
  mm_add_node("", 0, NULL);
  mm_add_node("show credits",
	      1,
	      mm_go_show_credits);
  mm_add_node("", 0, NULL);
  mm_add_node("set custom ssid",
	      1,
	      mm_go_ssid_input);
  mm_add_node("", 0, NULL);
  mm_add_node("show ssid list",
	      1,
	      mm_go_show_ssid_list);

  /*
  // debugging
  char label[16];
  int i;
  for (i = 3; i <= 46; i++) {
    mm_add_node("", 0, NULL);
    snprintf(label, 16, "adv-test-%02d", i);
    mm_add_node(label,
		i % 2,
		NULL);
  }
  mm_alphasort(mm_list, 8, num_selections);
  // /debugging
  */

  mm_set_selection_by_hist();

  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 1,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 NULL);
}


void mm_create_files(void) {
  
  // the directory to open and scan (will be whatever pwd[] is)
  DIR *dir;
  // note: if the 16k stack space becomes really tight, this is a prime
  //       candidate for something to malloc on the heap
  // TODO: detect directory and add trailing / to label
  //         - and hope further trailing / handline is unneeded
  //  struct stat t_statbuf;
  // hint: 42
  struct dirent *arthur;

  // stack->heap fodder
  struct stat t_statbuf;


  mm_free();

  mm_set_doublemode(0);

  mm_add_node("/========================\\", 0, NULL);
  mm_add_node("|                        |", 0, NULL);
  mm_add_node("| Contents of directory- |", 0, NULL);
  mm_add_node("|                        |", 0, NULL);
  if (strncmp(pwd, "/", 2) == 0) {
    snprintf(t_scratch_path, 
	     MENU_LABEL_MAXLEN,
	     "| / (root)               |");
    
  } else {
    snprintf(t_scratch_path, 
	     MENU_LABEL_MAXLEN,
	     "| %-23s|",
	     pwd);
  }
  mm_add_node(t_scratch_path,
	      0,
	      NULL);
  // allow wrappage
  mm_add_node("|                        |", 0, NULL);
  mm_add_node("\\========================/", 0, NULL);
  mm_add_node("", 0, NULL);

  if (strncmp(pwd, "/", 2) == 0) {
    mm_add_node("(go back)",
		1,
		mm_go_files_back);
  } else {
    mm_add_node("../ (go back)",
		1,
		mm_go_files_back);
  }

  // open directory
  dir = opendir(pwd);

  // catch possible error
  if (dir == NULL) {
    mm_add_node("", 0, NULL);
    mm_add_node("ERROR: opendir failed",
		0,
		NULL);

    mm_set_selection_by_index(1);

    return;
  }

  // readdir returns struct dirent *, which has d_name[NAME_MAX] entry name string, and NULL on end of dir
  while ((arthur = readdir(dir)) != NULL) {
    // leave out . and .. (for .. we have back already)
    if ((strncmp(arthur->d_name, ".", 2) != 0) &&
	(strncmp(arthur->d_name, "..", 3) != 0)) {
      
      if (strncmp(pwd, "/", 2) == 0) {
	snprintf(t_scratch_path, 
		 FS_PATH_MAXLEN, 
		 "/%s",
		 arthur->d_name);
      } else {
	snprintf(t_scratch_path, 
		 FS_PATH_MAXLEN, 
		 "%s/%s",
		 pwd,
		 arthur->d_name);
      }
      // TODO: catch error
      retval = stat(t_scratch_path, &t_statbuf);
      
      if (retval == -1) {
	//	die("stat error");
	die();
      } else if (S_ISDIR(t_statbuf.st_mode)) {
	snprintf(t_scratch_path, 
		 MENU_LABEL_MAXLEN,
		 "%s/",
		 arthur->d_name);
      } else {
	snprintf(t_scratch_path, 
		 MENU_LABEL_MAXLEN,
		 "%s",
		 arthur->d_name);
      }

      mm_add_node("", 0, NULL);
      mm_add_node(t_scratch_path,
		  1,
		  mm_go_files_entry);
    }
  }

  // TODO: catch error / rv=-1
  closedir(dir);

  // 9 is dependent on header formatting above (and doublespacing)
  mm_alphasort(mm_list, 9, num_selections);

  mm_set_selection_by_label(mm_hist_peek());
  mm_hist_pop();

  mcp_fade_start(MCP_SUB_SCREEN, 
		 3, 
		 1,
		 MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS,
		 idle_rate,
		 NULL);
}
