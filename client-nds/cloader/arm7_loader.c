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
#include <nds.h>
#include <dswifi7.h>

void runNdsLoaderCheck (void) {
  if(*((vu32*)0x027FFE24) == (u32)0x027FFE04) {
    // exp
    Wifi_Deinit();
    irqDisable (IRQ_ALL);
    *((vu32*)0x027FFE34) = (u32)0x06000000;
    swiSoftReset();
  } 
}
