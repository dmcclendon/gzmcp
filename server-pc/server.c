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
## gzmcpc: gzmcp daemon
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
# usage: gzmcpd
#
# The Guitar-ZyX Master Control Program Daemon listens on the localhost at 
# the tcp port specified by XXX GZMCP_PORT or 24642 for a gzmcp-client to 
# start sending a command stream which will be  processed appropriately.
# A broadcast token is sent every 7 seconds on udp port 24642 to announce
# our presence to any locally attached networks.
#
*/



#include <unistd.h>

#include <sys/types.h> 

#include <sys/stat.h> 

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

#include <ifaddrs.h>

#include <math.h>

#include <alsa/asoundlib.h>

#include <signal.h>

#include "server.h"




char prog_name[PATH_MAXLEN + 1];
char prog_dir[PATH_MAXLEN + 1];

int cmd_socket_fd;

long bcast_portnum;
long cmd_portnum;

snd_seq_t *mainport;

int alsa_output_port;

char port_name[256];

int user_exit;

int user_count = 0;



int 
main(int argc, char *argv[]) {

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
	  "========================================\n");
  fprintf(stdout, 
	  "Guitar-ZyX Master Control Program Server\n");
  fprintf(stdout, 
	  " - v2k9.dev - (c)2009 Douglas McClendon\n");
  fprintf(stdout, 
	  "========================================\n");
  fprintf(stdout, "\n\n\n");

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

    // or use defaults
    cmd_portnum = GZMCP_DEF_TCP_PORT;
    bcast_portnum = GZMCP_DEF_UDP_PORT;

  }

  // start midi output port
  midi_init();

  // start listening for and taking care of clients
  network_init();

  // broadcast an availability message to locally attached networks
  broadcast();

  // configure_reaper: set up child process signal handler
  configure_reaper();

  // TODO: no supported exit method exists yet
  user_exit = 0;

  while (!user_exit) {

    get_and_handle_a_connection();
      
  } // end main loop ending on as yet specd userexit

  // stop midi output port
  midi_shutdown();

  // release network resources 
  network_shutdown();

  // be seeing you...
  return(0);
}




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

  // if no / found, dirname is
  if (last_result == path) {
    // edunote: normally I go with n of strlen(string), but this from the
    //          strncpy manpage worries me-
    // "If there is no terminating null byte in the first n characters of 
    //  src, strncpy() produces an unterminated string in dest."
    strncpy(dest, ".", 2);
    return;
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



void midi_init(void) {

  int retval;

  //
  // alsa init
  //
  retval = snd_seq_open(&mainport, 
		    "default", 
		    SND_SEQ_OPEN_OUTPUT, 
		    0);
  
  if (retval < 0) {
    printf("error starting alsa sequencer client\n");
  }
  
  snd_seq_set_client_name (mainport, "gzmcp");
  
  snd_config_update_free_global();
  
  // create alsa sequencer client
  sprintf(port_name, "gzmcp output");
  alsa_output_port =			\
    snd_seq_create_simple_port(
			       mainport, 
			       port_name,
			       SND_SEQ_PORT_CAP_READ |
			       SND_SEQ_PORT_CAP_SUBS_READ,
			       SND_SEQ_PORT_TYPE_APPLICATION
			       );

  //
  // end alsa init
  //
}


void midi_shutdown(void) {

  // alsa cleanup
  snd_seq_close(mainport);
}



void network_init(void) {

  // server net address
  // for standard socket address structure 
  struct sockaddr_in server_address;

  // for setsockopt
  int optval;


  // open an internet stream socket for the server
  cmd_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (cmd_socket_fd < 0) {
    perror("socket()");
    die("failed to open server socket");
  }
  
  // I don't like waiting 3 minutes to restart if there is a crash
  // (i.e. netstat -auntp | grep <PORTNUM> yields FIN_WAIT1),
  // thus use SO_REUSEADDR
  optval = 1;
  if (setsockopt(cmd_socket_fd, 
		 SOL_SOCKET, 
		 SO_REUSEADDR, 
		 &optval, sizeof(optval)) < 0) {
    perror("setsockopt()");
    die("failed to set socket options\n");
  }
  
  // initialize/zero-out the server_address structure
  memset((char *) &server_address, 0, sizeof(server_address));

  // configure the server_address structure for our purposes
  // INET, vs e.g. UNIX for local non network sockets
  server_address.sin_family = AF_INET;

  // INADDR_ANY here means to bind to any/all ip addresses of this host
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);

  // configure port using correct network(htons) format
  server_address.sin_port = htons(cmd_portnum);
  
  // try to bind to the socket (this is the main gzmcpd test)
  if (bind(cmd_socket_fd, (struct sockaddr *) &server_address,
	   sizeof(server_address)) < 0) {
    perror("bind()");
    die("failed to bind to server socket");
  }
  
  // listen for connections
  if (listen(cmd_socket_fd,1) < 0) {
    perror("listen()");
    die("failed to listen to server socket");
  }
}



