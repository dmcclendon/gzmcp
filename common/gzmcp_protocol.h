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
## gzmcp: protocol definition
##
#############################################################################
##
## Copyright 2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/



#define GZMCP_DEF_MAGIC_COOKIE 0x42244224

#define GZMCP_DEF_TCP_PORT 24642
#define GZMCP_DEF_UDP_PORT 24642

#define GZMCP_BEACON_MSG_MAXLEN 1023

#define GZMCP_HANDSHAKE_MAXLEN 11





typedef enum {
  GZMCP_CMD_UNDEFINED,
  GZMCP_CMD_HANDSHAKE,
  GZMCP_CMD_PRESET,
  GZMCP_CMD_PARAMETER,
  GZMCP_CMD_GETUPDATE,
  GZMCP_CMD_SHUTDOWN,
  GZMCP_CMD_MAX = 0xFFFFFFFA,
} gzmcp_cmd_type;

typedef struct gzmcp_cmd_handshake_data_tag {
  char string[GZMCP_HANDSHAKE_MAXLEN + 1];
} gzmcp_cmd_handshake_data;


typedef struct gzmcp_cmd_preset_data_tag {
  int32_t channel;
  int32_t preset;
} gzmcp_cmd_preset_data;

typedef struct gzmcp_cmd_parameter_data_tag {
  int32_t channel;
  int32_t parameter;
  int32_t value;
} gzmcp_cmd_parameter_data;

typedef struct gzmcp_cmd_getupdate_data_tag {
  // size of last cached copy, -1 if not available
  uint32_t size;
  // xor of all ints in the file
  uint32_t hash;
  // 4 sample bytes of last cached copy,
  // first and last byte of first and second half of file
  uint32_t sample_data;
} gzmcp_cmd_getupdate_data;

typedef union gzmcp_cmd_data_tag {
  gzmcp_cmd_handshake_data handshake;
  gzmcp_cmd_preset_data preset;
  gzmcp_cmd_parameter_data parameter;
  gzmcp_cmd_getupdate_data getupdate;
} gzmcp_cmd_data;


typedef struct gzmcp_cmd_tag {
  gzmcp_cmd_type type;
  gzmcp_cmd_data data;
} gzmcp_cmd;
