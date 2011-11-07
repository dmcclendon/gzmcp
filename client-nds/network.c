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
## gzmcpc::network: network functions
##
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/




#include <nds.h>

#include <dswifi9.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include <unistd.h>

#include <fcntl.h>

#include <stdio.h>

#include <stdarg.h>

#include <string.h>

#include <errno.h>



#include "mode__tpw__jam.h"

#include "debug.h"

#include "graphics.h"

#include "network.h"

#include "sound.h"


#include "sounds.h"

#include "sounds_bin.h"




network_state_mode network_state = NSM_START;
network_state_mode last_network_state = NSM_START;
int nsm_init = 0;

u8 network_event_rate = DEFAULT_NETWORK_EVENT_RATE; 

int zyx_ap_id = -1;

Wifi_AccessPoint zyx_ap;

short cmd_port_number = GZMPCD_CMD_PORTNUM;
short bcast_port_number = GZMPCD_BCAST_PORTNUM;

unsigned char wepkeys[4][32];

int wepkeyid;

int wepmode;

int dhcp;

struct in_addr gzmcpc_ipaddr = {0,};
int gzmcpc_ipaddr_last_byte = {0,};
struct in_addr gzmcpc_subnet_mask = {0,};
struct in_addr gzmcpc_gateway = {0,};
struct in_addr gzmcpc_dns1 = {0,};
struct in_addr gzmcpc_dns2 = {0,};

char ap_ssid[SSID_MAXLEN + 1];

time_val next_sync_with_server = {0, 0};


int cstate_preset = CM_PRESET_X_NUM;
int cstate_wpx = 63;
int cstate_wpy = 63;


int server_socket_fd = 0;




struct sockaddr_in bind_address;

struct sockaddr_in bcaster_address;

int bcast_socket_fd = 0;
long bcast_socket_fd_opts = 0;

int network_vanished = 0;

gzmcp_cmd newcmd;

long magic_cookie;

gzmcp_cmd handshake_cmd;

time_val net_nap = {0, 0};

time_val nsm_timeout = {0, 0};


int sstate_preset = -1;
int sstate_wpx = -1;
int sstate_wpy = -1;






void gzmcpc_init_net(void) {
  
  // gratuitous (test) debug logging
  debug_log("in gzmcpc_init_net...\n");

  // initialize system wireless networking capabilities
  gzmcpc_init_wifi();

  // initialize server_socket_fd
  server_socket_fd = 0;
  // initialize bcast_socket_fd
  bcast_socket_fd = 0;
  
  // initialize network status
  textout(TLT_NETWORK_STATUS, "offline");

  //
  // initialize handshake
  //

  // put the handshake magic cookie where it can be sizeof()d
  magic_cookie = (long)GZMCP_DEF_MAGIC_COOKIE;

  // create the handshake_cmd packet (aka the handshake)
  handshake_cmd.type = GZMCP_CMD_HANDSHAKE;
  memcpy((void *)handshake_cmd.data.handshake.string, 
	 (void *)&magic_cookie,
	 sizeof(magic_cookie));

  // XXX: stackcorruption?
  // this ugliness stays until I find out how the .type field
  // is getting corrupted.
  debug_log_more("handshake_cmd.type is %d\n",
	    handshake_cmd.type);
  debug_log_more("GZMCP_CMD_HANDSHAKE (gch) is %d\n",
	    GZMCP_CMD_HANDSHAKE);
  debug_log_more("sizeof magic_cookie is %d\n",
	    sizeof(magic_cookie));
  debug_log_more("handshake created...\n");

}



void gzmcpc_init_wifi(void) {
  
  // gratuitous (test) debug logging
  debug_log("in gzmcpc_init_wifi...\n");

  // XXX: document the parameter (replace this comment)
  Wifi_InitDefault(false);
  
  // init/cleanup (?unknown if needed, vaguely recall recommended)
  Wifi_DisconnectAP();

  // init/cleanup (?unknown if needed, vaguely recall recommended)
  Wifi_DisableWifi();

}


void socket_set_blocking(int socket_fd) {

  int val_arg = 0;

  retval = ioctl(socket_fd, FIONBIO,  &val_arg);

  // effective assertion
  if (retval < 0) {
    textout(TLT_ERROR, "socket ioctl failed");
    die();
  }

}