void bcast_run_beacon(void) {

  // return values
  int rv;

  // file descriptor for the server brodcast udp socket
  int bcast_socket_fd;

  struct sockaddr_in broadcast_address;
  int bytes_sent;
  // for setsockopt
  int optval;

  long magic_cookie;
  char beacon_msg[GZMCP_BEACON_MSG_MAXLEN + 1];

  // for crafting broadcast udp packets for all local nets
  struct ifaddrs *interface_addresses;
  struct ifaddrs *ia_ptr;


  // put the cookie where it can be sizeof()d
  magic_cookie = (long)GZMCP_DEF_MAGIC_COOKIE;

  // create the broadcast udp socket
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
  optval = 1;
  if (setsockopt(bcast_socket_fd, 
		 SOL_SOCKET, 
		 SO_BROADCAST, 
		 &optval, sizeof(optval)) < 0) {
    perror("setsockopt()");
    die("failed to set broadcast socket options\n");
  }


  // create a destination address structure, initialize...
  memset(&broadcast_address, 0, sizeof(broadcast_address));

  // configure the broadcast_address structure for our purposes
  // INET, vs e.g. UNIX for local non network sockets
  broadcast_address.sin_family = AF_INET;
  broadcast_address.sin_port = htons(bcast_portnum);
  // note: saving the .sin_addr for just prior to sendto

  // note:  For now I'm going on my interpretation of this-
  //        http://www.faqs.org/rfcs/rfc947.html
  //        which very strongly implies that broadcasting
  //        on the internet is restricted to the physical 
  //        networks connected to by the host.  I.e. that one
  //        can count on routers, even non-nat ones, not forwarding
  //        broadcast udp traffic.
  //
  // note additionally: This says that 255.255.255.255 used 
  //    after enabling broadcast socket opt, is the local subnet,
  //    while e.g. 192.168.1.255 is a 'directed broadcast address',
  //    implying you can do directed broadcasts.

  //
  // prepare beacon message, i.e. magic cookie + hostname
  //
  // the beacon message starts with the magic cookie
  memcpy(beacon_msg,
	 &magic_cookie,
	 sizeof(magic_cookie));
  // and ends with the hostname
  rv = gethostname(beacon_msg + sizeof(GZMCP_DEF_MAGIC_COOKIE),
		   GZMCP_BEACON_MSG_MAXLEN - sizeof(GZMCP_DEF_MAGIC_COOKIE));
  // ensure null termination
  beacon_msg[GZMCP_BEACON_MSG_MAXLEN] = 0;

  if (rv < 0) {
    perror("gethostname()");
    die("could not create beacon message\n");
  }

  // note: ctrl-c(KILL/BRK?) to the parent does get the broadcasting child killed
  while(1) {

    // note: the below link combined with other example code I saw convinces
    //    me that this is indeed correct, i.e. the sock_addr* casting of 
    //    a sock_addr_in*.
    //    http://www.linuxquestions.org/questions/programming-9 ---
    //          /sockaddrin-and-sockaddr-176787/
    //

    // note: it is not this simple with multiple interfaces
    // http://www.developerweb.net/forum/showthread.php?t=5085
    /* 
    broadcast_address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    bytes_sent = sendto(bcast_socket_fd,
			beacon_msg,
			strlen(beacon_msg),
			0,
			(struct sockaddr *)&broadcast_address,
			sizeof(broadcast_address));
    if (bytes_sent < 0) {
      // udp makes no guarantees
      //      perror("sendto()");
      //      status("could not send message\n");
    }
    */

    // send one broadcast packet for each locally connected interface
    if (getifaddrs(&interface_addresses) == 0) {

      ia_ptr = interface_addresses;
      while (ia_ptr) {
	
	if ((ia_ptr->ifa_addr)->sa_family == AF_INET) {
	  
	  //	  printf("sending to address ... %s\n",
	  //		 inet_ntoa(((struct sockaddr_in *)(ia_ptr->ifa_dstaddr))->sin_addr));

	  broadcast_address.sin_addr.s_addr = ((struct sockaddr_in *)(ia_ptr->ifa_dstaddr))->sin_addr.s_addr;
	  bytes_sent = sendto(bcast_socket_fd,
			      beacon_msg,
			      strlen(beacon_msg),
			      0,
			      (struct sockaddr *)&broadcast_address,
			      sizeof(broadcast_address));
	  if (bytes_sent < 0) {
	    // udp makes no guarantees
	    //      perror("sendto()");
	    //      status("could not send message\n");
	  }
	
	} // end if (ia_ptr->ifa_addr->a_family == AF_INET) 

	ia_ptr = ia_ptr->ifa_next;

      } // end while (ia_ptr)      

      freeifaddrs(interface_addresses);

    } // end if getifaddrs

    usleep(GZMCP_BEACON_PERIOD_USEC);
  } // end while(1)

}


