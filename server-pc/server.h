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
#
#############################################################################
#############################################################################
##
## gzmcp: daemon
##
#############################################################################
##
## Copyright 2007-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/




#define GZMCP_BEACON_PERIOD_USEC 7777777

#define CMD_BUF_SIZE 16

#define PATH_MAXLEN 1023
#define STATUS_MSG_MAXLEN 1023

#define UPD_WR_BUF_SZ 8192

#define UPDATE_FILENAME "gzmcp-update.nds"
#define VARLIBDIR "/var/lib/gzmcp"

#if defined MORE_DEBUG
#define debug_more(args...) status(args)
#else
#define debug_more(args...)
#endif




#include "gzmcp_protocol.h"







void usage(void);

long ec_strtol(char *str);


void basename(char *dest, char *path);

void dirname(char *dest, char *path);


void status(const char *msg_fmt, ...);

void die(const char *msg_fmt, ...);


void midi_init(void);

void midi_shutdown(void);


void network_init(void);

void broadcast(void);

void network_shutdown(void);

int wait_for_connection(void);

void handle_cmd_handshake(int client_socket_fd,
			  gzmcp_cmd cmd);
void handle_cmd_getupdate(int client_socket_fd);

void connection_handler(int client_socket_fd);

void get_and_handle_a_connection(void);

void configure_reaper(void);

void offspring_reaper(void);