void socket_set_nonblocking(int socket_fd) {

  int val_arg = 1;

  retval = ioctl(socket_fd, FIONBIO,  &val_arg);

  // effective assertion
  if (retval < 0) {
    textout(TLT_ERROR, "socket ioctl failed");
    die();
  }

}



void debug_dump_cmd(gzmcp_cmd cmd) {
  debug_log_more("debug_dump_command: cmd.type: %08x\n",
	    cmd.type);
  debug_log_more("debug_dump_command: cmd[1-4]: %08x\n",
	    ((long *)&cmd)[0]);
  debug_log_more("debug_dump_command: cmd[5-8]: %08x\n",
	    ((long *)&cmd)[1]);
  debug_log_more("debug_dump_command: cmd[9-12]: %08x\n",
	    ((long *)&cmd)[2]);
  debug_log_more("debug_dump_command: cmd[13-16]: %08x\n",
	    ((long *)&cmd)[3]);
}



int xmit_cmd(int socket_fd, gzmcp_cmd cmd) {

  // for read/write return values
  int numbytes;

  // only do work if needed
  if ((network_state != NSM_ONLINE) &&
      (cmd.type != GZMCP_CMD_HANDSHAKE)) {
    return 0;
  }

  debug_dump_cmd(cmd);

  // write the test message to the socket
  numbytes = send(socket_fd,
		  (void *)&cmd,
		  sizeof(gzmcp_cmd),
		  0);

  // check for errors
  //  if (numbytes < 0) textout(TLT_ERROR, "xmit_cmd:send");
  // or not since non-blocking (but still should check for other errors)

  return numbytes;

}



void sync_with_server(void) {

  if (cstate_preset != sstate_preset) {

    //
    // create a command packet and transmit to server
    //
    newcmd.type = GZMCP_CMD_PRESET;
    newcmd.data.preset.channel = 1;
    newcmd.data.preset.preset = cstate_preset;
    xmit_cmd(server_socket_fd, newcmd);

    // update state
    sstate_preset = cstate_preset;
  }

  if (touch_whammy_enabled) {

    // XXX add optional decaying logic, i.e. only half the difference each time
    
    if (cstate_wpx != sstate_wpx) {
      //
      // create a command packet and transmit to server
      //
      newcmd.type = GZMCP_CMD_PARAMETER;
      newcmd.data.parameter.channel = 1;
      newcmd.data.parameter.parameter = touch_whammy_x_midi_parm;
      newcmd.data.parameter.value = cstate_wpx;
      xmit_cmd(server_socket_fd, 
	       newcmd);
      
      // update state
      sstate_wpx = cstate_wpx;
    }

    if (cstate_wpy != sstate_wpy) {
      //
      // create a command packet and transmit to server
      //
      newcmd.type = GZMCP_CMD_PARAMETER;
      newcmd.data.parameter.channel = 1;
      newcmd.data.parameter.parameter = touch_whammy_y_midi_parm;
      newcmd.data.parameter.value = cstate_wpy;
      xmit_cmd(server_socket_fd, 
	       newcmd);
      
      // update state
      sstate_wpy = cstate_wpy;
    }

  }

}



void netmode_go_offline(void) {

  // may be nothing to shutdown, so ignore return value
  shutdown(server_socket_fd, 0);
  closesocket(server_socket_fd);
  shutdown(bcast_socket_fd, 0);
  closesocket(bcast_socket_fd);

  // disconnect from network/AP
  Wifi_DisconnectAP();
  Wifi_DisableWifi();

  // cursor rotating is online indicator
  cursor_rotating = 0;
  
  // print meaningful data to top lcd
  textout(TLT_MODE, "L+start to go online");
  // leave STATUS alone, as it may have an error message
  //  textout(TLT_STATUS, "XXX");
  textout(TLT_NETWORK_STATUS, "offline");
  textout(TLT_FX_MODE, "N/A");
  textout(TLT_TOUCH_VAL, "X=NA /\\/ Y=NA");
  
  if (!user_vstrobe_enabled) {
    vstrobe_enabled = 0;
  }

  // ooh, ahh, earcandy
  mmEffectEx(&sounds[SFX_OFFLINE]);

}



void netmode_offline_init(void) {

  netmode_go_offline();

}



