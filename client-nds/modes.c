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
## gzmcpc::modes: some source file
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

#include "mcp.h"

#include "modes.h"

#include "mode__intro__main.h"
#include "mode__intro__credits.h"
#include "mode__main_menu.h"
#include "mode__get_update.h"
#include "mode__ssid__input.h"
#include "mode__tpw__jam.h"




mode_def modes[MODE_NUM_MODES] = {
  {  
    mode__intro__main___init,
    mode__intro__main___top_renderer,
    mode__intro__main___bot_renderer,
    mode__intro__main___input_handler,
    mode__intro__main___idle,
    mode__intro__main___exit,
  },
  {  
    mode__intro__credits___init,
    mode__intro__credits___top_renderer,
    mode__intro__credits___bot_renderer,
    mode__intro__credits___input_handler,
    mode__intro__credits___idle,
    mode__intro__credits___exit,
  },
  {  
    mode__main_menu___init,
    mode__main_menu___top_renderer,
    mode__main_menu___bot_renderer,
    mode__main_menu___input_handler,
    mode__main_menu___idle,
    mode__main_menu___exit,
  },
  {  
    mode__get_update___init,
    mode__get_update___top_renderer,
    mode__get_update___bot_renderer,
    mode__get_update___input_handler,
    mode__get_update___idle,
    mode__get_update___exit,
  },
  {  
    mode__ssid__input___init,
    mode__ssid__input___top_renderer,
    mode__ssid__input___bot_renderer,
    mode__ssid__input___input_handler,
    mode__ssid__input___idle,
    mode__ssid__input___exit,
  },
  {
    mode__tpw__jam___init,
    mode__tpw__jam___top_renderer,
    mode__tpw__jam___bot_renderer,
    mode__tpw__jam___input_handler,
    mode__tpw__jam___idle,
    mode__tpw__jam___exit,
  },
};

time_val mode_start = {0, 0};

long mode_ms = 0;
long exit_mode_ms = 0;

gzmcp_mode last_mode = MODE_NUM_MODES;

gzmcp_mode mode = MODE_NUM_MODES;


#ifdef FMODE
gzmcp_mode next_mode = FMODE;
gzmcp_mode new_mode = FMODE;
#else
gzmcp_mode next_mode = MODE_INTRO__MAIN;
gzmcp_mode new_mode = MODE_INTRO__MAIN;
#endif


  

void system_xmode_new(gzmcp_mode mcp_mode) {
  // mode transitions cannot happen once started
  if (mode == next_mode) {
    next_mode = mcp_mode;
    exit_mode_ms = mode_ms;
  }
}


void system_xmode_reinit(void) {

  int i;

  //
  // note, the following memory clearing might be good practice
  //       (better with dmafill perhaps), but hasn't yet proven
  //       to improve anything.
  //

  // experiment: clear memory banks
  // 256 * 256 * 2 = 128k 
  for (i = 0 ; i < 256 * 256 ; i++) {
    VRAM_A[i] = (u16)0;
  }

  // 128k
  for (i = 0 ; i < 256 * 256 ; i++) {
    VRAM_C[i] = (u16)0;
  }

  // 32k
  for (i = 0 ; i < 64 * 256 ; i++) {
    VRAM_H[i] = (u16)0;
  }

  // 16k
  for (i = 0 ; i < 32 * 256 ; i++) {
    VRAM_I[i] = (u16)0;
  }

  videoBgDisableSub(0);
  videoBgDisableSub(1);
  videoBgDisableSub(2);
  videoBgDisableSub(3);
  videoBgDisable(0);
  videoBgDisable(1);
  videoBgDisable(2);
  videoBgDisable(3);

  BG_PALETTE[255] = RGB15(0, 0, 0);
  BG_PALETTE_SUB[255] = RGB15(0, 0, 0);

  swiWaitForVBlank();

  // set main screen to be on the top lcd display
  lcdMainOnTop();

  // init screens to non-blending
  REG_BLDCNT = BLEND_NONE;
  REG_BLDCNT_SUB = BLEND_NONE;

  // default to fully faded to black
  mcp_set_blend(MCP_MAIN_SCREEN, MCP_MAX_BLEND_LEVEL);
  mcp_set_blend(MCP_SUB_SCREEN, MCP_MAX_BLEND_LEVEL);

}


void system_xmode_real(void) {
  new_mode = next_mode;
}

