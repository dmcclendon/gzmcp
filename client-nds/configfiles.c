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
# gzmcpc::configfiles: configfile functions
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


#include <fat.h>


#include "mode__tpw__jam.h"

#include "configfiles.h"

#include "debug.h"

#include "network.h"

#include "rak.h"



int fs_avail = 0;


char linebuf[MAX_CONFIG_LINE_SIZE + 1];
char rest_buf[MAX_CONFIG_LINE_SIZE + 1];
char parm_id_buf[MAX_CONFIG_LINE_SIZE + 1];
char parm_name_buf[MAX_CONFIG_LINE_SIZE + 1];





void 
gzmcpc_read_config(void) {

  FILE *fs_file;

  int param_id_dec;
  int rv;

  // this is a bit gross, need to refactor configuration options
  // initialize ssid
  snprintf(ap_ssid, SSID_MAXLEN, "%s", ZYX_AP_NAME);

  // initialize filesystem access if possible
  fs_avail=fatInitDefault();
  
  if (fs_avail) {

    // TODO
    //        - (opt) default whammypad_[xy]_inverse (boolean:true/false)
    //        - (opt) default whammypad_[xy]_minimum (integer 0..127) (err, should be per parameter)
    //        - (opt) default whammypad_[xy]_maximum (integer 0..127) (err, should be per parameter)
    //        - (opt) default final/master volume (integer for now)

    //      - does ///defaults exist?
    //        - if so read it
    //	- if not, write a template
    //      - does ///midispec exist?
    //        - if so read it
    //	- if not, write a template
        
    // IDEA: have trapdoor goto, closes files if open, etc "configuration file reading failed badly"
    
    // TODO- errorcheck, mkdir, etc...
    //    fs_dir = opendir("/boot/zyx/gzmcp");

    // XXX: this is probably stupid, should just try opening
    //      files and watching return/error values for things
    //      like file not exist

    //    found_conf_file = 0;
    //    while ((fs_dirent = readdir(fs_dir)) != NULL) {
      
    ////    stat(fs_dirent->d_name, &fs_stat_info);
    ////    fs_stat_info.st_size;
      
    //      if (! S_ISDIR(fs_stat_info.st_mode)) {
    //	if (strcmp((const char *)(fs_dirent->d_name), "defaults") == 0) {
    //	  found_conf_file = 1;
    //	  break;
    //	}
    //      } // end if not a directory
      
    //    } // end iterating over directory entries


    ////
    //// defaults
    ////
    fs_file = fopen("/gzmcp/defaults", "r");
    
    while (fgets(linebuf, MAX_CONFIG_LINE_SIZE, fs_file) != NULL) {
      rv = sscanf(linebuf, 
		  "%[^=]=%[^\n]\n", 
		  parm_name_buf,
		  parm_id_buf);
      if (rv >= 2) {
	if (strncmp(parm_name_buf, "initial_effect", MAX_CONFIG_LINE_SIZE) == 0) {
	  // bah...
	} else if (strncmp(parm_name_buf, "initial_whammypad_x_parameter", MAX_CONFIG_LINE_SIZE) == 0) {
	  touch_whammy_x_midi_parm = (int)strtol(parm_id_buf, (char **) NULL, 0);
	} else if (strncmp(parm_name_buf, "initial_whammypad_y_parameter", MAX_CONFIG_LINE_SIZE) == 0) {
	  touch_whammy_y_midi_parm = (int)strtol(parm_id_buf, (char **) NULL, 0);
	} else if (strncmp(parm_name_buf, "button_a_preset_num", MAX_CONFIG_LINE_SIZE) == 0) {
	  button_a_preset_num = (int)strtol(parm_id_buf, (char **) NULL, 0);
	} else if (strncmp(parm_name_buf, "button_b_preset_num", MAX_CONFIG_LINE_SIZE) == 0) {
	  button_b_preset_num = (int)strtol(parm_id_buf, (char **) NULL, 0);
	} else if (strncmp(parm_name_buf, "button_x_preset_num", MAX_CONFIG_LINE_SIZE) == 0) {
	  button_x_preset_num = (int)strtol(parm_id_buf, (char **) NULL, 0);
	} else if (strncmp(parm_name_buf, "button_y_preset_num", MAX_CONFIG_LINE_SIZE) == 0) {
	  button_y_preset_num = (int)strtol(parm_id_buf, (char **) NULL, 0);
	} else if (strncmp(parm_name_buf, "vstrobe_bpm", MAX_CONFIG_LINE_SIZE) == 0) {
	  vstrobe_bpm = (int)strtol(parm_id_buf, (char **) NULL, 0);
	} else if (strncmp(parm_name_buf, "debug_log", MAX_CONFIG_LINE_SIZE) == 0) {
	  debug_log_init(parm_id_buf);
	} else if (strncmp(parm_name_buf, "ap_ssid", MAX_CONFIG_LINE_SIZE) == 0) {
	  snprintf(ap_ssid, SSID_MAXLEN, "%s", parm_id_buf);
	} else {
	  // unknown entry
	}
      } else {
	// bad line
      }
    } // end while (!done)

    fclose(fs_file);

    ////
    //// /defaults
    ////


    ////
    //// midispec
    ////
    fs_file = fopen("/gzmcp/midispec", "r");
    
    while (fgets(linebuf, MAX_CONFIG_LINE_SIZE, fs_file) != NULL) {
      rv = sscanf(linebuf, 
		  "midival:%[^:]:%[^:]:%[^\n]\n", 
		  parm_id_buf,
		  parm_name_buf,
		  rest_buf);
      if (rv >= 2) {
	param_id_dec = strtol(parm_id_buf, (char **) NULL, 0);
	snprintf(rak_midi_labels[param_id_dec], MAX_CONFIG_LINE_SIZE, "%s", parm_name_buf);
      } else {
	// bad line
      }
    } // end while (!done)
    
    fclose(fs_file);

    ////
    //// /midispec
    ////

    ////
    //// presets
    ////
    fs_file = fopen("/gzmcp/presets", "r");
    
    while (fgets(linebuf, MAX_CONFIG_LINE_SIZE, fs_file) != NULL) {
      rv = sscanf(linebuf, 
		  "preset:::%[^:]:::%[^\n]\n", 
		  parm_id_buf,
		  parm_name_buf);
      if (rv >= 2) {
	param_id_dec = strtol(parm_id_buf, (char **) NULL, 0);
	// XXX constants, probably conflicting (string lengths)
	snprintf(rak_preset_labels[param_id_dec - 1], 
		 MAX_CONFIG_LINE_SIZE, 
		 "%s", 
		 parm_name_buf);
      } else {
	// bad line
      }
    } // end while (!done)
    
    fclose(fs_file);

    ////
    //// /presets
    ////
  
  } // end if fs_avail

}