void bcast_run_beacon_and_exit(void) {

  // pid of the client child process
  pid_t child_pid = -1;


  //
  // fork
  //

  child_pid = fork();

  if (child_pid < 0) {

    perror("fork()");
    die("forking error");

  } else if (child_pid != 0) {

    //
    // parent
    //
    
    // return, reaper handler will take care of waiting for children

  } else {

    //
    // child
    //

    bcast_run_beacon();

  }

}


void broadcast(void) {

  // forking: pid of the broadcaster child process
  pid_t bcast_child_pid = -1;


  //
  // fork
  //
      
  bcast_child_pid = fork();
  
  if (bcast_child_pid < 0) {
    
    perror("ERROR: forking error");
    die("forking error");
    
  } else if (bcast_child_pid != 0) {
    
    //
    // parent
    //
    
    pid_t some_child_pid = -1;

    // make sure the child process exits
    some_child_pid = wait(NULL);
    
    if (bcast_child_pid != some_child_pid) {
      
      status("debug: bcast_child_pid was %d, some_child_pid was %d\n",
	  bcast_child_pid, 
	  some_child_pid);
      die("problem waiting for broadcast client child process to terminate\n");
      
    } else {
      
      exit(0);
      
    } 
    
  } else {
    
    //
    // child
    //
    
    bcast_run_beacon_and_exit();
    
  } // end fork handling


}



void network_shutdown(void) {

  shutdown(cmd_socket_fd, SHUT_RDWR);

}



int wait_for_connection(void) {
  
  int sockfd = -1;

  // for standard socket address structure (of the accepted test connection)
  // (note: data returned by accept is not used by gzmcpd subsequently)
  int client_length;
  struct sockaddr_in client_address;

  // client_length and client address are required for the accept syscall,
  // but are not actually used subsequently by gzmcpd.  Presumably they
  // would be used if we cared about the origin of the incoming connection.
  // (i.e. if we were getting nmap-ed at this precise time, we would see
  // a 'corrupt' test message.  An improvement, would be to discard 
  // connections not from the localhost.  But despite the verboseness of
  // this code as a personal reference, that is more than I care about now).
  client_length = sizeof(client_address);

  // accept the connection and process it
  sockfd = accept(cmd_socket_fd, 
		  (struct sockaddr *)&client_address, 
		  (socklen_t *)&client_length);

  if (sockfd < 0) {

    // just fail this connection
    perror("accept()");
    status("failed to accept on server socket");

  } else {

    status("accepted connection from client: %s\n\n",
	   inet_ntoa(client_address.sin_addr));
  }
  
  fflush(stdout);
  fflush(stderr);

  return sockfd;
}

    

