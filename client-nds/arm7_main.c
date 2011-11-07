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
## gzmcpc::arm7_main: arm7 code (generally administrative)
##
#############################################################################
##
## Copyright 2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/



#include <nds.h>

#include <dswifi7.h>

#include <maxmod7.h>

#include "arm7_main.h"







int main(void) {

  // retrieve user customization/configuration data from non volatile memory
  readUserSettings();

  // power on the sound subsystem
  powerOn(POWER_SOUND);

  // initialize interrupt requests
  irqInit();

  // disable(?) wifi irq at first
  irqSet(IRQ_WIFI, 0);

  // initialize interprocessor communication pipe
  fifoInit();

  // ?? set the point during screen refresh when vblank handler is called ??
  SetYtrigger(80);

  // set up interprocessor sound data pipe
  installSoundFIFO();

  // set up interprocessor wifi data pipe
  installWifiFIFO();

  // set up interprocessor highlevel audio (maxmod) data pipe
  mmInstall(FIFO_MAXMOD);

  // set up interprocessor generic data pipe
  installSystemFIFO();

  // every vertical blank(?), process any new input
  irqSet(IRQ_VCOUNT, inputGetAndSend);

  // every vertical blank, process wifi transactions
  irqSet(IRQ_VBLANK, Wifi_Update);

  // set up the Real Time Clock handler
  initClockIRQ();

  // actually enable the now set up interrupt handlers
  irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK );

  // other than handling interrupts, just check for reset request
  while (1) {
    
    checkForAndHandleResetRequest();
  
    // only run the rest of the loop once per vert refresh
    swiWaitForVBlank();

  }

  // execution never reaches here
  return 0;

}


void checkForAndHandleResetRequest(void) {

  // did the arm9 request a reset?
  if (*((vu32*)0x027ffe24) == (u32)0x027ffe04) {

    // cease running interrupt handlers
    irqDisable(IRQ_ALL);
    
    // passme loop (?) magic incantation
    *((vu32*)0x027ffe34) = (u32)0x06000000;

    // reset
    swiSoftReset();

  }

}
