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
## gzmcpc::sound: sound functions
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
#include <maxmod9.h>

#include "sound.h"


#include "sounds.h"
#include "sounds_bin.h"




mm_sound_effect sounds[MSL_NSAMPS];




void gzmcpc_init_sound(void) {

  int i;

  // initialize maxmod sound system
  mmInitDefaultMem((mm_addr)sounds_bin);

  //
  // load sound effects                                                                                  
  //

  // note, i here is being used for enumerated values, i.e.
  // things like SFX_STARTUP, SFX_ONLINE, ...
  for (i = 0 ; i < MSL_NSAMPS ; i++) {
    mmLoadEffect(i);
    // this variable(s) is used to optionally tailor playback
    sounds[i].id = i;
    sounds[i].rate = 1024;
    sounds[i].handle = 0;
    sounds[i].volume = 255;
    sounds[i].panning = 127;
  }

}


void mcp_snd_click(void) {

  int save_volume;

  save_volume = sounds[SFX_METROGNOME].volume;
  sounds[SFX_METROGNOME].volume = save_volume / 4;
  mmEffectEx(&sounds[SFX_METROGNOME]);
  sounds[SFX_METROGNOME].volume = save_volume;
}


void end_sound(void) {

  int i;

  // free all sound resources
  for (i = 0 ; i < MSL_NSAMPS ; i++) {
    mmUnloadEffect(i);
  }
}