void handle_cmd_handshake(int client_socket_fd,
			  gzmcp_cmd cmd) {

  // for read/write return values
  int bytes_written;


  debug_more("debug: processing a handshake command...");
  status("processing a handshake command...");

  bytes_written = write(client_socket_fd,
			(void *)&cmd,
			sizeof(gzmcp_cmd));
  
  if (bytes_written < 0) {
    perror("write()");
    status("warning: failed to write to server socket to return handshake");
  } else if (bytes_written < sizeof(gzmcp_cmd)) {
    status("BUG/FIXME: wrote partial handshake\n");
    // XXX TODO: send the rest
  } else {
    debug_more("debug: wrote complete handshake\n");
    status("debug: wrote complete handshake\n");
  }
  
}



void handle_cmd_getupdate(int client_socket_fd) {

  unsigned char magic[2];

  // for read/write return values
  int bytes_written;
  int rv;

  // for update file size...
  struct stat stat_buf;

  FILE *update_file;

  // for update file send loop
  int bytes_to_write;
  int chunk_bytes_to_write;
  unsigned char write_buf[UPD_WR_BUF_SZ];

  char update_filename[PATH_MAXLEN + 1];

  //edunote: ref to C99 standard that uint32_t is preferred: (over u_int32_t)
  // http://www.oreillynet.com/pub/a/network/2003/10/07/michael_barr.html
  uint32_t update_filesize = -1;
  // effectively a return packet type
  // todo: move to protocol or otherwise improve
  uint32_t return_data[3];

  // XXX: hardcoded magic
  
  magic[0] = 42;
  magic[1] = 24;

  debug_more("debug: processing a getupdate command...");
  
  status("checking for update");

  //
  // get filesize of update file
  //

  // first create full pathname to neighbor update file
  snprintf(update_filename, 
	   PATH_MAXLEN,
	   "%s/%s",
	   prog_dir,
	   UPDATE_FILENAME);
  rv = stat(update_filename, &stat_buf);
  if (rv != 0) {
    // finally try full pathname to systemwide update file
    snprintf(update_filename, 
	     PATH_MAXLEN,
	     "%s/%s",
	     VARLIBDIR,
	     UPDATE_FILENAME);
    rv = stat(update_filename, &stat_buf);
    if (rv != 0) {
      status("no update file available, won't send anything to client");
      return;
    }
  }
  
  update_filesize = (int)stat_buf.st_size;

  update_file = fopen(update_filename, "r");
  if (update_file == NULL) {
    die("failed to open update file %s for reading in order to send to client\n",
	update_filename);
  }

  status("update file size is %d bytes\n", 
	 update_filesize);

  status("sending return magic");

  // send magic
  bytes_written = write(client_socket_fd,
			(void *)magic,
			2);
  
  if (bytes_written < 0) {
    perror("write()");
    status("warning: failed to write to server socket to return magic");
  } else if (bytes_written < 2) {
    status("BUG/FIXME: wrote partial return magic\n");
    // XXX TODO: send the rest
  } else {
    debug_more("debug: wrote complete return magic\n");
  }
  
  status("sending update signature metadata");

  return_data[0] = update_filesize;
  // no parity/hash/xor/signature yet
  return_data[1] = 0;
  // no data sampling yet
  return_data[2] = 0;

  // 4 bytes per int32_t, 3 values in return_data[]
  bytes_written = write(client_socket_fd,
			(void *)return_data,
			4 * 3);
  
  if (bytes_written < 0) {
    perror("write()");
    status("warning: failed to write to server socket to return signature");
  } else if (bytes_written < (4 * 3)) {
    status("BUG/FIXME: wrote partial return signature\n");
    // XXX TODO: send the rest
  } else {
    debug_more("debug: wrote complete return magic\n");
  }
  
  status("sending update");

  // write up to 1kb at a time until done
  bytes_to_write = update_filesize;

  while (bytes_to_write) {

    if (bytes_to_write < UPD_WR_BUF_SZ) {
      chunk_bytes_to_write = bytes_to_write;
    } else {
      chunk_bytes_to_write = UPD_WR_BUF_SZ;
    }

    // no error checking here (yet)
    fread(write_buf, 1, chunk_bytes_to_write, update_file);

    while (chunk_bytes_to_write) {

      bytes_written = write(client_socket_fd,
			    (void *)write_buf,
			    chunk_bytes_to_write);
      
      if (bytes_written < 0) {
	perror("write()");
	die("failed to write to server socket to return update file data");
      } else if (bytes_written < chunk_bytes_to_write) {
	status("note: wrote partial update file data chunk: %d/%d bytes\n",
	       bytes_written, 
	       chunk_bytes_to_write);
      } 

      chunk_bytes_to_write -= bytes_written;
  
    } // end while (chunk_bytes_to_write)

    if (bytes_to_write < UPD_WR_BUF_SZ) {
      bytes_to_write = 0;
    } else {
      bytes_to_write -= UPD_WR_BUF_SZ;
    }

  } // end while (bytes_to_write)
  
}



