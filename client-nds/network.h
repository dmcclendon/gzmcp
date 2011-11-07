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
# gzmcpc::network: network functions
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_NETWORK_H
#define _NDS_CLIENT_NETWORK_H





#define WSCANNING_TIMEOUT_S 7

#define WASSOCIATING_TIMEOUT_S 42

#define BCAST_LISTEN_TIMEOUT_S 3600


#define ZYX_AP_NAME "unset"


#define GZMPCD_CMD_PORTNUM 24642

#define GZMPCD_BCAST_PORTNUM 24642
#define DEFAULT_NETWORK_EVENT_RATE 60


#define SSID_MAXLEN 63




#include "gzmcp_protocol.h"

#include <dswifi9.h>

#include "time.h"




typedef enum {
  NSM_START,
  NSM_OFFLINE,
  NSM_WSCANNING,
  NSM_WASSOCIATING,
  NSM_BCAST_LISTEN,
  NSM_UNCONNECTED,
  NSM_CONNECTED,
  NSM_HANDSHAKE_SENT,
  NSM_ONLINE,
} network_state_mode;




void gzmcpc_init_wifi(void);

void gzmcpc_init_net(void);

void socket_set_blocking(int socket_fd);
void socket_set_nonblocking(int socket_fd);

void debug_dump_cmd(gzmcp_cmd cmd);

int xmit_cmd(int socket_fd, gzmcp_cmd cmd);

void sync_with_server(void);

void netmode_go_offline(void);


void netmode_offline_init(void);
network_state_mode netmode_offline_handle(void);

void netmode_wscanning_init(void);
network_state_mode netmode_wscanning_handle(void);

void netmode_wassociating_init(void);
network_state_mode netmode_wassociating_handle(void);

void netmode_bcast_listen_init(void);
network_state_mode netmode_bcast_listen_handle(void);

void netmode_unconnected_init(void);
network_state_mode netmode_unconnecected_handle(void);

void netmode_connected_init(void);
network_state_mode netmode_connected_handle(void);

void netmode_handshake_sent_init(void);
network_state_mode netmode_handshake_sent_handle(void);

void netmode_online_init(void);
network_state_mode netmode_online_handle(void);

network_state_mode gzmcpc_net_next_state(network_state_mode current);





extern network_state_mode network_state;
extern network_state_mode last_network_state;
extern int nsm_init;

extern u8 network_event_rate; 

extern int socket_shutdown_state;

extern int zyx_ap_id;

extern Wifi_AccessPoint zyx_ap;

extern unsigned char wepkeys[4][32];

extern int wepkeyid;

extern int wepmode;

extern int dhcp;

extern struct in_addr gzmcpc_ipaddr;
extern int gzmcpc_ipaddr_last_byte;
extern struct in_addr gzmcpc_subnet_mask;
extern struct in_addr gzmcpc_gateway;
extern struct in_addr gzmcpc_dns1;
extern struct in_addr gzmcpc_dns2;

extern char ap_ssid[SSID_MAXLEN + 1];

extern time_val next_sync_with_server;

extern int cstate_preset;
extern char cstate_preset_name[64];
extern int cstate_wpx;
extern int cstate_wpy;

extern int server_socket_fd;

#endif // _NDS_CLIENT_NETWORK_H