network_state_mode netmode_offline_handle(void) {
  // nothing to do while OFFLINE
  return NSM_OFFLINE;
}



void netmode_wscanning_init(void) {

  // extra cleanslate attempt (is start button bypassing go_offline somehow?)
  Wifi_DisconnectAP();
  Wifi_DisableWifi();

  Wifi_EnableWifi();

  vstrobe_enabled = 1;
  
  // initialize text display 
  // TODO generate progrometer from start time
  textout(TLT_MODE, "net::going online");
  textout(TLT_STATUS, "net init...");
  textout(TLT_NETWORK_STATUS, "scanning nets...");

  Wifi_ScanMode();

  // set the network mode timeout relative to now
  nsm_timeout = time_val_add_ms(num_ticks, 
				1000 * WSCANNING_TIMEOUT_S);
    
}



network_state_mode netmode_wscanning_handle(void) {

  // for storing data of ap's from scan 
  Wifi_AccessPoint ap;
  // and the ssid thereof
  char ssidbuf[SSID_MAXLEN + 1];

  int i;
  int num_networks;


  // check for timeout condition
  if (time_val_compare(num_ticks, nsm_timeout) < 0) {
    // timeout expired
    textout(TLT_STATUS, "wifi ap search timeout");
    return NSM_OFFLINE;
  } // end if timeout expired

  // note: could add logic here to make sure the search code
  //       is only executed 10 times a second.  Curious to
  //       see if it effects the UI.

  num_networks = Wifi_GetNumAP();
      
  textout(TLT_STATUS, "%d networks found", num_networks);

  zyx_ap_id = -1;
  
  for (i = 0 ; i < num_networks ; i++) {
    
    if (WIFI_RETURN_OK == Wifi_GetAPData(i,&ap)) {
      
      strncpy(ssidbuf, ap.ssid, ap.ssid_len);
      
      // assumption: Wifi_GetAPData's returned ap. will never
      //             have ap.ssid_len (and .ssid) of >32, as
      //             per wifi spec.  ssidbuf is 64.
      // null terminate
      ssidbuf[(int)ap.ssid_len] = 0;
      
      if (! strncmp(ap_ssid, ssidbuf, strlen(ap_ssid))) {
	zyx_ap_id = i;
      }
      
    } // end got data for ap
    
  } // end iteration over networks
  
  if (zyx_ap_id != -1) {
    
    return NSM_WASSOCIATING;

  }

  // havent found a good wifi ap yet, keep trying
  return NSM_WSCANNING;
}



void netmode_wassociating_init(void) {

  unsigned long dns1, dns2;

  // found a Guitar-ZyX Access Point
  textout(TLT_MODE, "net::going online");
  textout(TLT_STATUS, "found mcpnet...");
  textout(TLT_NETWORK_STATUS, "connecting to net...");

  // in this case, flag _handle to go offline with appropriate message
  if (Wifi_GetAPData(zyx_ap_id, &zyx_ap) != WIFI_RETURN_OK) {
    network_vanished = 1;
    // don't try to do rest of init
    return;
  } else {
    network_vanished = 0;
  }

  //
  // initialize wep config
  //
  
  wepkeyid = 0;
  
  // initialize wepkeys to null terminated strings
  wepkeys[0][0] = 0;
  wepkeys[1][0] = 0;
  wepkeys[2][0] = 0;
  wepkeys[3][0] = 0;
  
  wepmode = 0;
  
  // dns1 = 192.168.1.1
  // dns1 = 0x0101A8C0;
  // dns1 = 0xC0A80101;
  dns1 = 0;
  
  // dns2 = 0.0.0.0
  dns2 = 0;
  
  // dhcp IP/not dns, because gzmcp does not use dns
  Wifi_SetIP(0, 0, 0, dns1, dns2);
  
  if (strncmp(ap_ssid, "wfc", strlen(ap_ssid)) == 0) {
    // use user's firmware WFC connection info 
    // (as configured by another wifi enabled game/app)
    Wifi_AutoConnect();
  } else {
    // use the user supplied ap_ssid
    Wifi_ConnectAP(&zyx_ap, wepmode, wepkeyid, wepkeys[0]);
  }

  textout(TLT_STATUS, "connecting to ap...");
  
  // set the scan timeout relative to now
  nsm_timeout = time_val_add_ms(num_ticks, 
				1000 * WASSOCIATING_TIMEOUT_S);

}



