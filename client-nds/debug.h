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
# gzmcpc::debug: 'debugging' functions
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/


#ifndef _NDS_CLIENT_DEBUG_H
#define _NDS_CLIENT_DEBUG_H




#define DBG_MSG_MAX_LEN 1024

#if defined MORE_DEBUG_LOGGING
#define debug_log_more(format, args...) debug_log(format, args)
#else
#define debug_log_more(format, args...)
#endif











void debug_log_init(const char *log_filename);
void debug_log(const char *debug_fmt, ...);

int get_mem_used();
int get_mem_free();

void die(void);




extern int retval;

extern int debug_tweak_var;

extern char debug_log_filename_g[256];

#endif // _NDS_CLIENT_DEBUG_H
