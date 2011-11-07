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
## gzmcpc::libdmc: Doug's Misc C Library
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
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


#include <fat.h>


#include "dmc.h"

#include "debug.h"







int dmc_fs_cp(const char *src,
	   const char *dst) {

  unsigned char *cp_buf;
  FILE *src_file;
  FILE *dst_file;
  int done = 0;
  int bytes_read = -1;
  int bytes_written = -1;

  // alloc a buffer
  cp_buf = malloc(DMC_FS_CP_BUF);
  if (cp_buf == NULL) {
    return -1;
  }

  // delete dest if present
  unlink(dst);

  // open files
  // TODO: error check
  src_file = fopen(src, "r");
  if (src_file == NULL) {
    free(cp_buf);
    return -1;
  }
  dst_file = fopen(dst, "w");
  if (dst_file == NULL) {
    free(cp_buf);
    fclose(src_file);
    return -1;
  }

  // data copy loop
  while (!done) {

    bytes_read = fread(cp_buf,
		       1,
		       DMC_FS_CP_BUF,
		       src_file);

    // libfat workaround
    fseek(dst_file,
          0,
          SEEK_SET);
    fseek(dst_file,
          0,
          SEEK_END);

    bytes_written = fwrite(cp_buf,
			   1,
			   bytes_read,
			   dst_file);

    if (bytes_written != bytes_read) {
      // assuming no possible acceptable short writes (for now)
      die();
    }

    // check for eof condition
    if (feof(src_file)) {
      done = 1;
    }

  }

  // free resources
  fclose(src_file);
  fclose(dst_file);
  free(cp_buf);

  // success
  return 0;

}
