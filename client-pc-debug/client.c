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
#
#
# usage: gzmcp-client-debug.x86
#
# The Guitar-ZyX Master Control Program Debug Client listens on the localhost
# at the udp port specified by GZMCP_DEF_UDP_PORT or 24642 for a gzmcp
# server to broadcast its presence.  Upon identification of gzmcp server
# a command connection will be initiated with it, and a special command
# syntax entered at the client's stdin, will cause gzmcp commands to be
# sent to the server, just as the real NDS client does.
#
# The acceptable commands are >>>
#
# preset <preset int>
# parameter <parameter int> <value int>
# exit(or quit)
#
# note: for these effectively midi commands, channel 1 is used.
#
*/



#include <unistd.h>

#include <sys/types.h> 

#include <sys/wait.h>

#include <stdio.h>

#include <stdlib.h>

#include <stdarg.h>

#include <errno.h>
#include <limits.h>

#include <string.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>

#include <math.h>

#include <alsa/asoundlib.h>

#include <signal.h>


#include "client.h"




char prog_name[PATH_MAXLEN + 1];
char prog_dir[PATH_MAXLEN + 1];

int cmd_socket_fd;

long magic_cookie;



int 
main(int argc, char *argv[]) {

  // port number specified by the user on the commandline
  long bcast_portnum;
  long cmd_portnum;

  // server net address
  // for standard socket address structure 
  struct sockaddr_in server_address;


  //
  // derived global 'constants'
  //
  dirname(prog_dir, argv[0]);
  basename(prog_name, argv[0]);

  //
  // Hello User
  //
  fprintf(stdout, "\n\n\n");
  fprintf(stdout, 
	  "==================================================\n");
  fprintf(stdout, 
	  "Guitar-ZyX Master Control Program X86 Debug Client\n");
  fprintf(stdout, 
	  " - v2k9.dev - (c)2009 Douglas McClendon\n");
  fprintf(stdout, 
	  "==================================================\n");
  fprintf(stdout, "\n\n\n");
  fprintf(stdout, 
	  "==================================================\n");
  fprintf(stdout, 
	  "\nvalid gzmcp commands\n--------------------\n\n");
  fprintf(stdout, 
	  ">>> set-preset <new_preset(int)>\n\n");
  fprintf(stdout, 
	  ">>> set-parameter <parameter_id(int)> <value(int)>\n\n");
  fprintf(stdout, 
	  "==================================================\n");
  fprintf(stdout, "\n\n\n");

  //
  // set defaults
  //
  cmd_portnum = GZMCP_DEF_TCP_PORT;
  bcast_portnum = GZMCP_DEF_UDP_PORT;

  //
  // parse commandline
  //
  // TODO: add better getopt-ish, with --verbose...
  if (argc > 2) {
    // too many args: tell the user how to use
    usage();
  } else if (argc == 2) {
    // get the user specified port number from the command line arg
    cmd_portnum = ec_strtol(argv[1]);
    bcast_portnum = ec_strtol(argv[1]);
  } else {
    // defaults are fine
  } // end parsing commandline

  // put the global handshake cookie where it can be sizeof()d
  magic_cookie = (long)GZMCP_DEF_MAGIC_COOKIE;

  //
  // wait_for_bcast: wait for a server broadcast, retrieving the address
  //
  wait_for_bcast(bcast_portnum,
		 &server_address);

  //
  // connect to the server, exchange handshake
  //
  connect_to_server(cmd_portnum,
		    server_address);

  //
  // get_and_handle_user_commands: prompt user for input and handle
  //
  get_and_handle_user_commands();

  //
  // network_shutdown: clean up network resources
  //
  network_shutdown();

  //
  // done
  //
  return(0);

} // end main





void usage(void) {
  fprintf(stderr, "\n\nusage: gzmcpd [<port number> default=24642]\n\n\n");
  exit(1);
}



long ec_strtol(char *str) {

  char *endptr;
  long val;

  // error check strtol 
  // note: code verbatim from man strtol
  // To distinguish success/failure after call 
  errno = 0;    
  val = strtol(str, &endptr, 10);
    
  // Check for various possible errors 
  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
      || (errno != 0 && val == 0)) {
    perror("strtol");
    exit(EXIT_FAILURE);
  }
    
  if (endptr == str) {
    fprintf(stderr, "No digits were found\n");
    exit(EXIT_FAILURE);
  }
  
  // If we got here, strtol() successfully parsed a number 
  return val;
}



