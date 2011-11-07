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
## gzmcpc::mode__intro__main: mode for first intro splashscreen
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

#include <stdio.h>

#include <dswifi9.h>

#include <maxmod9.h>

#include "dmc.h"

#include "graphics.h"

#include "input.h"

#include "main.h"

#include "mcp.h"

#include "modes.h"

#include "mode__intro__main.h"

#include "network.h"

#include "sound.h"

#include "cloader/arm9_loader.h"



#include "sounds.h"

#include "sounds_bin.h"

#include "resources/bitmaps/guitar-zyx.splash.main.h"

#include "resources/bitmaps/dlava.h"

#include "resources/bitmaps/mcpfont.h"






ConsoleFont mcpfont;




void mode__intro__main___init(void) {

  // ooh, ahh, earcandy
  mmEffectEx(&sounds[SFX_STARTUP]);

  // initialize main screen 
  // edunote: had tried MODE_5_3D here, but after 3D was actually used,
  //          it would interfere with the bitmap showing.  I think I'm
  //          effectively bghiding all layers in intermode init, but
  //          maybe I'm not, or maybe there is another way to hide the 3D
  //          layer.  Probably I should figure out how to render 3D on 
  //          top of a bitmap successfully.  Then the rest might follow.
  videoSetMode(MODE_5_2D);

  // map main screen background fourth (128k) region to vram bank A
  vramSetBankA(VRAM_A_MAIN_BG_0x06060000);
  
  // NOTE: current half educated assumption is that or-ing together
  //       bg enabled layers here is nothing but effectively an
  //       advanced call to bgShow, which is also a part of bgInit
  // NOTE2: after some time, I'm convinced of the above, and about
  //        to write a function bg_init_hidden()
  videoSetModeSub(MODE_5_2D);

  // map sub screen background (only? 1/4?) to vram bank C
  vramSetBankC(VRAM_C_SUB_BG);

  mcp_console_init(&bottom_screen, 
		   MCP_SUB_SCREEN,
		   0, 
		   1,
		   1,
		   BgType_Text4bpp, 
		   BgSize_T_256x256, 
		   31, 
		   0);

  // set printf sink
  consoleSelect(&bottom_screen);

  // set console background layer to top priority
  bgSetPriority(bottom_screen.bgId, 0);

  // note: this must be done _after_ consoleInit (as that resets it)
  //       and _after_ loading our 8bit indexed bitmap reloads it
  // set to black to allow renderer to really control
  BG_PALETTE_SUB[255] = RGB15(0, 0, 0);

  // show the console layer
  mcp_bg_show(MCP_SUB_SCREEN, 0);

  // default to fully faded (to black)
  mcp_set_blend(MCP_MAIN_SCREEN, 
		MCP_MAX_BLEND_LEVEL);
  mcp_set_blend(MCP_SUB_SCREEN, 
		MCP_MAX_BLEND_LEVEL);

  // fade the mainscreen background to/from black, layer 3
  REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG3;
  // fade the lava background to/from black, layer 3
  REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG2;

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
  // note: priorities 0..3, 0 highest priority
  bgSetPriority(bg3, 3);

  // mcp_bg_init covers this
  // maybe this prevents flicker during decompress
  //  bgHide(bg3);

  // load main splash screen into screen/background memory (bgs3)
  decompress(guitar_zyx_splash_mainBitmap, 
	     (u16*)bgGetGfxPtr(bg3),  
	     LZ77Vram);
  // maybe this prevents flicker during decompress
  mcp_bg_show(MCP_MAIN_SCREEN, 3);

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

  // mcp_bg_init covers this
  //  bgHide(bgs2);

  // note: as per libnds doc on dma, do the flush first
  DC_FlushRange(dlavaBitmap, 256*256);
  dmaCopy(dlavaBitmap, bgGetGfxPtr(bgs2), 256*256);
  DC_FlushRange(dlavaPal, 256*2);
  dmaCopy(dlavaPal, BG_PALETTE_SUB, 256*2);
  
  bgShow(bgs2);

  // reinitialize the text color
  BG_PALETTE_SUB[255] = RGB15(0, 0, 0);

  /* bad experiment, but probably will use later anyway)
  consoleInit(&bottom_screen_x, 
	      3, 
	      BgType_ExRotation,
	      BgSize_ER_256x256,
	      31, 
	      1, 
	      false,
	      false);

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
  bgSetPriority(bottom_screen_x.bgId, 0);
  bgShow(bottom_screen_x.bgId);
  */


}