void rewrite_defaults_with_new_ssid(void) {

  int rv;

  FILE *defaults_file;
  FILE *tmp_defaults_file;

  if (! fs_avail) return;

  // sigh, no umask or chmod, so take pains to overwrite the original file
  // TODO: check return values of everything, for now... a little faith
  defaults_file = fopen("/gzmcp/defaults", "r");
  tmp_defaults_file = fopen("/gzmcp/defaults.tmp", "w");
  while (fgets(linebuf, MAX_CONFIG_LINE_SIZE, defaults_file) != NULL) {
      fprintf(tmp_defaults_file, "%s", linebuf);
  }
  fclose(defaults_file);
  fclose(tmp_defaults_file);

  tmp_defaults_file = fopen("/gzmcp/defaults.tmp", "r");
  defaults_file = fopen("/gzmcp/defaults", "w");
  
  while (fgets(linebuf, MAX_CONFIG_LINE_SIZE, tmp_defaults_file) != NULL) {
    rv = sscanf(linebuf, 
		"%[^=]=%[^\n]\n", 
		parm_name_buf,
		parm_id_buf);
    if (rv >= 2) {
      if (strncmp(parm_name_buf, "ap_ssid", MAX_CONFIG_LINE_SIZE) == 0) {
	// write out new entry
	fprintf(defaults_file, "ap_ssid=%s\n", ap_ssid);
      } else {
	// normal entry
	fprintf(defaults_file, "%s", linebuf);
      }
    } else {
      fprintf(defaults_file, "%s", linebuf);
    }
  } // end while (!done)

  fclose(defaults_file);
  fclose(tmp_defaults_file);

  unlink("/gzmcp/defaults.tmp");
}