void basename(char *dest, char *path) {

  char *new_result;
  char *last_result;


  // look for the last '/'
  last_result = path;
  while ((new_result = strstr(last_result, "/")) != NULL) {
    last_result = new_result + 1;
  }

  // copy to dest, everything after the last '/'
  strncpy(dest, last_result, PATH_MAXLEN);

  // guarantee null terminated destination string
  dest[PATH_MAXLEN] = 0;

  return;
}



void dirname(char *dest, char *path) {

  char *new_result;
  char *last_result;
  int dirname_length;


  // look for the last '/'
  last_result = path;
  while ((new_result = strstr(last_result, "/")) != NULL) {
    last_result = new_result + 1;
  }

  // calculdate the length of the result we want
  dirname_length = strlen(path) - strlen(last_result) - 1;

  if (dirname_length > PATH_MAXLEN) {
    die("dirname() result greater than PATH_MAXLEN - %d\n",
	PATH_MAXLEN);
  }

  // copy to dest, everything up to last '/'
  strncpy(dest, 
	  path, 
	  dirname_length);

  // guarantee null terminated destination string
  dest[PATH_MAXLEN] = 0;

  return;
}



void status(const char *msg_fmt, ...) {

  char print_msg[STATUS_MSG_MAXLEN + 1];

  va_list msg_arg;


  // standard printf style variable argument processing...
  va_start(msg_arg, msg_fmt);
  vsnprintf(print_msg, 
	    STATUS_MSG_MAXLEN,
            msg_fmt, 
	    msg_arg);
  va_end(msg_arg);

  fprintf(stdout, 
	  "%s: %s\n", 
	  prog_name,
	  print_msg);
}



void die(const char *msg_fmt, ...) {

  char print_msg[STATUS_MSG_MAXLEN + 1];

  va_list msg_arg;


  // standard printf style variable argument processing...
  va_start(msg_arg, msg_fmt);
  vsnprintf(print_msg, 
	    STATUS_MSG_MAXLEN,
            msg_fmt, 
	    msg_arg);
  va_end(msg_arg);

  fprintf(stdout, 
	  "%s: FATAL ERROR: %s\n", 
	  prog_name,
	  print_msg);
  exit(1);

}



void wait_for_bcast(int portnum,
		    struct sockaddr_in* address) {

  // file descriptor for the server brodcast udp socket
  int bcast_socket_fd;

  int found_server = 0;

  int bytes_received;
  unsigned int from_length;

  // for setsockopt
  int optval;

  // buffer to receive broadcast messages
  char beacon_msg[GZMCP_BEACON_MSG_MAXLEN + 1];


  status("listening for server broadcasts on udp port %d\n",
	 portnum);

  status("listening for magic cookie - 0x%08x\n",
	 magic_cookie);

  // create the broadcast reception udp socket
  //   - AF_INET means ipv4, vs AF_INET6 vs AF_UNIX/AF_LOCAL
  //   - SOCK_DGRAM means udp, vs SOCK_STREAM for tcp
  //   - 0 means default protocol type for family
  bcast_socket_fd = socket(AF_INET, 
			   SOCK_DGRAM,
			   0);
  if (bcast_socket_fd < 0) {
    perror("socket()");
    die("couldn't create broadcast socket");
  }

  // enable broadcast
  // note: perhaps this is unnecessary, since we only receive
  optval = 1;
  if (setsockopt(bcast_socket_fd, 
		 SOL_SOCKET, 
		 SO_BROADCAST, 
		 &optval, sizeof(optval)) < 0) {
    perror("setsockopt()");
    die("failed to set broadcast socket options\n");
  }


  // create a destination address structure, initialize...
  memset(address, 0, sizeof(*address));

  // configure the address structure for our purposes
  // INET, vs e.g. UNIX for local non network sockets
  address->sin_family = AF_INET;
  address->sin_port = htons(portnum);
  // ???
  address->sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(bcast_socket_fd, (struct sockaddr *) address,
    	   sizeof(*address)) < 0) {
    perror("bind()");
    die("failed to bind to broadcast socket");
  }
  
  //
  // listen for broadcasts until a gzmcp server is discovered
  //
  while(!found_server) {

    // initialize the length of the address result
    from_length = sizeof(struct sockaddr_in);

    bytes_received = recvfrom(bcast_socket_fd,
			      beacon_msg,
			      GZMCP_BEACON_MSG_MAXLEN,
			      0,
			      (struct sockaddr*)address,
			      &from_length);
    //    status("debug: bytes_received for beacon was %d\n", 
    //	   bytes_received);
    if (bytes_received < 0) {
      // udp makes no guarantees
      //      perror("sendto()");
      //      status("could not send message\n");
    } else {
      // check if message is a gzmcp server broadcast message
      if (bcmp((void *)beacon_msg, 
	       (void *)&magic_cookie, 
	       4) == 0) {
	// a correct magic cookie
	status("server at %s had the right magic cookie!\n", 
	       inet_ntoa(address->sin_addr));
	found_server = 1;

      } else {
	// not a correct magic cookie
	status("got an incorrect magic cookie! ignoring...\n");
      }

    } // end handling received possible cookie data

  } // end while(1)

} // end wait_for_bcast()



