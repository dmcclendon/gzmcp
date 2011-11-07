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
## gzmcpc::mode__get_update: some source file
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

#include <fat.h>


#include <stdio.h>

#include <string.h>

#include <sys/socket.h>

#include <unistd.h>




#include "debug.h"

#include "dmc.h"

#include "graphics.h"

#include "mcp.h"

#include "modes.h"

#include "mode__intro__main.h"

#include "mode__get_update.h"

#include "network.h"

#include "cloader/arm9_loader.h"


#include "resources/bitmaps/guitar-zyx.splash.main.h"

#include "resources/bitmaps/dlava.h"





int overwrite_self_with_update;


int update_file_size = 42;
int download_bytes = 0;





void mode__get_update___init(void) {

  // to store the retrieved update
  FILE *update_file;

  // packet structure for sending commands to the server
  gzmcp_cmd newcmd;

  // for send() return values
  int numbytes;
  int rv, i;

  // for reading the update file
  int bytes_to_read;
  //  unsigned char read_buf[UPD_RD_BUF_SZ];
  unsigned char *read_buf;

  // for magic return value checking
  unsigned char magic_found;
  unsigned char last_byte;
  unsigned char second_to_last_byte;

  // for returned signature
  uint32_t update_signature[3];


  // initialize main screen
  videoSetMode(MODE_5_2D);

  // map main screen background fourth (128k) region to vram bank A
  vramSetBankA(VRAM_A_MAIN_BG_0x06060000);
  
  // set the secondary/sub screen for text and a background
  videoSetModeSub(MODE_5_2D);

  // map sub screen background (only? 1/4?) to vram bank C
  vramSetBankC(VRAM_C_SUB_BG);

  mcp_console_init(&bottom_screen,
		   MCP_SUB_SCREEN,
		   0,
		   1,
		   1,
		   BgType_Text4bpp, 
		   BgSize_T_256x256, 
		   31, 
		   0);

  // set printf sink
  consoleSelect(&bottom_screen);

  // set console background layer to top priority
  bgSetPriority(bottom_screen.bgId, 0);

  // note: this must be done _after_ consoleInit (as that resets it)
  //       and _after_ loading our 8bit indexed bitmap reloads it
  // set to black to allow renderer to really control
  BG_PALETTE_SUB[255] = RGB15(0, 0, 0);

  bgShow(bottom_screen.bgId);

  // default to fully faded to black (31) not the opposite (0)
  //  REG_BLDY = 31;
  // XXX firstpass nofade
  REG_BLDY = 0;
  // but for credits, background starts unfaded
  REG_BLDY_SUB = 0;

  // fade the mainscreen background to/from black, layer 3
  REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG3;
  // fade the lava background to/from black, layer 3
  REG_BLDCNT_SUB = BLEND_FADE_BLACK | BLEND_SRC_BG3;

  // init subscreen layer/background 3 
  // the mapbase offset of 24 here means 24*16k which means utilizing
  // the 4th of the possible main background memory regions that vram
  // bank A can be mapped to.  I.e. above we mapped to the 4th.  Had
  // we mapped to the 1st, we would have used offset 0.  
  // note: vram bank A is 128k, i.e. 8 * 16k.
  // note: *16k is because of bitmap type, else would be *2k
  bg3 = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 24, 0);
  // its initial priority, lowest (to emphasize lack of other enabled layers)
  // priorities 0..3, 0 highest priority
  bgSetPriority(bg3, 3);

  bgHide(bg3);

  // load main splash screen into screen/background memory (bgs3)
  // note: if I wanted to do this quickly/perfectly/doublebufferred somehow
  //       I'm not sure if I'd need to tweak the bgInitSub offset or pointer
  //       here or some such.  As it is, the fade to black ensures no tear
  //       as this new image gets loaded into display memory.
  decompress(guitar_zyx_splash_mainBitmap, 
	     (u16*)bgGetGfxPtr(bg3),  
	     LZ77Vram);
  
  bgShow(bg3);

  // note: using offset=4, because 4 will be 64k offset, where 31 above is 62k
  // (thus above using only 2k? seems plausible with tiles for console text chars
  bgs3 = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 4, 0);
  // its initial priority, lowest (to emphasize lack of other enabled layers)
  // priorities 0..3, 0 highest priority
  bgSetPriority(bgs3, 2);

  bgHide(bgs3);

  // as per libnds doc on dma
  DC_FlushRange(dlavaBitmap, 256*256);
  dmaCopy(dlavaBitmap, bgGetGfxPtr(bgs3), 256*256);
  DC_FlushRange(dlavaPal, 256*2);
  dmaCopy(dlavaPal, BG_PALETTE_SUB, 256*2);
  
  bgShow(bgs3);

  //  BG_PALETTE_SUB[255] = RGB15(0, 0, 0);
  // XXX: firstpass 
  BG_PALETTE_SUB[255] = RGB15(0, 24, 0);

  //
  // above is intro__credits with main splash, below is mode init
  //

  // clear text screen
  printf("\x1b[2J");

  printf("\x1b[11;0H  ============================  ");
  printf("\x1b[13;0H                                ");
  printf("\x1b[13;0H  >> sending update request...  ");
  printf("\x1b[15;0H  ============================  ");
  swiWaitForVBlank();

  // send getupdate command
  newcmd.type = GZMCP_CMD_GETUPDATE;
  // TODO: check for existing cached copy, and set these appropriately
  newcmd.data.getupdate.size = 0;
  newcmd.data.getupdate.hash = 0;
  newcmd.data.getupdate.sample_data = 0;
  rv = xmit_cmd(server_socket_fd, newcmd);

  if (rv != sizeof(gzmcp_cmd)) {
    printf("\x1b[13;0H                            ");
    printf("\x1b[13;0H  >> dbg:rv was %d / %d",
	   rv,
	   sizeof(gzmcp_cmd));
    for (i = 0 ; i < 120 ; i++) {
      swiWaitForVBlank();
    }
  }

  //
  // receive update file from server
  //

  // discard incoming data until magic is found
  // todo: this could be a generic function with magic and socket_fd as arguments
  // XXX: repitition of this status mechanism is temporary, to be replaced with
  //      a user-abortable nonblocking inmplementation of this mode
  printf("\x1b[11;0H  ============================  ");
  printf("\x1b[13;0H                                ");
  printf("\x1b[13;0H  >> waiting for server ...     ");
  printf("\x1b[15;0H  ============================  ");
  swiWaitForVBlank();
  magic_found = 0;
  last_byte = 0;
  second_to_last_byte = 0;
  while (!magic_found) {
    second_to_last_byte = last_byte;
    // recieve a byte
    numbytes = recv(server_socket_fd,
		    (void *)&last_byte,
		    1, 
		    0);

    // check for errors
    if (numbytes < 0) {
      printf("\x1b[13;0H                              ");
      printf("\x1b[13;0H  ~~ rcv mgc err:%d:", numbytes);
      for (i = 0 ; i < 30 ; i++) {
	swiWaitForVBlank();
      }

      perror("recv()");
      //      die("get_update: waiting for return magic");
      die();
    }
    
    printf("\x1b[13;0H                              ");
    printf("\x1b[13;0H  >> got a byte > %d", last_byte);
    swiWaitForVBlank();
    for (i = 0 ; i < 30 ; i++) {
      swiWaitForVBlank();
    }

    if ((last_byte == 24) && 
	(second_to_last_byte == 42)) {
      printf("\x1b[13;0H                              ");
      printf("\x1b[13;0H  >> connected to server");
      swiWaitForVBlank();
      magic_found = 1;
    } 
    
  } // end while (!magic_found)
  
  // read file size, signature, and sample data
  numbytes = recv(server_socket_fd,
		  (void *)&update_signature,
		  4 * 3, 
		  0);
  // check for errors
  if (numbytes < 0) {
    perror("recv()");
    //    die("get_update: waiting for return signature");
    die();
  } else if (numbytes != (4 * 3)) {
    //    die("get_update: return signature incomplete %d/12 bytes received",
    //	numbytes);
    die();
  }
  
  bytes_to_read = update_signature[0];
  
  printf("\x1b[13;0H                              ");
  printf("\x1b[13;0H  >> update is %d KB",
	 bytes_to_read / 1024);
  swiWaitForVBlank();
  
  for (i = 0 ; i < 10 ; i++) {
    swiWaitForVBlank();
  }

  // allocate read buffer
  read_buf = (unsigned char*)malloc(UPD_RD_BUF_SZ);
  if (read_buf == NULL) {
    printf("\x1b[13;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");
    printf("\x1b[13;0H     failed to malloc rdbuf ");
    die();
    return;
  }

  // open output file
  update_file = fopen("/gzmcp/gzmcp-update.nds", "w");
  
  if (update_file == NULL) {
    printf("\x1b[13;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");
    printf("\x1b[13;0H     ~ failed to open updst ");
    return;
  }

  // read up to UPD_RD_BUF_SZ at a time until done
  bytes_to_read = update_signature[0];
  while (bytes_to_read) {
    numbytes = recv(server_socket_fd,
		    (void *)read_buf,
		    UPD_RD_BUF_SZ, 
		    0);
    // check for errors
    if (numbytes < 0) {
      perror("recv()");
      //      die("get_update: while getting update file data");
      die();
    }
    
    // libfat bug workaround
    fseek(update_file,
	  0,
	  SEEK_SET);
    fseek(update_file,
	  0,
	  SEEK_END);
    // write data to file
    // XXX major experiment, swapping 1 and numbytes (now undone)
    fwrite(read_buf,
	   1,
	   numbytes,
	   update_file);
    
    // update amount of work left to do
    bytes_to_read -= numbytes;

    // quick and dirty status update
    printf("\x1b[13;0H                            ");
    printf("\x1b[13;0H  >> %d KB to go...",
	   bytes_to_read / 1024);
    swiWaitForVBlank();
    
  }

  // free read_buf
  free(read_buf);
  
  // close output file
  fclose(update_file);

  // shutdown application-level network state
  netmode_go_offline();

  if (overwrite_self_with_update) {
    // if user wanted to, overwrite ourselves with update (after backup)

    printf("\x1b[13;0H                            ");
    printf("\x1b[13;0H  >> backing up self...");
    swiWaitForVBlank();

    dmc_fs_cp("/gzmcp/gzmcp-client.nds",
	      "/gzmcp/gzmcp-client.bak");

    printf("\x1b[13;0H                            ");
    printf("\x1b[13;0H  >> updating self...");
    swiWaitForVBlank();

    dmc_fs_cp("/gzmcp/gzmcp-update.nds",
	      "/gzmcp/gzmcp-client.nds");

    printf("\x1b[13;0H                            ");
    printf("\x1b[13;0H  >> cleaning up...");
    swiWaitForVBlank();

    unlink("/gzmcp/gzmcp-update.nds");

    printf("\x1b[13;0H                            ");
    printf("\x1b[13;0H  >> rebootstrapping...");
    swiWaitForVBlank();

    execz("/gzmcp/gzmcp-client.nds", 0, NULL);
  } else {
    printf("\x1b[13;0H                            ");
    printf("\x1b[13;0H  >> rebootstrapping...");
    swiWaitForVBlank();

    execz("/gzmcp/gzmcp-update.nds", 0, NULL);
  }
  // error: code should not reach here
}

