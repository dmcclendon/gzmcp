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
## gzmcp: debug client (x86)
##
#############################################################################
##
## Copyright 2007-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/




#define USER_CMD_MAXLEN 1023
#define PATH_MAXLEN 1023
#define STATUS_MSG_MAXLEN 1023

#define UPD_RD_BUF_SZ 1024



#include "gzmcp_protocol.h"







void usage(void);

long ec_strtol(char *str);


void basename(char *dest, char *path);

void dirname(char *dest, char *path);


void status(const char *msg_fmt, ...);

void die(const char *msg_fmt, ...);


void wait_for_bcast(int portnum, struct sockaddr_in* address);


void do_handshake(int cmd_socket_fd);

void set_preset(int cmd_socket_fd, 
		char *preset_string);

void set_parameter(int cmd_socket_fd, 
		   char *parameter_string,
		   char *value_string);

void get_update(int cmd_socket_fd, 
		char *destination_filename);


void connect_to_server(int portnum, struct sockaddr_in server_address);

void get_and_handle_user_commands(void);

void network_shutdown(void);