void do_handshake(int cmd_socket_fd) {

  // for read/write return values
  int numbytes;

  // for outgoing handshake
  gzmcp_cmd handshake_cmd;
  // for incoming handshake
  gzmcp_cmd reflected_handshake_cmd;


  // create handshake command/packet
  handshake_cmd.type = GZMCP_CMD_HANDSHAKE;
  memcpy((void *)handshake_cmd.data.handshake.string, 
	 (void *)&magic_cookie,
	 sizeof(magic_cookie));
  
  // send the handshake
  numbytes = send(cmd_socket_fd,
		  (void *)&handshake_cmd,
		  sizeof(gzmcp_cmd), 
		  0);
  // check for errors
  if (numbytes < 0) {
    perror("send()");
    die("handshake send");
  }
  
  //  status("debug: handshake sent, numbytes is %d, sizeof handshake cmd is %d\n",
  //	 numbytes, sizeof(gzmcp_cmd));
  if (numbytes == sizeof(gzmcp_cmd)) {
    status("handshake sent\n");
  } else {
    die("failed to send handshake");
  }

  // recieve the reciprocal handshake
  numbytes = recv(cmd_socket_fd,
		  (void *)&reflected_handshake_cmd,
		  sizeof(gzmcp_cmd), 
		  0);
  // check for errors
  if (numbytes < 0) {
    perror("recv()");
    die("reflected handshake reception");
  }
  
  //  status("debug: reflected handshake received, numbytes is %d, sizeof handshake is %d\n",
  //	 numbytes, sizeof(gzmcp_cmd));
  
  if (numbytes == sizeof(gzmcp_cmd)) {
    
    // check reflected handshake against the original
    if (memcmp((void*)&reflected_handshake_cmd,
	       (void*)&handshake_cmd,
	       sizeof(gzmcp_cmd)) == 0 ) {
      status("correct handshake received\n");
    } else {
      die("incorrect handshake received");
    }

  } else {
    // TODO(?) receive rest of partial handshake
    die("received partial handshake back");
  }

}



void set_preset(int cmd_socket_fd, 
		char *preset_string) {

  // packet structure for sending commands to the server
  gzmcp_cmd newcmd;

  // for send() return values
  int numbytes;


  // create new command/packet
  newcmd.type = GZMCP_CMD_PRESET;
  newcmd.data.preset.channel = 1;
  newcmd.data.preset.preset = ec_strtol(preset_string);
  
  status("setting preset to %d ...\n", 
	 newcmd.data.preset.preset);
  
  // send the new command
  numbytes = send(cmd_socket_fd,
		  (void *)&newcmd,
		  sizeof(gzmcp_cmd), 
		  0);
  // check for errors
  if (numbytes < 0) {
    perror("send()");
    die("set-preset command sending failed");
  }
  
  // debug
  status("debug: set-preset sent, numbytes is %d\n",
	 numbytes);
  
  if (numbytes == sizeof(gzmcp_cmd)) {
    status("set-preset command sent\n");
  } else {
    die("failed to send set-preset command");
  }

}



