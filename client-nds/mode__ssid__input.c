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
## gzmcpc::mode__ssid__input: ssid input mode
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

#include <string.h>

#include "debug.h"

#include "configfiles.h"

#include "graphics.h"

#include "input.h"

#include "modes.h"

#include "mode__ssid__input.h"

#include "network.h"

#include "sound.h"



#include "sounds.h"

#include "sounds_bin.h"

#include "resources/bitmaps/guitar-zyx.splash.main.h"




char ssid_input_buffer[SSID_MAXLEN + 1];
int ssid_input_cursor = 0;

int console_shown;




void mode__ssid__input___init(void) {

  console_shown = 0;

  // initialize main screen
  videoSetMode(MODE_5_2D);

  // map main screen background fourth (128k) region to vram bank A
  vramSetBankA(VRAM_A_MAIN_BG_0x06060000);

  // set the secondary/sub screen for text 
  videoSetModeSub(MODE_5_2D);

  // use vram bank c for the secondary/sub-screen(background)
  vramSetBankC(VRAM_C_SUB_BG);

  // default to fully faded to black
  REG_BLDY = 31;
  // set mainscreen to fade to black mode, utilizing bgs3
  REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG3;

  // default to not faded to black
  REG_BLDY_SUB = 31;
  // set subscreen to fade to black mode, utilizing bgs3
  REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG1;

  // XXX add a comment documenting args here
  // initialize the console
  // note: These should be the defaults used by consoleDemoInit()
  //       (but BgSize_T_256x512 I'm not so sure about)
  //       This is because these values are crafted to work with
  //       the similar defaults from keyboardDemoInit(), which
  //       I likewise copied.
  // 22,3...
  consoleInit(&bottom_screen, 
	      0, 
	      BgType_Text4bpp, 
	      BgSize_T_256x256, 
	      22, 
	      3, 
	      false,
	      true);

  consoleSelect(&bottom_screen);

  BG_PALETTE_SUB[255] = RGB15(0, 0, 0);

  // "This function sets the console to use sub display, 
  // VRAM_C, and BG0 and enables MODE_0_2D on the sub display"
  //  consoleDemoInit();

  //  keyboardDemoInit();
  //  "Same as calling keyboardInit(NULL, 3, BgType_Text4bpp, 
  //                                BgSize_T_256x512, 20, 0, false, true)"

  keyboardInit(&keyboard, 
	       1,
	       BgType_Text4bpp, 
	       BgSize_T_256x512, 
	       20, 
	       0, 
	       false, 
	       false);

  bgHide(keyboard.background);

  // mo experiment (just what a final true does on the kbInit)
  KeyMap* map = keyboard.mappings[keyboard.state];
  u16* pal = keyboard.keyboardOnSub ? BG_PALETTE_SUB : BG_PALETTE;
  decompress(keyboard.tiles, bgGetGfxPtr(keyboard.background),  LZ77Vram);
  dmaCopy(map->mapDataReleased, bgGetMapPtr(keyboard.background), 
	  map->width * map->height * keyboard.grid_height * keyboard.grid_width * 2 / 64);
  dmaCopy(keyboard.palette, pal, keyboard.paletteLen);
  // /mo

  keyboard.scrollSpeed = 1024;

  // note, when trying for fun swapping these, I think it exposed a desmume bug
  bgSetPriority(keyboard.background, 3);
  bgSetPriority(bottom_screen.bgId, 2);
  bgHide(keyboard.background);
  bgHide(bottom_screen.bgId);

  // the example code invokes this once, will try it that way first
  //  keyboardShow();

  // kbshow exp
  keyboard.visible = 1;
  bgSetScroll(keyboard.background, 0, keyboard.offset_y);
  bgUpdate();

  // /kbshow exp

  // init screen layer/background 3 
  bg3 = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 24, 0);
  // its initial priority, lowest (to emphasize lack of other enabled layers)
  // priorities 0..3, 0 highest priority
  bgSetPriority(bg3, 3);

  // load main splash screen into screen/background memory (bg3)
  decompress(guitar_zyx_splash_mainBitmap, (u16*)bgGetGfxPtr(bg3),  LZ77Vram);

  iprintf("\n");
  iprintf("================================\n");
  // tailer header if this is an initial setting
  if (! strncmp(ap_ssid, "unset", strlen(ap_ssid))) {
  iprintf("   Initial WiFi Configuration   \n");
  } else {
  iprintf("       WiFi Configuration       \n");
  }
  iprintf("\n");
  iprintf("Enter SSID, or 'wfc', then Rtrn\n");
  iprintf("================================\n");
  iprintf("\n");
  iprintf("SSID> ");
      
  bgShow(keyboard.background);
  bgShow(bottom_screen.bgId);

}

