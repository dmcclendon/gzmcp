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
## gzmcpc::time: timekeeping functions
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

#include "time.h"

#include "debug.h"




char timer_num_ticks = 0;

time_val num_ticks = {0, 0};



int time_val_compare(time_val time_a, time_val time_b) {
  
  if (time_b.s > time_a.s) {
    return 1;
  } else if (time_b.s < time_a.s) {
    return -1;
  } else if (time_b.ms > time_a.ms) {
    return 1;
  } else if (time_b.ms < time_a.ms) {
    return -1;
  } else {
    return 0;
  }
}

time_val time_val_add_ms(time_val time_a, int ms) {
  
  time_val return_value;

  return_value = time_a;
  return_value.ms += ms;
  return_value.s += return_value.ms / 1000;
  return_value.ms = return_value.ms % 1000;

  return return_value;
}

long ms_since(time_val time) {
  return (num_ticks.s - time.s) * 1000 + (num_ticks.ms - time.ms);
}

void ms_timer_handler(void) {

  timer_num_ticks++;

  if (timer_num_ticks == 16) {

    timer_num_ticks = 0;

    num_ticks.ms++;
    
    if (num_ticks.ms == 1000) {
      num_ticks.s++;
      num_ticks.ms = 0;
      
      //    if (num_ticks.s == 3600 * 24 * 366) {
      if (num_ticks.s == 31622400) {
	// one year is old enough for this program
	// XXX ... instead of exerting the mental effort to consider
	//         all wrapping situations 
	die();
      }

    }

  }

}

void gzmcpc_init_time(void) {

  TIMER0_CR = 0;

  // 16khz timer
  TIMER_DATA(0) = TIMER_FREQ(16384);

  // note: emacs syntax tab auto formatting did this to the line extensions.
  TIMER_CR(0) =					\
    TIMER_ENABLE |				\
    TIMER_DIV_1 |				\
    TIMER_IRQ_REQ;

  // configure gzmcp tick timer interrupt handler
  irqSet(IRQ_TIMER0, ms_timer_handler);

  // enable the ms timer irq
  irqEnable(IRQ_TIMER0);

}