void connection_handler(int client_socket_fd) {
  
  // simple data buffers for use with the socket

  // the buffer to store newly read commands from the network pipe
  // the +1 is to support storage for a partially read command
  unsigned char cmd_read_buf[(CMD_BUF_SIZE + 1) * sizeof(gzmcp_cmd)];
  int cmd_data_size;

  // a pointer to walk through the read buffer
  void *next_cmd;

  // no goto here
  int client_done;

  // for read/write return values
  int bytes_read;

  // command structure for incoming commands
  gzmcp_cmd cmd;

  // pid of the client child process
  pid_t child_pid = -1;

  // a midi event to send to a midi receiver
  snd_seq_event_t midi_event;


  //
  // fork
  //

  child_pid = fork();

  if (child_pid < 0) {

    perror("fork()");
    die("forking error");

  } else if (child_pid != 0) {

    //
    // parent
    //
    
    close(client_socket_fd);

    // return, reaper handler will take care of waiting for children

  } else {

    //
    // child
    //
    
    // initialize the reception buffers
    memset(cmd_read_buf, 0, (CMD_BUF_SIZE + 1) * sizeof(gzmcp_cmd));
    cmd_data_size = 0;
    
    // begin command processing loop
    client_done = 0;
    while (!client_done) {
      
      //
      // read command message
      //

      // read into the buffer, offset by a possible unprocessed partial command
      bytes_read = read(client_socket_fd, 
			cmd_read_buf + cmd_data_size, 
			CMD_BUF_SIZE * sizeof(gzmcp_cmd));
      if (bytes_read < 0) {
	perror("read()");
	status("error: failed to read from server socket");
	status("client disconnected, user count is now %d\n", user_count);
	status("the reaper should be knocking right about now...\n");
	client_done = 1;
	return;
      } else {
	// debug/info
	if (bytes_read != 0) {
	  debug_more("debug: received %d bytes of new command data\n", bytes_read);
	}
	// integrate the partial command with the newly read stream
	cmd_data_size += bytes_read;
      }
      

      //
      // process one or more commands
      //

      // start at the beginning
      next_cmd = cmd_read_buf;

      while (cmd_data_size >= sizeof(gzmcp_cmd)) {

	debug_more("debug: processing a command...");

	// extract the first remaining command
	memcpy((void *)&cmd,
	       next_cmd, 
	       sizeof(gzmcp_cmd));
	// increment the next command
	next_cmd += sizeof(gzmcp_cmd);
	// decrement the amount of data to be processed
	cmd_data_size -= sizeof(gzmcp_cmd);

	debug_more("debug: command type is %08x",
		   cmd.type);

	//
	// check for command type and handle appropriately
	//

	if (cmd.type == GZMCP_CMD_HANDSHAKE) {
	  handle_cmd_handshake(client_socket_fd, 
			       cmd);
	}

	if (cmd.type == GZMCP_CMD_GETUPDATE) {
	  handle_cmd_getupdate(client_socket_fd);
	}	

	// check for set-preset
	if (cmd.type == GZMCP_CMD_PRESET) {
	  
	  status("changing to preset %d on channel %d\n", 
		 cmd.data.preset.preset,
		 cmd.data.preset.channel);

	  //
	  // craft and send a midi event
	  // 
	  
	  // init structure
	  snd_seq_ev_clear(&midi_event);
	  // set things up ...
	  snd_seq_ev_set_pgmchange(&midi_event, 
				   cmd.data.preset.channel - 1, 
				   cmd.data.preset.preset);
	  snd_seq_ev_set_subs(&midi_event);
	  snd_seq_ev_set_direct(&midi_event);
	  // .. and fire off the midi event
	  snd_seq_event_output_direct(mainport, 
				      &midi_event);
	  

	} // end if preset

	// check for set-parameter
	if (cmd.type == GZMCP_CMD_PARAMETER) {
	  
	  status("changing parameter %d to value %d on channel %d\n", 
		 cmd.data.parameter.parameter,
		 cmd.data.parameter.value,
		 cmd.data.parameter.channel);

	  //
	  // craft and send a midi event
	  // 

	  // new structure
	  snd_seq_event_t midi_event;
	  // init structure
	  snd_seq_ev_clear(&midi_event);
	  // set things up ...
	  snd_seq_ev_set_controller(&midi_event,
				    cmd.data.parameter.channel - 1,
				    cmd.data.parameter.parameter,
				    cmd.data.parameter.value);
	  snd_seq_ev_set_subs(&midi_event);
	  snd_seq_ev_set_direct(&midi_event);
	  // ... and fire it off
	  snd_seq_event_output_direct(mainport,
				      &midi_event);

	} // end if preset

	debug_more("debug: done processing command...");

      } // end while complete commands are available to process

      // store any remaining partial command at the beginning of the buffer
      if (cmd_data_size) {
	memcpy(cmd_read_buf,
	       next_cmd, 
	       cmd_data_size);
      }

    } // end while client is not done (reading from network for new commands)

  } // end child of the fork

} // end connection_handler()

	  