void mode__ssid__input___top_renderer(void) {

  unsigned char tfontint;

  // do a 1 second fade in
  if (mode_ms < 3000) {
    REG_BLDY = 31 - (31 * mode_ms/ 3000);
    REG_BLDY_SUB = 31 - (31 * mode_ms/ 3000);

    tfontint = (unsigned char)((int)(font_intensity) * 
			       (mode_ms - 0) / 
			       3000);

    BG_PALETTE_SUB[255] = RGB15(tfontint / 3,
				tfontint,
				tfontint / 3);

  } else {
    if (! console_shown) {
      bgShow(keyboard.background);
      bgShow(bottom_screen.bgId);
      console_shown = 1;
    }
    // TODO: need fadeout mechanism on mode exit
  }
}

void mode__ssid__input___bot_renderer(void) {
  

}

void mode__ssid__input___input_handler(void) {

  int key;

  // get possible keystroke
  key = keyboardUpdate();

  // handle keystroke if necessary
  if (key > 0) {
    // handle return
    // TODO: define magic key codes
    if (key == 10) {
      // experiment - smoother ui?
      bgUpdate();
      swiWaitForVBlank();

      snprintf(ap_ssid, SSID_MAXLEN, "%s", ssid_input_buffer);
      // TODO write out new configfile
      rewrite_defaults_with_new_ssid();
      // go offline
      netmode_go_offline();
      // start trying to go online with the new ssid
      network_state = NSM_START;
      system_xmode_new(MODE_TPW__JAM);
    } else if (key == 8) {
      // do nothing on an extra backspace
      if (ssid_input_cursor) {
	iprintf("%c", key);
	ssid_input_cursor--;
	ssid_input_buffer[ssid_input_cursor + 1] = '\0';
      }
    } else {
      iprintf("%c", key);
      ssid_input_buffer[ssid_input_cursor] = key;
      ssid_input_buffer[ssid_input_cursor + 1] = '\0';
      ssid_input_cursor++;
    }
  }

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
      system_xmode_new(MODE_TPW__JAM);
    }

    if (downkeys & KEY_X) {
      system_xmode_new(MODE_TPW__JAM);
    }

    if (downkeys & KEY_Y) {
      system_xmode_new(MODE_TPW__JAM);
    }

    if (downkeys & KEY_A) {
      system_xmode_new(MODE_TPW__JAM);
    }

    if (downkeys & KEY_B) {
      system_xmode_new(MODE_TPW__JAM);
    }

  }


  // touchpad handling is independent of modifiers (at the moment)
  if ((downkeys & KEY_TOUCH) || (heldkeys & KEY_TOUCH)) {

  }

}

void mode__ssid__input___idle(void) {
  //
  // handle fadeout
  //
  if (mode != next_mode) {
    if (((mode_ms - exit_mode_ms) > SSID__INPUT__TOP_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > SSID__INPUT__BOT_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > SSID__INPUT__BOT_TXT_FADE_OUT_DURATION_MS)) {

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

      system_xmode_real();
    }
  }
}


void mode__ssid__input___exit(void) {

}
