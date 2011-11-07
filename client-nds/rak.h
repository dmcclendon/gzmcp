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
# 
# gzmcpc::rak: rak header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_RAK_H
#define _NDS_CLIENT_RAK_H



#define MAX_PRESETS 80

#define PRESET_LABEL_MAXLEN 32
#define MIDI_LABEL_MAXLEN 32












void gzmcpc_init_rak(void);

const char * get_rak_midi_label(int midival);




extern char rak_midi_labels[128][MIDI_LABEL_MAXLEN];

extern char rak_preset_labels[80][PRESET_LABEL_MAXLEN];


#endif // _NDS_CLIENT_RAK_H
