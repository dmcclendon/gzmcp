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
## gzmcpc::metrognome: metrognome functions
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


#include "metrognome.h"

#include "sound.h"

#include "time.h"


#include "sounds.h"

#include "sounds_bin.h"




int astrobe_bpm = ASTROBE_DEFAULT_BPM;
int astrobe_enabled = 0;

time_val next_metrognome_play = {0, 0};




void play_metrognome(void) {

  if (astrobe_enabled) {
    // play metrognome
    mmEffectEx(&sounds[SFX_METROGNOME]);
  }

}