network_state_mode netmode_wassociating_handle(void) {

  int associated;

  // check for timeout condition
  if (time_val_compare(num_ticks, nsm_timeout) < 0) {
    // timeout expired
    textout(TLT_STATUS, "net assoc timeout");
    return NSM_OFFLINE;
  } // end if timeout expired

  if (network_vanished) {
    textout(TLT_STATUS, "network vanished");
    return NSM_OFFLINE;
  }

  //
  // check associating status
  //
  associated = Wifi_AssocStatus();

  if (associated == ASSOCSTATUS_ASSOCIATED) {

    // clear mode&status now that connection is done
    textout(TLT_MODE, "");
    textout(TLT_STATUS, "");

    // store gzmcpc's ip info
    gzmcpc_ipaddr = Wifi_GetIPInfo(&gzmcpc_gateway,
				   &gzmcpc_subnet_mask,
				   &gzmcpc_dns1,
				   &gzmcpc_dns2);

    // transmogrify network byte order into host byte order
    gzmcpc_ipaddr.s_addr = ntohl(gzmcpc_ipaddr.s_addr);
    gzmcpc_gateway.s_addr = ntohl(gzmcpc_gateway.s_addr);
    gzmcpc_subnet_mask.s_addr = ntohl(gzmcpc_subnet_mask.s_addr);
    gzmcpc_dns1.s_addr = ntohl(gzmcpc_dns1.s_addr);
    gzmcpc_dns2.s_addr = ntohl(gzmcpc_dns2.s_addr);
    
    // get the last byte of our ipaddr, for possible debug output
    //    gzmcpc_ipaddr_last_byte = 
    //      (int)(gzmcpc_ipaddr.s_addr - 
    //	    (gzmcpc_ipaddr.s_addr & gzmcpc_subnet_mask.s_addr));

    return NSM_BCAST_LISTEN;

  } // end case of assoc status == associated

  if (associated == ASSOCSTATUS_CANNOTCONNECT) {

    textout(TLT_STATUS, "net assoc failed");

    return NSM_OFFLINE;

  }

  // not yet associated, keep on trying...
  return NSM_WASSOCIATING;

}



void netmode_bcast_listen_init(void) {

  int optval;


  textout(TLT_MODE, "net::going online");
  textout(TLT_STATUS, "joined network");
  textout(TLT_NETWORK_STATUS, "server discovery...");

  debug_log("listening for server broadcasts on udp port %d\n",
	    bcast_port_number);
  
  debug_log("listening for magic cookie - 0x%08x\n",
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
    textout(TLT_ERROR, "couldn't create broadcast socket");
    die();
  }
  
  // enable broadcast
  // note: perhaps this is unnecessary, since we only receive
  optval = 1;
  if (setsockopt(bcast_socket_fd, 
		 SOL_SOCKET, 
		 SO_BROADCAST, 
		 &optval, sizeof(optval)) < 0) {
    perror("setsockopt()");
    debug_log("failed to set broadcast socket options\n");
    textout(TLT_ERROR, "failed to set broadcast socket options");
    die();
  }
      
  // create a destination address structure, initialize...
  memset((void *)&bind_address, 0, sizeof(bind_address));
  
  // configure the address structure for our purposes
  // INET, vs e.g. UNIX for local non network sockets
  bind_address.sin_family = AF_INET;
  bind_address.sin_port = htons(bcast_port_number);
  // we're going to bind to all local interfaces
  bind_address.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (bind(bcast_socket_fd, 
	   (struct sockaddr *)&bind_address,
	   sizeof(bind_address)) < 0) {
    perror("bind()");
    textout(TLT_ERROR, "failed to bind to broadcast socket");
    die();
  }
  
  debug_log("now setting bcast socket to nonblocking...\n");

  // set socket to non-blocking mode
  socket_set_nonblocking(bcast_socket_fd);

  // set the scan timeout relative to now
  nsm_timeout = time_val_add_ms(num_ticks, 
				1000 * BCAST_LISTEN_TIMEOUT_S);
  
}