void get_and_handle_a_connection(void) {

  // file descriptor for the client connection socket
  int client_socket_fd = -1;

  // forking: pid of the client child process
  pid_t child_pid = -1;

  // for return value of the final wait(ing for the child process to exit)
  pid_t some_child_pid = -1;


  // get a connection
  client_socket_fd = wait_for_connection();
  
  // handle it by spawing a server process with fork
  if (client_socket_fd != -1) {
    
    //
    // fork
    //
    
    child_pid = fork();
    
    if (child_pid < 0) {
      
      perror("ERROR: forking error");
      die("forking error");
      
    } else if (child_pid != 0) {
      
      //
      // parent
      //
      
      // close child socket file descriptor
      close(client_socket_fd);
      
      // make sure the child process exits
      some_child_pid = wait(NULL);
      
      if (child_pid != some_child_pid) {
	
	die("problem waiting for client child process to terminate\ndebug: child_pid was %d, some_child_pid was %d\n",
	    child_pid, 
	    some_child_pid);
	
      } else {
	
	exit(0);
	
      } 
      
    } else {
      
      //
      // child
      //
      
      // close parent server socket file descriptor
      // XXX: nowork?
      //	close(cmd_socket_fd);
      
      connection_handler(client_socket_fd);
      
    } // end fork handling
    
  } // end if valid client_socket_fd
  
}


void configure_reaper(void) {

  // signal handler action type
  struct sigaction sig_action;   
                                                   

  // set signal handler
  sig_action.sa_handler = (__sighandler_t)offspring_reaper;

  // mask all signals
  if (sigfillset(&sig_action.sa_mask) < 0) {
    perror("sigfillset()");
    die("sigfillset failed");
  }

  // this means that interrupted system calls will be restarted
  sig_action.sa_flags = SA_RESTART;

  // set signal disposition for child-termination signals 
  if (sigaction(SIGCHLD, &sig_action, 0) < 0) {
    perror("sigaction()");
    die("sigaction failed");
  }

}



void offspring_reaper(void) {

  pid_t pid;

  // wait on the next child process to exit
  while ( ( pid = waitpid((pid_t)-1, NULL, WNOHANG) ) > 0 ) {

    // decrement the global user_count
    user_count--;

    status("childreaper: user count is now %d\n", 
	   user_count);
  }

}
