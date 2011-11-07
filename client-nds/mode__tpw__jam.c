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
## gzmcpc::mode__tpw__jam: TouchPadWhammy Jamming Mode (i.e. the mcp's soul)
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


#include "debug.h"

#include "dmc.h"

#include "graphics.h"

#include "input.h"

#include "mcp.h"

#include "metrognome.h"

#include "modes.h"

#include "mode__get_update.h"

#include "mode__tpw__jam.h"

#include "network.h"

#include "rak.h"

#include "sound.h"

#include "time.h"


#include "sounds.h"

#include "sounds_bin.h"

#include "resources/bitmaps/dlava.h"



uint8 comb_charge[COMB_WIDTH][COMB_HEIGHT];

unsigned char x3d_intensity = 0;

v16 model_xcenter = (MODEL_XMAX + MODEL_XMIN) / 2;
v16 model_ycenter = (MODEL_YMAX + MODEL_YMIN) / 2;
v16 model_zcenter = (MODEL_ZMAX + MODEL_ZMIN) / 2;
v16 model_width = MODEL_XMAX - MODEL_XMIN;
v16 model_height = MODEL_YMAX - MODEL_YMIN;
v16 model_depth = MODEL_ZMAX - MODEL_ZMIN;


int touch_whammy_enabled = 0;
int touch_whammy_x_midi_parm = DEFAULT_X_PARM;
int touch_whammy_y_midi_parm = DEFAULT_Y_PARM;

int current_preset = 1;

int button_x_preset_num = CM_PRESET_X_NUM;
int button_y_preset_num = CM_PRESET_Y_NUM;
int button_a_preset_num = CM_PRESET_A_NUM;
int button_b_preset_num = CM_PRESET_B_NUM;

int vstrobe_bpm = VSTROBE_DEFAULT_BPM;
int vstrobe_enabled = 0;
int user_vstrobe_enabled = 0;





void mode__tpw__jam___init(void) {

  // set main screen to be on the top lcd display
  lcdMainOnBottom();

  videoSetMode(MODE_5_3D);

  // EXPERIMENT: 3D examples don't show this as necessary, but maybe
  //             it will fix my issue with intro mode after this 
  //             not showing the texture
  // RESULT: no difference
  // map main screen background fourth (128k) region to vram bank A
  vramSetBankA(VRAM_A_MAIN_BG_0x06060000);

  // initialize 3D bottom (touch) screen graphics subsystem
  // XXX: this is now a mode specific function, probably should reside in
  //      this file.
  gzmcpc_init_3d();

  // return to desired state
  videoSetModeSub(MODE_5_2D);

  // use vram bank c for the secondary/sub-screen(background)
  vramSetBankC(VRAM_C_SUB_BG);

  mcp_console_init(&top_screen, 
		   MCP_SUB_SCREEN,
		   0,
		   1,
		   1,
		   BgType_Text4bpp, 
		   BgSize_T_256x256, 
		   31, 
		   0);

  consoleSelect(&top_screen);

  bgSetPriority(top_screen.bgId, 0);

  bgShow(top_screen.bgId);

  // default to fully faded to black (31) not the opposite (0)
  REG_BLDY_SUB = 31;
  // fade the subscreen background to/from black, layer 3
  REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG3;

  // note: using offset=4, because 4 will be 64k offset, where 31 above is 62k
  // (thus above using only 2k? seems plausible with tiles for console text chars
  bgs3 = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 4, 0);
  // its initial priority, lowest (to emphasize lack of other enabled layers)
  // priorities 0..3, 0 highest priority
  bgSetPriority(bgs3, 2);

  bgHide(bgs3);

  // as per libnds doc on dma
  DC_FlushRange(dlavaBitmap, 256*256);
  dmaCopy(dlavaBitmap, bgGetGfxPtr(bgs3), 256*256);
  DC_FlushRange(dlavaPal, 256*2);
  dmaCopy(dlavaPal, BG_PALETTE_SUB, 256*2);
  
  bgShow(bgs3);

}