network_state_mode netmode_bcast_listen_handle(void) {

  // buffer to receive broadcast messages
  char beacon_msg[GZMCP_BEACON_MSG_MAXLEN + 1];

  // size of returned address structure
  unsigned int from_length;

  // size of broadcast message data received
  int bytes_received;


  // check for timeout condition
  if (time_val_compare(num_ticks, nsm_timeout) < 0) {
    // timeout expired
    textout(TLT_STATUS, "server discovery timeout");
    return NSM_OFFLINE;
  } // end if timeout expired
  
  
  // initialize the length of the address result
  from_length = sizeof(bcaster_address);
  
  // note: cast (int *) is due to different nds devsuite impl??
  bytes_received = recvfrom(bcast_socket_fd,
			    beacon_msg,
			    GZMCP_BEACON_MSG_MAXLEN,
			    0,
			    (struct sockaddr*)&bcaster_address,
			    (int *)&from_length);

  debug_log("debug: bytes_received for beacon was %d\n", 
  	   bytes_received);

  // check for locally failed recv
  if (bytes_received < 0) {
    // udp makes no guarantees
    //      perror("sendto()");
    debug_log_more("could not receive message\n");
    return NSM_BCAST_LISTEN;
  }

  // check if message is a gzmcp server broadcast message
  if (bcmp((void *)beacon_msg, 
	   (void *)&magic_cookie, 
	   4) == 0) {
    // a correct magic cookie
    debug_log("server at %s had the right magic cookie!\n", 
	      inet_ntoa(bcaster_address.sin_addr));
    
    // throttle a bit?
    net_nap = time_val_add_ms(num_ticks, 420);

    // success, target server is in bcaster_address
    return NSM_UNCONNECTED;
    
  } else {
    // not a correct magic cookie
    debug_log("got an incorrect magic cookie! ignoring...\n");
  }

  // keep listening...
  return NSM_BCAST_LISTEN;
}



void netmode_unconnected_init(void) {

  debug_log("server found at ip address -- %s\n",
	    inet_ntoa(bcaster_address.sin_addr));

  textout(TLT_MODE, "net::going online");
  textout(TLT_STATUS, 
	  "server found"); 
  //  textout(TLT_STATUS, 
  //	  "srv::%s",
  //	  inet_ntoa(bcaster_address.sin_addr));
  textout(TLT_NETWORK_STATUS, "connecting...");

  //
  // open an internet stream socket for the client
  //
  
  // pointless init
  server_socket_fd = 0;
  
  // create socket
  server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  // error check socket()
  if (server_socket_fd < 0) {
    
    // TODO: make something like this the assertion function
    textout(TLT_ERROR, "socket open failed");
    
    // assertion: should not fail
    die();
    
  }
      
  // connect to cmd_port_number on the found server
  bcaster_address.sin_port = htons(cmd_port_number);
      
  // set socket to non-blocking mode
  socket_set_nonblocking(server_socket_fd);
  
  // debug throttling
  net_nap = time_val_add_ms(num_ticks, 420);
  
}



network_state_mode netmode_unconnected_handle(void) {

  // NOTE: I was confused a bit, and had const as part of the cast of
  //       arg 2, but then realized the confusion was because const
  //       wasn't a necessary (or sensical) part of the cast.  Actually
  //       I'm still a bit confused.  The const in the man 2 connect
  //       prototype confuses me...  I.e. the below doesn't generate
  //       warnings, but theoretically the data in bcaster_address is
  //       not const.  (could be modified by another thread?).
  
  retval = connect(server_socket_fd,
		   (struct sockaddr *)&bcaster_address,
		   sizeof(bcaster_address));

  debug_log_more("connect retval for server connect was - %d ...\n", 
	    retval);

  // historic(?) note: bad things happened with a textout+delay of errno here
  // that was a delay with the archaic wait_a_bit function which spun around
  // vblank intr
  
  // XXX catch and check errno appropriately 

  // network throttling (in other words, probably a network race condition)
  // (or maybe closesocket fixed that)
  net_nap = time_val_add_ms(num_ticks, 420);
  
  // nonblocking, nonfatal
  if (retval < 0) {
    // keep trying...
    return NSM_UNCONNECTED;
  }
  
  // throttle a bit?
  net_nap = time_val_add_ms(num_ticks, 420);

  debug_log("server socket now connected...\n");

  return NSM_CONNECTED;

}



void netmode_connected_init(void) {

}