void set_parameter(int cmd_socket_fd, 
		   char *parameter_string,
		   char *value_string) {

  // packet structure for sending commands to the server
  gzmcp_cmd newcmd;

  // for send() return values
  int numbytes;


  // create new command/packet
  newcmd.type = GZMCP_CMD_PARAMETER;
  newcmd.data.parameter.channel = 1;
  newcmd.data.parameter.parameter = ec_strtol(parameter_string);
  newcmd.data.parameter.value = ec_strtol(value_string);
  
  status("setting parameter %d to value %d ...\n", 
	 newcmd.data.parameter.parameter,
	 newcmd.data.parameter.value);
  
  // send the new command
  numbytes = send(cmd_socket_fd,
		  (void *)&newcmd,
		  sizeof(gzmcp_cmd), 
		  0);
  // check for errors
  if (numbytes < 0) {
    perror("send()");
    die("set-parameter command sending failed");
  }
  
  // debug
  status("debug: set-parameter sent, numbytes is %d\n",
	 numbytes);
  
  if (numbytes == sizeof(gzmcp_cmd)) {
    status("set-parameter command sent\n");
  } else {
    die("failed to send set-parameter command");
  }
      
}



void get_update(int cmd_socket_fd, 
		char *destination_filename) {

  // to store the retrieved update
  FILE *update_file;

  // packet structure for sending commands to the server
  gzmcp_cmd newcmd;

  // for send() return values
  int numbytes;
  
  // for reading the update file
  int bytes_to_read;
  unsigned char read_buf[UPD_RD_BUF_SZ];

  // for magic return value checking
  unsigned char magic_found;
  unsigned char last_byte;
  unsigned char second_to_last_byte;

  // for returned signature
  uint32_t update_signature[3];


  // create new command/packet
  newcmd.type = GZMCP_CMD_GETUPDATE;
  // TODO: check for existing cached copy, and set these appropriately
  newcmd.data.getupdate.size = 0;
  newcmd.data.getupdate.hash = 0;
  newcmd.data.getupdate.sample_data = 0;
  
  status("getting update as file %s...\n",
	 destination_filename);
  
  // send the new command
  numbytes = send(cmd_socket_fd,
		  (void *)&newcmd,
		  sizeof(gzmcp_cmd), 
		  0);
  // check for errors
  if (numbytes < 0) {
    perror("send()");
    die("get-update command sending failed");
  }
  
  // debug
  status("debug: get-update sent, numbytes is %d\n",
	 numbytes);
  
  if (numbytes == sizeof(gzmcp_cmd)) {
    status("get-update command sent\n");
  } else {
    die("failed to send set-parameter command");
  }
      
  //
  // receive update file from server
  //

  // discard incoming data until magic is found
  // todo: this could be a generic function with magic and socket_fd as arguments
  status("waiting for get-update return magic...\n");
  magic_found = 0;
  last_byte = 0;
  second_to_last_byte = 0;
  while (!magic_found) {
    second_to_last_byte = last_byte;
    // recieve a byte
    numbytes = recv(cmd_socket_fd,
		    (void *)&last_byte,
		    1, 
		    0);
    // check for errors
    if (numbytes < 0) {
      perror("recv()");
      die("get_update: waiting for return magic");
    }
  
    if ((last_byte == 24) && 
	(second_to_last_byte == 42)) {
      status("get_update: got correct return magic\n");
      magic_found = 1;
    } else {
      status("get_update: haven't yet gotten correct return magic\n");
    }

  } // end while (!magic_found)

  // read file size, signature, and sample data
  numbytes = recv(cmd_socket_fd,
		  (void *)&update_signature,
		  4 * 3, 
		  0);
  // check for errors
  if (numbytes < 0) {
    perror("recv()");
    die("get_update: waiting for return signature");
  } else if (numbytes != (4 * 3)) {
    die("get_update: return signature incomplete %d/12 bytes received",
	numbytes);
  }

  bytes_to_read = update_signature[0];

  status("get_update: return signature received: update is %d bytes\n",
	 bytes_to_read);
  
  // open output file
  update_file = fopen(destination_filename, "w");

  if (update_file == NULL) {
    status("get-update failed: cannot open update destination - %s - file for writing\n");
    return;
  }

  // read up to 1kb at a time until done
  bytes_to_read = update_signature[0];
  while (bytes_to_read) {
    numbytes = recv(cmd_socket_fd,
		    (void *)read_buf,
		    UPD_RD_BUF_SZ, 
		    0);
    // check for errors
    if (numbytes < 0) {
      perror("recv()");
      die("get_update: while getting update file data");
    }

    // write data to file
    fwrite(read_buf,
	   1,
	   numbytes,
	   update_file);

    // update amount of work left to do
    bytes_to_read -= numbytes;

  }


  // close output file
  fclose(update_file);

}



