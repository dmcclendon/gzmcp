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
## gzmcpc::debug: 'debugging' functions
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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>


#include <fat.h>


#include "configfiles.h"

#include "debug.h"




int retval = 0;

int debug_tweak_var = 9;

char debug_log_filename[256] = "off";
FILE *debug_log_file = 0;

extern u8 __end__[];
extern u8 __eheap_end[];




void debug_log_init(const char *log_filename) {
  // workaround involving opening and closing the file for each write
  snprintf(debug_log_filename, 256, log_filename);

  // sigh, this did not seem to work, maybe fflush not implemented
  // XXX exception handling
  //  debug_log_file = fopen(debug_log_filename, "a");
  // note: this never really gets closed, depending on fflush to work
}


void debug_log(const char *debug_fmt, ...) {

  va_list debug_arg;

  char output_buffer[DBG_MSG_MAX_LEN];

  // only do work if necessary
  if (!fs_avail) {
    return;
  }
  if (strncmp(debug_log_filename, "off", 256) == 0) {
    return;
  }

  va_start(debug_arg, debug_fmt);
  vsnprintf(output_buffer, DBG_MSG_MAX_LEN, 
	    debug_fmt, debug_arg);
  va_end(debug_arg);

  // write the message to the logfile
  debug_log_file = fopen(debug_log_filename, "a");
  fprintf(debug_log_file, "%s", output_buffer);
  // was using this prior to workaround of open/close for every write
  //  fflush(debug_log_file);
  fclose(debug_log_file);
}



u8* get_heap_start() {
  return __end__;
}
u8* get_heap_end() {
  return (u8*)sbrk(0);
}
u8* get_heap_limit() {
  return __eheap_end;
}

int get_mem_used() {    
  struct mallinfo info = mallinfo();
  return info.uordblks;
}

int get_mem_free() {    
  struct mallinfo info = mallinfo();
  return info.fordblks + (get_heap_limit() - get_heap_end());
}


void die(void) {
  while (1);
}