network_state_mode netmode_connected_handle(void) {

  //
  // connected, time to send handshake
  //
  // MAYBETODO: add current datestamp as part of handshake (maybe hashed)
  //            - or app version hash, password from flash, ...

  
  // XXX: this is a workaround to ?stackcorruption?  I.e. should only
  //      need to set this once, but somehow the .type field is getting
  //      reset to 0???
  // create the handshake_cmd packet (aka the handshake)
  handshake_cmd.type = GZMCP_CMD_HANDSHAKE;
  memcpy((void *)handshake_cmd.data.handshake.string, 
	 (void *)&magic_cookie,
	 sizeof(magic_cookie));

  // XXX: stackcorruption? (see other XXX comments)
  debug_log_more("dumping handshake_cmd before sending it\n");
  debug_dump_cmd(handshake_cmd);

  //
  // transmit handshake to server
  //
  retval = xmit_cmd(server_socket_fd, 
		    handshake_cmd);

  debug_log_more("trying to send handshake, xmit_cmd retval is %d\n",
	    retval);

  // XXX: stackcorruption? (see other XXX comments)
  debug_log_more("dumping handshake_cmd after sending it\n");
  debug_dump_cmd(handshake_cmd);

  if (retval == sizeof(gzmcp_cmd)) {
    
    textout(TLT_STATUS, "sent handshake"); 
    
    //net_nap = time_val_add_ms(num_ticks, 42);

    return NSM_HANDSHAKE_SENT;
    
  } else {
    
    // this is acceptable nonblocking behavior
    
  } // end if/else handshake xmit_cmd-ed correctly
  

  // keep trying...
  return NSM_CONNECTED;

}



void netmode_handshake_sent_init(void) {

  // TODO: either init textouts here, or collapse this mode into unconnected

}



network_state_mode netmode_handshake_sent_handle(void) {

  int numbytes;
  gzmcp_cmd reflected_handshake_cmd;

  //
  // handshake sent, listen for reciprocation
  //

  // XXX: stackcorruption? (see other XXX comments)
  handshake_cmd.type = GZMCP_CMD_HANDSHAKE;
  
  // XXX: stackcorruption? (see other XXX comments)
  debug_log_more("dumping handshake_cmd before receiving reflected\n");
  debug_dump_cmd(handshake_cmd);

  ////
  //// receive the reflected handshake
  ////
  numbytes = recv(server_socket_fd,
		  (void *)&reflected_handshake_cmd,
		  sizeof(gzmcp_cmd), 
		  0);
  // nonblocking, nonfatal
  // XXX catch errno and handle server disconnecting, etc... (prob elsewhere too)
  if (numbytes < 0) {
    // note, this (strerror) workes, but I'm getting "no more processes here"??
    debug_log_more("perror...%s\n",
		   strerror(errno));
    debug_log_more("failed to receive handshake -nb-%d\n",
	      numbytes);
    // keep trying...
    return NSM_HANDSHAKE_SENT;
  }

  if (numbytes != sizeof(gzmcp_cmd)) {
    debug_log_more("failed to receive handshake -nb-%d\n",
		   numbytes);
    //    textout(TLT_DEBUG,
    //	    "hndshk_recv_sz:%d",
    //	    numbytes);

    // keep trying...
    return NSM_HANDSHAKE_SENT;
  }

  // XXX: stackcorruption? (see other XXX comments)
  debug_log_more("dumping handshake_cmd before memcmp\n");
  debug_dump_cmd(handshake_cmd);

  // check reflected handshake against the original
  if (memcmp((void*)&reflected_handshake_cmd,
	     (void*)&handshake_cmd,
	     sizeof(gzmcp_cmd)) != 0 ) {

    // XXX: stackcorruption? (see other XXX comments)
    debug_log_more("dumping handshake_cmd after memcmp\n");
    debug_dump_cmd(handshake_cmd);
    debug_log_more("dumping reflected_handshake_cmd after memcmp\n");
    debug_dump_cmd(reflected_handshake_cmd);
    
    
    //
    // incorrect handshake
    //
    textout(TLT_STATUS, 
	    "bad handshake reply");
    textout(TLT_DEBUG, 
	    "bhsr:%s:", 
	    reflected_handshake_cmd.data.handshake.string);
    
    debug_log("bad handshake received :: %s ::\n", 
	      reflected_handshake_cmd.data.handshake.string);
	
    // keep trying as this is not a fatal error...
    return NSM_HANDSHAKE_SENT;
	
  } else {
    //
    // correct handshake
    //
    textout(TLT_STATUS, "handshake received");

    return NSM_ONLINE;
	
  } // end if correct handshake

}



