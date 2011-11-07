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
## gzmcpc::main: Guitar-ZyX Master Control Program
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



#include "main.h"

#include "configfiles.h"

#include "debug.h"

#include "graphics.h"

#include "input.h"

#include "metrognome.h"

#include "mcp.h"

#include "modes.h"

#include "network.h"

#include "rak.h"

#include "sound.h"

#include "time.h"


#include "sounds.h"

#include "sounds_bin.h"




unsigned char idle_rate = DEFAULT_IDLE_RATE;
time_val next_idle = {0, 0};



int main() {	

  // battery life is not a constraint right now :)
  powerOn(POWER_ALL);

  // assumption is that we are loaded from something that starts with white
  fade_in_from_white();

  // intialize rakarrack interface state
  gzmcpc_init_rak();
  
  // read defaults from fat filesystem
  gzmcpc_read_config();
  
  // initialize sound subsystem
  gzmcpc_init_sound();
  
  // initialize application networking subsystem
  gzmcpc_init_net();
  
  // initialize global timekeeping subsystem (ms accuracy)
  gzmcpc_init_time();
  
  //
  // input-polling/event-handling/state-machine/rendering loop
  //
  while (1) {

    //
    // turn the cranks
    //

    // network state is generally orthogonal to mode state
    network_state = gzmcpc_net_next_state(network_state);

    // check for network mode state transition
    if (network_state != last_network_state) {
      // tell next network mode state it needs to init
      nsm_init = 0;
      last_network_state = network_state;
    }

    // check for mode state transition
    if (new_mode != mode) {

      // execute per mode cleanup function (for previous mode)
      if (mode != MODE_NUM_MODES) modes[mode].goodbye();

      // record time of mode start
      mode_start = num_ticks;

      // set current and last mode
      last_mode = mode;
      mode = new_mode;

      // reset system state that modes expect initialized
      system_xmode_reinit();

      // execute per mode initialization function
      modes[mode].hello();

    }

    // calculate a frequently used value
    mode_ms = ms_since(mode_start);

    // poll for user input
    if (ms_since(next_input_poll) > 0) {

      // get new input data
      gzmcpc_poll_input();

      // call global input handler
      global_input_processor();

      // call per mode input handler
      modes[mode].input_processor();

      // set alarm for next polling cycle
      next_input_poll = time_val_add_ms(next_input_poll, 1000 / input_poll_rate);

    } // end polling for user input


    // render top screen
    if (ms_since(next_top_render) > 0) {
      modes[mode].icandy_top_reindeer();
      next_top_render = time_val_add_ms(next_top_render, 1000 / top_render_rate);
    }

    // render bottom screen
    if (ms_since(next_bottom_render) > 0) {
      modes[mode].icandy_bot_reindeer();
      next_bottom_render = 
	time_val_add_ms(next_bottom_render, 1000 / bottom_render_rate);
    }

    // synchronize server
    if (ms_since(next_sync_with_server) > 0) {
      sync_with_server();
      next_sync_with_server = 
	time_val_add_ms(next_sync_with_server, 1000 / network_event_rate);
    }

    // metrognome
    if (ms_since(next_metrognome_play) > 0) {
      play_metrognome();
      // XXX? worthwhile optimization ?= calculate at astrobe_bpm 
      //      modification time, tradeoff would extra variable (or 
      //      just replace astrobe_bpm with astrobe_b_ms_period)
	next_metrognome_play = 
	  time_val_add_ms(next_metrognome_play, 1000 * 60 / astrobe_bpm);
    }

    // run idle function
    if (ms_since(next_idle) > 0) {
      mcp_fade_update();
      modes[mode].slacker_funk();
      next_idle = time_val_add_ms(next_idle, 1000 / idle_rate);
    }

    // done with frame loop, wait for vblank, etc...
    gzmcpc_flush_frame();

  } // end while(1) 

  // execution never reaches here
  return 0;

} // end main()




void fade_in_from_white(void) {
  lcdMainOnTop();
  videoSetMode(MODE_5_2D);
  vramSetBankA(VRAM_A_MAIN_BG_0x06060000);
  videoSetModeSub(MODE_5_2D);
  vramSetBankC(VRAM_C_SUB_BG);
  int bg = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
  int bgs = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
  dmaFillWords(0xFFFFFFFF, BG_PALETTE, 256*2);
  dmaFillWords(0x00000000, bgGetGfxPtr(bg), 256*256);
  dmaFillWords(0xFFFFFFFF, BG_PALETTE_SUB, 256*2);
  dmaFillWords(0x00000000, bgGetGfxPtr(bgs), 256*256);
  int counter, i;

  for (counter = 31; counter >= 0; counter--) {
    BG_PALETTE[0] = RGB15(counter, counter, counter);
    BG_PALETTE_SUB[0] = RGB15(counter, counter, counter);
    for (i = 0; i < 2; i++) {
      swiWaitForVBlank();
    }
  }
}
