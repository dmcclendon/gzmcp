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
## gzmcpc::input: some source file
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


#include "input.h"

#include "modes.h"

#include "network.h"

#include "graphics.h"




touchPosition touchdata;
u16 heldkeys = 0;
u16 upkeys = 0;
u16 downkeys = 0;

input_held_type heldover = IH_NONE;
time_val heldover_sunset = {0, 0};

unsigned char input_poll_rate = DEFAULT_INPUT_POLL_RATE;
time_val next_input_poll = {0, 0};




void gzmcpc_poll_input(void) {

  //
  // get input data
  //

  touchRead(&touchdata);

  scanKeys();

  heldkeys = keysHeld();

  downkeys = keysDown();

  upkeys = keysUp();

}


void global_input_processor(void) {

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

    if (downkeys & KEY_START) {

      // TODO: functionify this

      if (network_state == NSM_OFFLINE) {
	
	// cursor rotating is online indicator
	cursor_rotating = 1;
	
	// begin network state machine, start starts a scan and subsequent 
	// conncection
	network_state = NSM_START;
	
      } else {
	
	// go offline
	textout(TLT_STATUS, "player disconnected");
	network_state = NSM_OFFLINE;
	
      } // end if/else (network_state == OFFLINE)
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

      if (mode == MODE_MAIN_MENU) {
	if (last_mode != MODE_NUM_MODES) system_xmode_new(last_mode);
      } else {
	system_xmode_new(MODE_MAIN_MENU);
      }

    }

  }


  // touchpad handling is independent of modifiers (at the moment)
  if ((downkeys & KEY_TOUCH) || (heldkeys & KEY_TOUCH)) {

  }

}