void mode__intro__main___top_renderer(void) {

  int t_blend;

  // 
  // initialize to unfaded values
  // 
  t_blend = 0;


  // 
  // do top background fade-in
  // 
  if (mode_ms < INTRO__MAIN__TOP_BG_FADE_IN_START_MS) {
    t_blend = 31;
  } else if (mode_ms < (INTRO__MAIN__TOP_BG_FADE_IN_START_MS + 
			INTRO__MAIN__TOP_BG_FADE_IN_DURATION_MS)) {
    // main screen bg fade
    t_blend = 31 - ((mode_ms - INTRO__MAIN__TOP_BG_FADE_IN_START_MS) * 31 / 
		     INTRO__MAIN__TOP_BG_FADE_IN_DURATION_MS);

  } 

  //
  // handle fadeout
  //
  if (mode != next_mode) {
    if ((mode_ms - exit_mode_ms) < INTRO__MAIN__TOP_BG_FADE_OUT_DURATION_MS) {
      t_blend = ((mode_ms - exit_mode_ms) * 
		 31 / INTRO__MAIN__TOP_BG_FADE_OUT_DURATION_MS);
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

void mode__intro__main___bot_renderer(void) {

  unsigned char t_font_int;
  int t_blend;


  // 
  // initialize to unfaded values
  // 
  t_blend = 0;
  t_font_int = (unsigned char)((int)font_intensity * 1);

  // 
  // do bg fade-in
  // 
  if (mode_ms < INTRO__MAIN__BOT_BG_FADE_IN_START_MS) {
    // pre fade-in
    t_blend = 31;
  } else if (mode_ms < (INTRO__MAIN__BOT_BG_FADE_IN_START_MS + 
			INTRO__MAIN__BOT_BG_FADE_IN_DURATION_MS)) {
    // main screen bg fade-in
    t_blend = 31 - ((mode_ms - INTRO__MAIN__BOT_BG_FADE_IN_START_MS) * 31 / 
		    INTRO__MAIN__BOT_BG_FADE_IN_DURATION_MS);

  } 

  // 
  // do text fade-in
  // 
  if (mode_ms < INTRO__MAIN__BOT_TXT_FADE_IN_START_MS) {
    // pre fade-in
    t_font_int = 0;
  } else if (mode_ms < (INTRO__MAIN__BOT_TXT_FADE_IN_START_MS + 
			INTRO__MAIN__BOT_TXT_FADE_IN_DURATION_MS)) {
    t_font_int = (unsigned char)((int)(font_intensity) * 
			       (mode_ms - INTRO__MAIN__BOT_TXT_FADE_IN_START_MS) / 
			       INTRO__MAIN__BOT_TXT_FADE_IN_DURATION_MS);
  } 


  //
  // do fadeout, possibly overriding above
  //
  if (mode != next_mode) {
    // fade-out
    if ((mode_ms - exit_mode_ms) < INTRO__MAIN__BOT_BG_FADE_OUT_DURATION_MS) {
      t_blend = ((mode_ms - exit_mode_ms) * 
		      31 / INTRO__MAIN__BOT_BG_FADE_OUT_DURATION_MS);
    } else {
      t_blend = 31;
    }
    // only fadeout
    t_blend = MAX(t_blend, mcp_get_blend(MCP_SUB_SCREEN));

    if ((mode_ms - exit_mode_ms) < INTRO__MAIN__BOT_TXT_FADE_OUT_DURATION_MS) {
      t_font_int = (unsigned char)((int)(font_intensity) * 
				 (INTRO__MAIN__BOT_TXT_FADE_OUT_DURATION_MS - (mode_ms - exit_mode_ms)) / 
				 INTRO__MAIN__BOT_TXT_FADE_OUT_DURATION_MS);
    } else {
      t_font_int = 0;
    }
    // only fadeout
    t_font_int = MIN(t_font_int, RGB15_TO_G5(BG_PALETTE_SUB[255]));
  }

  mcp_set_blend(MCP_SUB_SCREEN,
		t_blend);
  // set the font color from the calculated intensity (greenish)
  BG_PALETTE_SUB[255] = RGB15(t_font_int / 3,
			      t_font_int,
			      t_font_int / 3);



  // clear the console text
  consoleClear();

  // tell user they can skip the intro
  printf("\x1b[05;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");
  printf("\x1b[06;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");

  printf("\x1b[08;0H       website - http://    ");

  printf("\x1b[10;0H      VirOS.org/GuitarZyX   ");

  printf("\x1b[12;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");
  printf("\x1b[13;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");


  printf("\x1b[17;0H       press 'A' to skip    ");

}

void mode__intro__main___input_handler(void) {

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
      execz("/gzmcp/gzmcp-update.nds", 0, NULL);
    }

    if (downkeys & KEY_Y) {
      execz("/gzmcp/gzmcp-client.nds", 0, NULL);
    }

    if (downkeys & KEY_A) {
      execz("/data/apps/dso/DSOrganize.nds", 0, NULL);
    }

    if (downkeys & KEY_B) {
      execz("/gzmcp/gzmcp-update.bak", 0, NULL);
    }

  } else if (heldkeys & KEY_R) {
    //
    // held(R)+something
    //

  } else {
    //
    // no interesting modifer keys held
    //

    if (downkeys & KEY_START) {
    }

    if (downkeys & KEY_SELECT) {
    }

    if (downkeys & KEY_X) {
    }

    if (downkeys & KEY_Y) {
    }

    if (downkeys & KEY_A) {
      // if no ssid has been set, prompt the user
      if (! strncmp(ap_ssid, "unset", strlen(ap_ssid))) {
	system_xmode_new(MODE_SSID__INPUT);
      } else {
	system_xmode_new(MODE_TPW__JAM);
      }
    }

    if (downkeys & KEY_B) {
    }

  }


  // touchpad handling is independent of modifiers (at the moment)
  if ((downkeys & KEY_TOUCH) || (heldkeys & KEY_TOUCH)) {

  }

}

void mode__intro__main___idle(void) {
  //
  // handle fadeout
  //
  if (mode != next_mode) {
    if (((mode_ms - exit_mode_ms) > INTRO__MAIN__TOP_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > INTRO__MAIN__BOT_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > INTRO__MAIN__BOT_TXT_FADE_OUT_DURATION_MS)) {
      /*
      // XXX: this should get moved elsewhere, and use dma
      // XXX: not even remotely sure it is necessary or helps, just a theory
      
      // clear background image memory bank
      // 256 * 256 * 2 = 128k 
      dmaFillHalfWords(0,
		       (u16*)bgGetGfxPtr(bg3),
		       256 * 256 * 2);
      //      memptr = (u16*)bgGetGfxPtr(bg3);
      //      for (i = 0 ; i < (256 * 256) ; i++) memptr[i] = (u16)0;
      
      dmaFillHalfWords(0,
		       (u16*)bgGetGfxPtr(bgs2),
		       256 * 256);

      dmaFillHalfWords(0,
		       (u16*)bgGetGfxPtr(bgs0),
		       256 * 256 / 2);
      */
      system_xmode_real();
    }
  }


  if (mode == next_mode) {
    //
    // check for timed exit
    //
    if (mode_ms > (INTRO__MAIN__TOP_BG_FADE_IN_START_MS +
		   INTRO__MAIN__TOP_BG_FADE_IN_DURATION_MS +
		   INTRO__MAIN__TOP_BG_HOLD_DURATION_MS)) {
      
      // if no ssid has been set, prompt the user
      if (! strncmp(ap_ssid, "unset", strlen(ap_ssid))) {
	system_xmode_new(MODE_SSID__INPUT);
      } else {
	system_xmode_new(MODE_TPW__JAM);
      }
      
    }

  }

}


void mode__intro__main___exit(void) {

}