void mode__tpw__jam___top_renderer(void) {
  
  long msphase, phase;
  //  long phase_prev, phase_next;
  char stringthing[MAX_TEXT_OUTPUT_LINE_SIZE];
  int vstrobe_period_ms;

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
  if (mode_ms < TPW__JAM__BOT_BG_FADE_IN_START_MS) {
    // pre fade-in
    t_blend = 31;
  } else if (mode_ms < (TPW__JAM__BOT_BG_FADE_IN_START_MS + 
			TPW__JAM__BOT_BG_FADE_IN_DURATION_MS)) {
    // main screen bg fade-in
    t_blend = 31 - ((mode_ms - TPW__JAM__BOT_BG_FADE_IN_START_MS) * 31 / 
		    TPW__JAM__BOT_BG_FADE_IN_DURATION_MS);

  } 

  // 
  // do text fade-in
  // 
  if (mode_ms < TPW__JAM__BOT_TXT_FADE_IN_START_MS) {
    // pre fade-in
    t_font_int = 0;
  } else if (mode_ms < (TPW__JAM__BOT_TXT_FADE_IN_START_MS + 
			TPW__JAM__BOT_TXT_FADE_IN_DURATION_MS)) {
    t_font_int = (unsigned char)((int)(font_intensity) * 
			       (mode_ms - TPW__JAM__BOT_TXT_FADE_IN_START_MS) / 
			       TPW__JAM__BOT_TXT_FADE_IN_DURATION_MS);
  } 


  //
  // do fadeout, possibly overriding above
  //
  if (mode != next_mode) {
    // fade-out
    if ((mode_ms - exit_mode_ms) < TPW__JAM__BOT_BG_FADE_OUT_DURATION_MS) {
      t_blend = ((mode_ms - exit_mode_ms) * 
		      31 / TPW__JAM__BOT_BG_FADE_OUT_DURATION_MS);
    } else {
      t_blend = 31;
    }
    // only fadeout
    t_blend = MAX(t_blend, mcp_get_blend(MCP_SUB_SCREEN));

    if ((mode_ms - exit_mode_ms) < TPW__JAM__BOT_TXT_FADE_OUT_DURATION_MS) {
      t_font_int = (unsigned char)((int)(font_intensity) * 
				 (TPW__JAM__BOT_TXT_FADE_OUT_DURATION_MS - (mode_ms - exit_mode_ms)) / 
				 TPW__JAM__BOT_TXT_FADE_OUT_DURATION_MS);
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


  // XXX: moving this hear causes top screen render lockup after server connect?????
  //      (a couple other lines of the same code is below)
  //  if (time_val_compare(next_top_render, num_ticks) <= 0) {
  //    return;
  //  }


  // XXX: all below might better be done once at modification time
  textout(TLT_FX_MODE, "%s", 
  	  rak_preset_labels[cstate_preset - 1]);

  textout(TLT_TOUCH_VALXP, "%s", 
  	  get_rak_midi_label(touch_whammy_x_midi_parm));

  textout(TLT_TOUCH_VALYP, "%s", 
    	  get_rak_midi_label(touch_whammy_y_midi_parm));


#define VSTROBE_NUM_CHARS 22

  // XXX could do this calculation at vstrobe_bpm modification time instead
  //     of every frame
  vstrobe_period_ms = (int)(
			    (
			     1.0f / 
			     (((float)vstrobe_bpm / 
			       (float)VSTROBE_NUM_CHARS) / 60.0f)
			     ) * 1000.0f
			    );

  // XXX and these as well
  textout(TLT_TOUCH_VAL, 
	  ":X=%02d:/\\/:Y=%02d:", 
	  cstate_wpx * 100 / 128,
	  cstate_wpy * 100 / 128);

  if (!vstrobe_enabled) {
    textout(TLT_AESTHETIC, " /\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/");
  } else {

    // age related death problem here (overflow the long)
    msphase = ((num_ticks.s * 1000) + num_ticks.ms) % vstrobe_period_ms;
    
    phase = (int)(((float)msphase / 
		   (float)vstrobe_period_ms) 
		  * (float)VSTROBE_NUM_CHARS * 2.0f);
    
    if (phase >= VSTROBE_NUM_CHARS) {
      phase = (VSTROBE_NUM_CHARS) - (phase - VSTROBE_NUM_CHARS);
    }
    
    // this is the old single direction wrapping method
    //if (phase == 0) {
    //phase_prev = VSTROBE_NUM_CHARS - 1;
    //} else {
    //phase_prev = phase - 1;
    //}
    
    //if (phase == (VSTROBE_NUM_CHARS - 1)) {
    //phase_next = 0;
    //} else {
    //phase_next = phase + 1;
    //}
    
    // dmcnote: looked it up, no -1 here, max size includes null char at end
    snprintf(stringthing, MAX_TEXT_OUTPUT_LINE_SIZE,
	     " /\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/");

    stringthing[phase + 2] = '-';
    stringthing[phase + 3] = '<';
    stringthing[phase + 4] = '=';
    stringthing[phase + 5] = '>';
    stringthing[phase + 6] = '-';
    
    textout(TLT_AESTHETIC, "%s", stringthing);

  } // end if/else vstrobe_enabled

  // XXX: same experiment as above
  //  next_top_render = time_val_add_ms(next_top_render, 1000 / TOP_RENDER_RATE);

  // actually render the text output
  render_textout();

}

void mode__tpw__jam___bot_renderer(void) {

  int i, j;

  uint8 cursor_r;
  uint8 cursor_g;
  uint8 cursor_b;
  uint8 cursor_a;
  uint8 cursor_center_r;
  uint8 cursor_center_g;
  uint8 cursor_center_b;
  uint8 cursor_center_a;


  int xcomb_row_height;
  int xcomb_col_width;
  int xtouch_pos_y;
  int xtouch_pos_x;
  int xcomb_row_cursor;
  int xcomb_col_cursor;

  int tmprate;
  int tmpvolume;
  int tmppanning;

  unsigned char t_3d_int;


  //
  // handle fade with intensity variable
  //

  // initialize to unfaded value
  t_3d_int = TPW_3D_MAX_INTENSITY;

  // 
  // do text fade-in
  // 
  if (mode_ms < TPW__JAM__BOT_3D_FADE_IN_START_MS) {
    // pre fade-in
    t_3d_int = 0;
  } else if (mode_ms < (TPW__JAM__BOT_3D_FADE_IN_START_MS + 
			TPW__JAM__BOT_3D_FADE_IN_DURATION_MS)) {
    t_3d_int = (unsigned char)((mode_ms - TPW__JAM__BOT_3D_FADE_IN_START_MS) * 
			       255 / 
			       TPW__JAM__BOT_3D_FADE_IN_DURATION_MS);
  } 


  //
  // do fadeout, possibly overriding above
  //
  if (mode != next_mode) {

    if ((mode_ms - exit_mode_ms) < TPW__JAM__BOT_3D_FADE_OUT_DURATION_MS) {
      t_3d_int = (unsigned char)((TPW__JAM__BOT_3D_FADE_OUT_DURATION_MS - 
				  (mode_ms - exit_mode_ms)) * 255 / 
				 TPW__JAM__BOT_3D_FADE_OUT_DURATION_MS);
    } else {
      t_3d_int = 0;
    }
    // only fadeout
    t_3d_int = MIN(t_3d_int, x3d_intensity);
  }

  x3d_intensity = t_3d_int;

  // update comb charge matrix
  // recharge

  // 1024 is a fixed point scaling thing
  // take into account upper and lower half rows
  xcomb_row_height = 192 * 1024 / (COMB_HEIGHT - 1);
  xtouch_pos_y = (touch_pos_y * 1024) + (xcomb_row_height >> 1);
  xcomb_row_cursor = xtouch_pos_y / xcomb_row_height;
  // XXX error check on xcomb_row_cursor < COMB_HEIGHT

  xcomb_col_width = 256 * 1024 / COMB_WIDTH;
  xtouch_pos_x = (touch_pos_x * 1024);
  xtouch_pos_x += (xcomb_col_width >> 1);
  if (xcomb_row_cursor % 2) {
    // odd row, shift left half a col width
    if (xtouch_pos_x > (xcomb_col_width >> 1)) {
      xtouch_pos_x -= (xcomb_col_width >> 1);
    } else {
      xtouch_pos_x = 0;
    }
  }
  xcomb_col_cursor = xtouch_pos_x / xcomb_col_width;
  // XXX
  if (xcomb_col_cursor >= COMB_WIDTH) xcomb_col_cursor--;

  // seems to be all good
  //  textout(TLT_STATUS, ":::%d:::%d:::", xcomb_row_cursor, xcomb_col_cursor);
  // XXX: clear up inversion confusion here or elsewhere, probably all
  //      kinds of ways to make the code more readable with utility functions,
  //      and better performance being careful where values are calculated 
  //      and how often.
  if (comb_charge[(COMB_WIDTH - 1) - xcomb_col_cursor][(COMB_HEIGHT - 1) - xcomb_row_cursor] < 224) {
    //
    // play metrognome
    //

    // save standard parameters for the effect
    tmprate = sounds[SFX_METROGNOME].rate; // 1024
    tmpvolume = sounds[SFX_METROGNOME].volume; // 255
    tmppanning = sounds[SFX_METROGNOME].panning; // 127

    sounds[SFX_METROGNOME].volume = 255 * xcomb_row_cursor / COMB_HEIGHT; // 255

    for (i = 0 ; i < (COMB_WIDTH >> 1) ; i++) {
      sounds[SFX_METROGNOME].rate = (int)((float)sounds[SFX_METROGNOME].rate * 1.059f);
    }
    for (i = 0 ; i < xcomb_col_cursor ; i++) {
      sounds[SFX_METROGNOME].rate = (int)((float)sounds[SFX_METROGNOME].rate / 1.059f);
    }

    //    sounds[SFX_METROGNOME].panning = 255 - (255 * xcomb_col_cursor / COMB_WIDTH); // 127
    mmEffectEx(&sounds[SFX_METROGNOME]);
    // restore standard parameters for the effect
    sounds[SFX_METROGNOME].rate = tmprate;
    sounds[SFX_METROGNOME].volume = tmpvolume;
    sounds[SFX_METROGNOME].panning = tmppanning;
    
  }
  comb_charge[(COMB_WIDTH - 1) - xcomb_col_cursor][(COMB_HEIGHT - 1) - xcomb_row_cursor] = 255;

  // update comb charge matrix
  for (i = 0; i < COMB_WIDTH ; i++) { 
    for (j = 0; j < COMB_WIDTH ; j++) { 
      
      // decay
      if (comb_charge[i][j] >= 2) {
	comb_charge[i][j] = comb_charge[i][j] - 2;
      } else {
	comb_charge[i][j] = 0;
      }
    }
  }

  // refactor to comb_row(touch_pos_x_val) and column/y
  
  
  init_modelview_matrix();

  // NDS does this automagically, and current api won't allow it
  // clear color and depth buffers
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // maybe redundant, 0..31 for rgba
  //  glClearColor(0, 0, 0, 31);


  // draw a cool cyber-honeycomb
  // including trippy charge tracer and background effects
  draw_simple_hex_grid(COMB_WIDTH,
		       COMB_HEIGHT,
		       RGB15(x3d_intensity >> 3,
			     x3d_intensity >> 3,
			     x3d_intensity >> 3));

  // define the cursor inner/outter colors based on current state
  if (touch_whammy_enabled) {
    cursor_r = (uint8)(0.8f * cursor_intensity * 256);
    cursor_g = (uint8)(0.2f * cursor_intensity * 256);
    cursor_b = (uint8)(0.2f * cursor_intensity * 256);
    cursor_a = 0;
    cursor_center_r = 255;
    cursor_center_g = 255;
    cursor_center_b = 255;
    cursor_center_a = 255;
  } else {
    cursor_r = (uint8)(0.2f * cursor_intensity * 256);
    cursor_g = (uint8)(0.2f * cursor_intensity * 256);
    cursor_b = (uint8)(0.7f * cursor_intensity * 256);
    cursor_a = 0;
    cursor_center_r = 0;
    cursor_center_g = 0;
    cursor_center_b = 0;
    cursor_center_a = 0;
  }

  // draw touchwhammypad cursor
  draw_hex(MODEL_XMAX - model_width * touch_pos_x / 256,
	   MODEL_YMAX - model_height * touch_pos_y / 192,
	   MODEL_ZMAX - model_depth / 4,
	   CURSOR_RADIUS,
	   cursor_rotation,
	   cursor_center_r,
	   cursor_center_g,
	   cursor_center_b,
	   cursor_center_a,
	   cursor_r,
	   cursor_g,
	   cursor_b,
	   cursor_a,
	   0,
	   0,
	   0,
	   255,
	   RGB15(x3d_intensity >> 3,
		 x3d_intensity >> 3,
		 x3d_intensity >> 3));

}

void mode__tpw__jam___input_handler(void) {

  //
  // handle input
  //

  // XXX refactor into classifcation/enumeration of 'interesting events
  //     watched for', and then have function to register an event
  //     handler for each.  I.e. layer of abstraction between tasks and
  //     the input event they are mapped to (then expose that in configfile,
  //     and/or config menu)

  // At the outermost layer of this nest, are all combinations of modifiers
  // I.e. heldkey combinations that are meant to do interesting things. 
  // After those cases are handled, then the normal non-modified cases are
  // handled, and the assumption is that if we are in a modifier situation,
  // we only want that code to be looked at.
  // note: potential exception: touchpad held perhaps should not interfere
  //       simultaneous other inputs.
  if ((heldkeys & KEY_L) && (heldkeys & KEY_R)) {
    //
    // held(L)+held(R)+something
    //
    
    //: The adjustable metrognome is currently the pro only feature.
    //: Please, do what you can to help me scrape by enough dough
    //: so that I can add more and more features to the pro version
    //: which will find their way into the GPL version after no more
    //: than 7 years (and probably much MUCH sooner than that).
#ifdef IM_POOR_OR_LICENSED_OR_NOT
    if (downkeys & KEY_LEFT) {
      // XXX no limits yet
      astrobe_bpm -= 1;
      textout(TLT_STATUS, "metrognome bpm: %d", astrobe_bpm);
      // XXX tired, is this really useful??
      //	heldover = IH_METROGNOME_DEC;
      // XXX? constant: IH_METROGNOME_HELDOVER_RATE_HZ = 10?
      heldover_sunset = time_val_add_ms(num_ticks, 224);
    }
    if (downkeys & KEY_RIGHT) {
      astrobe_bpm += 1;
      textout(TLT_STATUS, "metrognome bpm: %d", astrobe_bpm);
      //	heldover = IH_METROGNOME_INC;
      heldover_sunset = time_val_add_ms(num_ticks, 224);
    }
#endif //#ifdef  IM_POOR_OR_LICENSED_OR_NOT
    
    if (downkeys & KEY_UP) {
      textout(TLT_DEBUG, 
	      "mf:%d:mu:%d:",
	      get_mem_free(),
	      get_mem_used());
      textout(TLT_MODE, 
	      "mf:%d:mu:%d:",
	      get_mem_free(),
	      get_mem_used());
    }
    
    if (downkeys & KEY_DOWN) {
    }

    if (downkeys & KEY_X) {
      // experimental mode transition
      system_xmode_new(MODE_INTRO__MAIN);
      // XXX: perhaps imp fadeout(MODE_INTRO__MAIN), which sets mode vars, 
      //      and idlefunc handles the fade
    }

    if (downkeys & KEY_Y) {
      // experimental mode transition
      system_xmode_new(MODE_SSID__INPUT);
    }
    
    if (downkeys & KEY_A) {
      system_xmode_new(MODE_GET_UPDATE);
    }

    if (downkeys & KEY_B) {
      overwrite_self_with_update = 1;
      system_xmode_new(MODE_GET_UPDATE);
    }

    
    if (downkeys & KEY_START) {
      if (vstrobe_enabled) {
	textout(TLT_STATUS, "strobe disabled");
	vstrobe_enabled = 0;
	user_vstrobe_enabled = 1;
      } else {
	textout(TLT_STATUS, "strobe enabled");
	vstrobe_enabled = 1;
	user_vstrobe_enabled = 1;
      }
    }
    
    if (downkeys & KEY_SELECT) {
      if (astrobe_enabled) {
	textout(TLT_STATUS, "metrognome disabled");
	astrobe_enabled = 0;
      } else {
	textout(TLT_STATUS, "metrognome enabled");
	astrobe_enabled = 1;
	next_metrognome_play = time_val_add_ms(num_ticks, 1000 * 60 / astrobe_bpm);
      }
    }

    // this is temporary anyway probably...
    // only respect held(LR)+L|R for vstrobe bpm +/- if the user has toggled it at least once
    if (user_vstrobe_enabled) {
      
      if (downkeys & KEY_DOWN) {
	// XXX no limits yet
	vstrobe_bpm -= 1;
	textout(TLT_STATUS, "strobe bpm: %d", vstrobe_bpm);
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
      if (downkeys & KEY_UP) {
	vstrobe_bpm += 1;
	textout(TLT_STATUS, "strobe bpm: %d", vstrobe_bpm);
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
      
    } // end if (user_vstrobe_enabled)
    
  } else if (heldkeys & KEY_L) {
    //
    // held(L)+something
    //
    
    // theory: held(L)+ABXY is mirrorly as nice as held(R)+LRUD
    // (i.e. inverse of those should be rarely used UI things)
    
    if (downkeys & KEY_X) {
      // XXX need to get MAX_PRESET from server
      if (cstate_preset < MAX_PRESETS) {
	cstate_preset += 1;
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
    }
    
    if (downkeys & KEY_B) {
      if (cstate_preset > 1) {
	cstate_preset -= 1;
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
    }
    
    if (downkeys & KEY_UP) {
      debug_tweak_var++;
      textout(TLT_STATUS, "dtv:%d:", debug_tweak_var);
    }
    
    if (downkeys & KEY_DOWN) {
      debug_tweak_var--;
      textout(TLT_STATUS, "dtv:%d:", debug_tweak_var);
    }

    if (downkeys & KEY_START) {
    }
    
  } else if (heldkeys & KEY_R) {
    //
    // held(R)+something
    //
    
    if (downkeys & KEY_LEFT) {
      cursor_intensity -= 0.05f;
      if (cursor_intensity < 0.2f) cursor_intensity = 0.2f;
      heldover_sunset = time_val_add_ms(num_ticks, 100);
    }
    if (downkeys & KEY_RIGHT) {
      cursor_intensity += 0.05f;
      if (cursor_intensity > 0.8f) cursor_intensity = 0.8f;
      heldover_sunset = time_val_add_ms(num_ticks, 100);
    }
    
    if (downkeys & KEY_UP) {
      if (font_intensity < FONT_INTENSITY_MAX) font_intensity++;
      BG_PALETTE_SUB[255] = RGB15(font_intensity,
				  font_intensity,
				  font_intensity);
      heldover_sunset = time_val_add_ms(num_ticks, 422);
    }
    if (downkeys & KEY_DOWN) {
      if (font_intensity > FONT_INTENSITY_MIN) font_intensity--;
      BG_PALETTE_SUB[255] = RGB15(font_intensity,
				  font_intensity,
				  font_intensity);
      heldover_sunset = time_val_add_ms(num_ticks, 422);
    }
    
  } else {
    //
    // no interesting modifer keys held
    //
    
    if (downkeys & KEY_LEFT) {
      if (touch_whammy_x_midi_parm > 0) {
	touch_whammy_x_midi_parm--;
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
    }
    
    if (downkeys & KEY_RIGHT) {
      if (touch_whammy_x_midi_parm < 127) {
	touch_whammy_x_midi_parm++;
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
    }
    
    if (downkeys & KEY_DOWN) {
      if (touch_whammy_y_midi_parm > 0) {
	touch_whammy_y_midi_parm--;
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
    }
    
    if (downkeys & KEY_UP) {
      if (touch_whammy_y_midi_parm < 127) {
	touch_whammy_y_midi_parm++;
	heldover_sunset = time_val_add_ms(num_ticks, 224);
      }
    }
    
    
    if (downkeys & KEY_START) {
    } // end if KEY_START
    
    if (downkeys & KEY_SELECT) {
      if (touch_whammy_enabled) {
	touch_whammy_enabled = 0;
      } else {
	touch_whammy_enabled = 1;
      }
    }
    
    // IMP TODO: need to get a name to preset hash with overridable values in defaults file...
    if (downkeys & KEY_X) {
      cstate_preset = button_x_preset_num;
    }
    
    if (downkeys & KEY_Y) {
      cstate_preset = button_y_preset_num;
    }
    
    if (downkeys & KEY_A) {
      cstate_preset = button_a_preset_num;
    }
    
    if (downkeys & KEY_B) {
      cstate_preset = button_b_preset_num;
    }
    
  } // end button input handling


  // touchpad handling is independent of modifiers (at the moment)
  if ((downkeys & KEY_TOUCH) || (heldkeys & KEY_TOUCH)) {
    //
    // touchpad modifier, or slightly special, just downkey on the touchpad
    //
    
    // the range seems to be x 253.0 y 189.0, 
    // which is probably really 0-255,0-191
    // touchdata.px scans left to right on the screen
    // but I want it inverted, as the x whammy parm gets ramped up
    // toward the nut, or ramped down toward the bridge
    touch_pos_x = 255 - touchdata.px;
    // touchdata.py scans top to bottom on the screen
    // which is what I want as that means increasing as you stroke
    // up toward your face (which is down on the NDS)
    touch_pos_y = touchdata.py;
    
    // could have some ugly non float case logic to handle spreading
    // out the couple lost values as we really end up maybe a few values
    // shy of the full range on upper and lower (2..253) (4..189)
    // (that was right after I calibrated, which didn't seem to help)
    cstate_wpx = (int)(touch_pos_x / 2);
    cstate_wpy = (int)(touch_pos_y * 4 / 3 / 2);
    
  } // end touchpad input handling

}


void mode__tpw__jam___idle(void) {
  if (cursor_rotating) {
    cursor_rotation -= 1.23f;	
    if (cursor_rotation < 0.0f) {
      cursor_rotation += 360.0f;
    }
  }

  //
  // handle fadeout
  //
  if (mode != next_mode) {
    if (((mode_ms - exit_mode_ms) > TPW__JAM__TOP_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > TPW__JAM__BOT_BG_FADE_OUT_DURATION_MS) &&
	((mode_ms - exit_mode_ms) > TPW__JAM__BOT_TXT_FADE_OUT_DURATION_MS)) {
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
		       (u16*)bgGetGfxPtr(bgs1),
		       256 * 512 / 2);

      dmaFillHalfWords(0,
		       (u16*)bgGetGfxPtr(bgs0),
		       256 * 256 / 2);
      */
      system_xmode_real();
    }
  }

  if (mode == next_mode) {
  }

}


void mode__tpw__jam___exit(void) {

}


/*
  BCngU-0.1~

  B maj, lift bar/s1(thin).  then lift s2 and go to s543, then open,
  then c, starting with middle, and upper, then lower
  various n, n-1, n-2 repeat4, repeat3, then n-3, -2, n-1, and mz*pp
 */
