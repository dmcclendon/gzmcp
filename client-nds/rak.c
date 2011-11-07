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
## gzmcpc::rak: some source file
##
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/




#include <stdio.h>


#include "rak.h"



char rak_midi_labels[128][32];
char rak_preset_labels[80][32];





void gzmcpc_init_rak(void) {

  int i;

  // XXX: constant - and do for all other hardcoded values (128)
  // initialize midi parameter table
  for (i = 0 ; i < 128 ; i++) {
    snprintf(rak_midi_labels[i], MIDI_LABEL_MAXLEN, 
	     "midi-parameter-%03d", i);
  } 

  // preset names
  for (i = 0 ; i < 80 ; i++) {
    snprintf(rak_preset_labels[i], PRESET_LABEL_MAXLEN,
	     "preset:%02d:ZyX", i + 1);
  } 

}


const char * get_rak_midi_label(int midival) {
  // XXX: this needs to be pulled from server dynamically

  return rak_midi_labels[midival];

}