void netmode_online_init(void) {

  // clear mode now that we are connected
  textout(TLT_MODE, "online! :)");
  
  // reset socket to blocking mode
  // TODO: investigate using nonblocking mode for user data
  // TODO: investigate using udp for user data
  socket_set_blocking(server_socket_fd);

  // send initial command
  // TODO this should be a function (duplication with other code, abstract button...)

  //
  // create and send the new command/packet
  //
  newcmd.type = GZMCP_CMD_PRESET;
  newcmd.data.preset.channel = 1;
  newcmd.data.preset.preset = button_b_preset_num;
  xmit_cmd(server_socket_fd, newcmd);

  current_preset = button_b_preset_num;
  
  // ooh, ahh, earcandy
  mmEffectEx(&sounds[SFX_ONLINE]);

  // initialize text display 
  textout(TLT_MODE, "cloudsession live!");
  textout(TLT_STATUS, "jam on!!!!!!!");
  textout(TLT_NETWORK_STATUS, "online");
  
  if (!user_vstrobe_enabled) {
    vstrobe_enabled = 0;
  }

}



network_state_mode netmode_online_handle(void) {
  // nothing to do while ONLINE
  return NSM_ONLINE;
}


/* TEMPLATE NETWORK STATE MODE FUNCTION PAIR

void netmode_xxx_init(void) {


}



network_state_mode netmode_xxx_handle(void) {
  // nothing to do while XXX
  return NSM_XXX;
}


END TEMPLATE NETWORK STATE MODE FUNCTION PAIR */



network_state_mode gzmcpc_net_next_state(network_state_mode current) {

  network_state_mode next_state;


  debug_log_more("net_next_state: %d\n", current);

  // do nothing interesting by default
  next_state = current;

  // check for net_nap throttling
  // i.e. "if the current time is smaller than the nap marker, do nothing"
  if (time_val_compare(net_nap, num_ticks) < 0) {
    debug_log_more("net napping %d %d...\n",
		   num_ticks.s, num_ticks.ms);
    return current;
  }

  debug_log_more("not net napping...\n");

  //
  // handle all possible present states
  //
  // TODO: could convert this to an array of funcpointers like I do for main mode
  //       (i.e. no code redundancy)
  switch (current) {
    
  case NSM_START:
    // START is somewhat bogus

    // jump strait to WSCANNING
    return NSM_WSCANNING;
    
    break;

  case NSM_OFFLINE:

    if (!nsm_init) {
      netmode_offline_init();
      nsm_init = 1;
    }

    return netmode_offline_handle();

    break;

  case NSM_WSCANNING:

    if (!nsm_init) {
      netmode_wscanning_init();
      nsm_init = 1;
    }

    return netmode_wscanning_handle();

    break;

  case NSM_WASSOCIATING:

    if (!nsm_init) {
      netmode_wassociating_init();
      nsm_init = 1;
    }

    return netmode_wassociating_handle();

    break;

  case NSM_BCAST_LISTEN:

    if (!nsm_init) {
      netmode_bcast_listen_init();
      nsm_init = 1;
    }

    return netmode_bcast_listen_handle();

    break;

  case NSM_UNCONNECTED:

    if (!nsm_init) {
      netmode_unconnected_init();
      nsm_init = 1;
    }

    return netmode_unconnected_handle();

    break;

  case NSM_CONNECTED:

    if (!nsm_init) {
      netmode_connected_init();
      nsm_init = 1;
    }

    return netmode_connected_handle();

    break;

  case NSM_HANDSHAKE_SENT:

    if (!nsm_init) {
      netmode_handshake_sent_init();
      nsm_init = 1;
    }

    return netmode_handshake_sent_handle();

    break;

  case NSM_ONLINE:

    if (!nsm_init) {
      netmode_online_init();
      nsm_init = 1;
    }

    return netmode_online_handle();

    break;

  } // end switch (current) // current state

  // default to same state, though code should never reach here
  return next_state;

} // end gzmcpc_net_next_state()