void connect_to_server(int portnum, struct sockaddr_in server_address) {

  // return values
  int rv;


  // create the command dispatch tcp socket
  // open an internet stream socket to the server
  cmd_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (cmd_socket_fd < 0) {
    perror("socket()");
    die("failed to open server socket");
  }

  status("connecting to server at port %d...\n", 
	 portnum); 

  server_address.sin_port = htons(portnum);

  rv = connect(cmd_socket_fd,
	       (struct sockaddr *)&server_address,
	       sizeof(server_address));
  if (rv < 0) {
    perror("connect()");
    die("failed to connect to server\n");
  }

  // handshake with server
  do_handshake(cmd_socket_fd);

} // end connect_to_server()



void get_and_handle_user_commands(void) {

  // for user program termination request handling
  int user_exit;

  // string buffers for processing user input
  char user_cmd_string[USER_CMD_MAXLEN + 1];
  char user_cmd[USER_CMD_MAXLEN + 1];
  char user_cmd_arga[USER_CMD_MAXLEN + 1];
  char user_cmd_argb[USER_CMD_MAXLEN + 1];
  char user_cmd_argc[USER_CMD_MAXLEN + 1];
  // number of fields in user input
  int num_items;


  // TODO: no supported exit method exists yet
  user_exit = 0;

  // handle commands given by user at stdin
  while (!user_exit) {

    // prompt for a user command
    status("enter a command to send to the server\n");
    fprintf(stdout, "\ngzmcp command >>> ");

    // read a user command
    if(fgets(user_cmd_string, USER_CMD_MAXLEN, stdin) == NULL) {
      // input from terminal ceased
      user_exit = 1;
      status("end of user input reached, goodbye...\n");
      exit(0);
    }

    // output an end of line
    fprintf(stdout, "\n\n");

    // extract command type and arg fields
    num_items = sscanf(user_cmd_string, 
		       "%s %s %s %s",
		       user_cmd,
		       user_cmd_arga,
		       user_cmd_argb,
		       user_cmd_argc);
    
    // handle possible lack of command
    if (num_items == 0) { // no command input
      status("user input empty? try again...\n");
      continue;
    } 
    
    // check for and handle each known command
    if ((strncmp(user_cmd, "exit", strlen(user_cmd)) == 0 ) ||
	(strncmp(user_cmd, "quit", strlen(user_cmd)) == 0 )) {

      if (num_items != 1) {
	status("command ignored: %s takes no arguments\n",
	       user_cmd);
	continue;
      } else {
	user_exit = 1;
      }

    } else if (strncmp(user_cmd, "send-handshake", strlen(user_cmd)) == 0 ) {

      if (num_items != 1) {
	status("command ignored: %s takes no arguments\n",
	       user_cmd);
	continue;
      } else {
	do_handshake(cmd_socket_fd);
      }

    } else if ((strncmp(user_cmd, "set-preset", strlen(user_cmd)) == 0 )) {

      if (num_items != 2) {
	status("command ignored: %s takes exactly one argument\n",
	       user_cmd);
	continue;
      } else {
	set_preset(cmd_socket_fd, user_cmd_arga);
      }

    } else if ((strncmp(user_cmd, "set-parameter", strlen(user_cmd)) == 0 )) {

      if (num_items != 3) {
	status("command ignored: %s takes exactly two arguments\n",
	       user_cmd);
	continue;
      } else {
	set_parameter(cmd_socket_fd, user_cmd_arga, user_cmd_argb);
      }

    } else if ((strncmp(user_cmd, "get-update", strlen(user_cmd)) == 0 )) {

      if (num_items != 2) {
	status("command ignored: %s takes exactly one argument\n",
	       user_cmd);
	continue;
      } else {
	get_update(cmd_socket_fd, user_cmd_arga);
      }

    } else {

      status("unknown command - %s - try again...\n", user_cmd);

    } // end if/else handling of known user commands
    
  } // end while (!user_exit)

  // later...
  status("user requested exit, goodbye...\n");
  exit(0);

}



void network_shutdown(void) {

  shutdown(cmd_socket_fd, SHUT_RDWR);

} // end network_shutdown()