void mode__get_update___top_renderer(void) {
  
  // 
  // do top background fade
  // 
  if (mode_ms < GET_UPDATE__TOP_BG_FADE_IN_START_MS) {
  } else if (mode_ms < (GET_UPDATE__TOP_BG_FADE_IN_START_MS + 
			GET_UPDATE__TOP_BG_FADE_IN_DURATION_MS)) {
    // main screen bg fade
    REG_BLDY = 31 - ((mode_ms - GET_UPDATE__TOP_BG_FADE_IN_START_MS) * 31 / 
		     GET_UPDATE__TOP_BG_FADE_IN_DURATION_MS);

  } else if (mode_ms < (GET_UPDATE__TOP_BG_FADE_IN_START_MS + 
			GET_UPDATE__TOP_BG_FADE_IN_DURATION_MS +
			GET_UPDATE__TOP_BG_HOLD_DURATION_MS)) {
    REG_BLDY = 0;
  } else if (mode_ms < (GET_UPDATE__TOP_BG_FADE_IN_START_MS + 
			GET_UPDATE__TOP_BG_FADE_IN_DURATION_MS +
			GET_UPDATE__TOP_BG_HOLD_DURATION_MS + 
			GET_UPDATE__TOP_BG_FADE_OUT_DURATION_MS)) {
    REG_BLDY = (mode_ms - 
		(GET_UPDATE__TOP_BG_FADE_IN_START_MS + 
		 GET_UPDATE__TOP_BG_FADE_IN_DURATION_MS +
		 GET_UPDATE__TOP_BG_HOLD_DURATION_MS)) * 
      31 / GET_UPDATE__TOP_BG_FADE_OUT_DURATION_MS;
  }

}

void mode__get_update___bot_renderer(void) {

  unsigned char tfontint = 0;


  // clear the console text
  consoleClear();

  // 
  // do bot background fade
  // 
  if (mode_ms < GET_UPDATE__BOT_BG_FADE_IN_START_MS) {
  } else if (mode_ms < (GET_UPDATE__BOT_BG_FADE_IN_START_MS + 
			GET_UPDATE__BOT_BG_FADE_IN_DURATION_MS)) {
    // main screen bg fade
    REG_BLDY_SUB = 31 - ((mode_ms - GET_UPDATE__BOT_BG_FADE_IN_START_MS) * 31 / 
		     GET_UPDATE__BOT_BG_FADE_IN_DURATION_MS);

  } else if (mode_ms < (GET_UPDATE__BOT_BG_FADE_IN_START_MS + 
			GET_UPDATE__BOT_BG_FADE_IN_DURATION_MS +
			GET_UPDATE__BOT_BG_HOLD_DURATION_MS)) {
    REG_BLDY_SUB = 0;
  } else if (mode_ms < (GET_UPDATE__BOT_BG_FADE_IN_START_MS + 
			GET_UPDATE__BOT_BG_FADE_IN_DURATION_MS +
			GET_UPDATE__BOT_BG_HOLD_DURATION_MS + 
			GET_UPDATE__BOT_BG_FADE_OUT_DURATION_MS)) {
    REG_BLDY_SUB = (mode_ms - 
		(GET_UPDATE__BOT_BG_FADE_IN_START_MS + 
		 GET_UPDATE__BOT_BG_FADE_IN_DURATION_MS +
		 GET_UPDATE__BOT_BG_HOLD_DURATION_MS)) * 
      31 / GET_UPDATE__BOT_BG_FADE_OUT_DURATION_MS;
  }


  // 
  // do bottom text fade
  // 
  if (mode_ms < GET_UPDATE__BOT_TXT_FADE_IN_START_MS) {
  } else if (mode_ms < (GET_UPDATE__BOT_TXT_FADE_IN_START_MS + 
			GET_UPDATE__BOT_TXT_FADE_IN_DURATION_MS)) {
    tfontint = (unsigned char)((int)(font_intensity) * 
			       (mode_ms - GET_UPDATE__BOT_TXT_FADE_IN_START_MS) / 
			       GET_UPDATE__BOT_TXT_FADE_IN_DURATION_MS);
  } else if (mode_ms < (GET_UPDATE__BOT_TXT_FADE_IN_START_MS + 
			GET_UPDATE__BOT_TXT_FADE_IN_DURATION_MS +
			GET_UPDATE__BOT_TXT_HOLD_DURATION_MS)) {
    tfontint = (unsigned char)((int)font_intensity * 1);
  } else if (mode_ms < (GET_UPDATE__BOT_TXT_FADE_IN_START_MS + 
			GET_UPDATE__BOT_TXT_FADE_IN_DURATION_MS +
			GET_UPDATE__BOT_TXT_HOLD_DURATION_MS + 
			GET_UPDATE__BOT_TXT_FADE_OUT_DURATION_MS)) {
    tfontint = ((unsigned char)((int)font_intensity * 1)) -
      (unsigned char)((int)(font_intensity) * 
		      (mode_ms - (GET_UPDATE__BOT_TXT_FADE_IN_START_MS +
				  GET_UPDATE__BOT_TXT_FADE_IN_DURATION_MS +
				  GET_UPDATE__BOT_TXT_HOLD_DURATION_MS)) /
		      GET_UPDATE__BOT_TXT_FADE_OUT_DURATION_MS);
  }

  // set the font color from the calculated intensity (greenish)
  BG_PALETTE_SUB[255] = RGB15(tfontint / 3,
			      tfontint,
			      tfontint / 3);

  // tell user they can skip the intro
  printf("\x1b[05;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");
  printf("\x1b[06;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");

  printf("\x1b[08;0H           Downloading      ");

  printf("\x1b[10;0H        Updated Software    ");

  printf("\x1b[12;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");
  printf("\x1b[13;0H     ~~~~~~~~~~~~~~~~~~~~~~ ");


}

void mode__get_update___input_handler(void) {
  mode__intro__main___input_handler();
}

void mode__get_update___idle(void) {
}

void mode__get_update___exit(void) {

}
