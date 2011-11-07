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
## gzmcpc::graphics: visual output rendering
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

#include <stdio.h>

#include <stdarg.h>

#include <math.h> 


#include "graphics.h"

#include "debug.h"

#include "input.h"

#include "mcp.h"

#include "modes.h"

#include "mode__tpw__jam.h"

#include "network.h"

#include "sound.h"

#include "time.h"


#include "sounds.h"

#include "sounds_bin.h"




char text_output_line[TLT_NUM_TEXT_OUTPUT_LINE_TYPES][MAX_TEXT_OUTPUT_LINE_SIZE];

float cursor_rotation = 0.0f;

float cursor_intensity = DEFAULT_CURSOR_INTESITY;

v16 touch_pos_x = 128;
v16 touch_pos_y = 96;

int cursor_rotating = 1;

unsigned char font_intensity = DEFAULT_FONT_INTENSITY;

PrintConsole top_screen;
PrintConsole bottom_screen;
PrintConsole top_screen_x;
PrintConsole bottom_screen_x;

Keyboard keyboard;

float sqrt_three;

unsigned char top_render_rate = DEFAULT_TOP_RENDER_RATE;
unsigned char bottom_render_rate = DEFAULT_BOTTOM_RENDER_RATE;
time_val next_top_render = {0, 0};
time_val next_bottom_render = {0, 0};

int bg0, bg1, bg2, bg3;
int bgs0, bgs1, bgs2, bgs3;






void gzmcpc_init_3d(void) {

  int i, j;

  // initialize openGL-ish 3d engine
  glInit();
  
  // initialize comb charge matrix
  for (i = 0; i < COMB_WIDTH ; i++) { 
    for (j = 0; j < COMB_WIDTH ; j++) { 
      comb_charge[i][j] = 0;
    }
  }

  // single value lut
  sqrt_three=sqrtf(3.0f);
  
  // enable antialiasing
  glEnable(GL_ANTIALIAS);
  
  // background must be opaque for antialiasing 
  glClearColor(0,0,0,31);

  // background must have a unique polyID for antialiasing
  glClearPolyID(63);

  glClearDepth(0x7FFF);
  
  // enable translucency
  // XXX: haven't actually figured out nds opengl-ish alpha yet
  //  glEnable(GL_BLEND);
  
  // real OGL
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // nds OGL: use as a state, thus polys (and by that we mean tris or quads),
  //          will have this 0..15 alpha value
  // POLY_CULL_NONE for now
  glPolyFmt(POLY_ALPHA(15) | POLY_CULL_NONE);
  //glPolyFmt(POLY_CULL_NONE);

  // use threshold below ?? need to read up...
  // XXX: haven't actually figured out nds opengl-ish alpha yet
  //  glEnable(GL_ALPHA_TEST);
  // 0..15 from nds, not sure ... 4 bits alpha?
  // "sets minimum alpha value that will be used" ???
  //  glAlphaFunc(7);

  ////	  
  //// begin effective window reshape/init function
  ////	  

  // set viewport to the screen resolution
  glViewport(0,0,255,191);
  
  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();

  // set 3d perspective 'camera' type/parameters:
  // fov(?), aspect-ratio, nearz, farz
  //  gluPerspective(70, 256.0 / 192.0, 0.1, 100);

  // set 2d orthographic 'camera' type/parameters:
  glOrthof32(MODEL_XMIN,
	     MODEL_XMAX,
	     MODEL_YMIN,
	     MODEL_YMAX,
	     MODEL_ZMIN,
	     MODEL_ZMAX);

  glMatrixMode(GL_MODELVIEW);

  ////	  
  //// end effective window reshape/init function
  ////	  

  // nds openGL-ish: several nds specific attributes can be set
  glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
  

}  



void gzmcpc_display_main_splash_text(void) {

  //  printf("\x1b[04;0H |----------------------------| ");
      printf("\x1b[02;0H       Guitar-ZyX::M.C.P.       ");

      printf("\x1b[04;0H     Master Control Program     ");

}  



void textout(text_output_line_type linetype, const char *fmt, ...) {

  //
  // XXX would making these vars static help at all?
  //

  va_list arg;

  char message_prefix[MAX_TEXT_OUTPUT_LINE_SIZE];
  char temp_buffer[MAX_TEXT_OUTPUT_LINE_SIZE];

  // XXX is there a cleaner way to do this with an array (effective hash) initialization?
  switch (linetype) {
    
  case TLT_DEBUG:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     "DBG::");
    break;
  case TLT_ERROR:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     "ERROR: ");
    break;
  case TLT_FX_MODE:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     " F/X Mode /\\/ ");
    break;
  case TLT_MODE:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     "   MCP /\\/ ");
    break;
  case TLT_NETWORK_STATUS:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     " network /\\/ ");
    break;
  case TLT_STATUS:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	    " status /\\/ ");
    break;
  case TLT_TOUCH_VAL:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     " WhAmMyPaD /\\/ ");
    break;
  case TLT_TOUCH_VALXP:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     "   WPX// ");
    break;
  case TLT_TOUCH_VALYP:
    snprintf(message_prefix, 
	     MAX_TEXT_OUTPUT_LINE_SIZE,
	     "     WPY// ");
    break;
  case TLT_NODEBUG:
    //snprintf(message_prefix, MAX_TEXT_OUTPUT_LINE_SIZE"");
    // nicer way to avoid warning??
    message_prefix[0]='\0';
    break;
  case TLT_AESTHETIC:
    //snprintf(message_prefix, MAX_TEXT_OUTPUT_LINE_SIZE"");
    message_prefix[0]='\0';
    break;
  case TLT_NUM_TEXT_OUTPUT_LINE_TYPES:
    // handling all enum values, even the utility one avoids compiler warning
    break;
  }

  va_start(arg, fmt);
  vsnprintf(temp_buffer, 
	    MAX_TEXT_OUTPUT_LINE_SIZE, 
	    fmt, 
	    arg);
  va_end(arg);

  snprintf(text_output_line[linetype],
	   MAX_TEXT_OUTPUT_LINE_SIZE,
	   "%s%s", 
	   message_prefix, 
	   temp_buffer);

  debug_log_more("textout::%s\n",
		 text_output_line[linetype]);
}



void render_textout(void) {

  int i;

  // 3 should work too (max 99 would be fine)
  char rownum[4];


  // clear the screen
  printf("\x1b[2J");

  // the banner
  gzmcpc_display_main_splash_text();

  for (i = 0 ; i < TLT_NUM_TEXT_OUTPUT_LINE_TYPES ; i++) {

    // XXX as mentioned elsewhere, this screams for a hash, but don't care now
    switch (i) {
    case TLT_DEBUG:
      snprintf(rownum, 4, "%d", TS_DEBUG_ROW);
      break;
    case TLT_ERROR:
      snprintf(rownum, 4, "%d", TS_ERROR_ROW);
      break;
    case TLT_FX_MODE:
      snprintf(rownum, 4, "%d", TS_FX_MODE_ROW);
      break;
    case TLT_MODE:
      snprintf(rownum, 4, "%d", TS_MODE_ROW);
      break;
    case TLT_NETWORK_STATUS:
      snprintf(rownum, 4, "%d", TS_NETWORK_STATUS_ROW);
      break;
    case TLT_STATUS:
      snprintf(rownum, 4, "%d", TS_STATUS_ROW);
      break;
    case TLT_TOUCH_VAL:
      // XXX uh, yeah, inconsistent
      snprintf(rownum, 4, "%d", TS_TOUCH_VAL_ROW);
      break;
    case TLT_TOUCH_VALXP:
      snprintf(rownum, 4, "%d", TS_TOUCH_VALXP_ROW);
      break;
    case TLT_TOUCH_VALYP:
      snprintf(rownum, 4, "%d", TS_TOUCH_VALYP_ROW);
      break;
    case TLT_NODEBUG:
      snprintf(rownum, 4, "%d", TS_DEBUG_ROW);
      break;
    case TLT_AESTHETIC:
      snprintf(rownum, 4, "%d", TS_AESTHETIC_ROW);
      break;
    }

    // XXX obsolete due to screen clear?
    // note: with this, 345 wraps
    //    printf("\x1b[%s;0H12345678901234567890123456789012345", rownum);
    printf("\x1b[%s;0H                                ", rownum);

    printf("\x1b[%s;0H%s", rownum, text_output_line[i]);

  } // end for iteration over linetypes

}



void init_modelview_matrix(void) {

  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();

  // for non-ortho 3d
  //  void gluLookAtf32(int32 eyex, int32 eyey, int32 eyez, 
  //		    int32 lookAtx, int32 lookAty, int32 lookAtz, 
  //		    int32 upx, int32 upy, int32 upz)

  // global (animation?) xforms if desired...

  // move the camera, currently at the default origin looking
  // down the negative Z axis, backwards half the depth, thus
  // will use z=0 for all z's in this 2d ortho world.  Much of
  // this is for explicit educational value (though don't believe
  // any of it till initial code reviews are done)
  //
  // note: you wouldn't use divf32 instead of '/' unless you also
  //       used inttof32(-4).  Fixed point sucks.  Especially because
  //       f32(1.19.12) and v16(1.3.12) are both used in what seems
  //       ugly- i.e. f32 only for glOrtho, and v16 only for glVertex,
  //       thus confusing.
  glTranslate3f32(0,
		  0,
		  model_depth / -4);


  // example simple time based animation camera xform
  // - side to side motion, distance
  //
  //  if (num_ticks.s % 2) {
  //    glTranslate3f32((model_width / 8) *
  //		    (num_ticks.ms / 1000), 
  //		    0, 
  //		    0);
  //  } else {
  //    glTranslate3f32((model_width / 8) *
  //		    (1 - (num_ticks.ms / 1000)), 
  //		    0, 
  //		    0);
  //  }

}

void 
draw_hex(v16 centerx, v16 centery, 
	 v16 z, 
	 v16 radius, 
	 float rotation,
	 uint8 centerr, uint8 centerg, uint8 centerb, uint8 centera,
	 uint8 outterr, uint8 outterg, uint8 outterb, uint8 outtera,
	 uint8 rimr, uint8 rimg, uint8 rimb, uint8 rima,
	 u16 intensity)
{

  uint8 act_center_r;
  uint8 act_center_g;
  uint8 act_center_b;
  uint8 act_outter_r;
  uint8 act_outter_g;
  uint8 act_outter_b;


  act_center_r = (uint8)((u16)centerr * (u16)(RGB15_TO_R5(intensity)) / 31);
  act_center_g = (uint8)((u16)centerg * (u16)(RGB15_TO_G5(intensity)) / 31);
  act_center_b = (uint8)((u16)centerb * (u16)(RGB15_TO_B5(intensity)) / 31);
  act_outter_r = (uint8)((u16)outterr * (u16)(RGB15_TO_R5(intensity)) / 31);
  act_outter_g = (uint8)((u16)outterg * (u16)(RGB15_TO_G5(intensity)) / 31);
  act_outter_b = (uint8)((u16)outterb * (u16)(RGB15_TO_B5(intensity)) / 31);

  // save current modelview matrix
  // note: I think I am following the global method of thinking 
  //       rather than the local method, as described in the red
  //       book (p106 2nd ed).  
  glPushMatrix();

  // move to the desired center/location
  glTranslate3f32(centerx,
		  centery,
		  z);

  // rotate around the z axis
  glRotatef(rotation + 30.0f,0.0f,0.0f,1.0f);
    

#define P_A_X (0)
#define P_A_Y (0)
#define P_B_X (radius * -1)
#define P_B_Y (0)
#define P_C_X ((radius >> 1) * -1)
  // XXX? single value lut?
#define P_C_Y (mulf32(P_C_X, sqrtf32(inttof32(3))))
#define P_D_X (P_C_X * -1)
#define P_D_Y (P_C_Y)
#define P_E_X (radius)
#define P_E_Y (0)
#define P_F_X P_D_X
#define P_F_Y (P_D_Y * -1)
#define P_G_X ((radius >> 1) * -1)
#define P_G_Y P_F_Y
    
  glBegin(GL_TRIANGLE_STRIP);
    
  glColor3b(act_outter_r, act_outter_g, act_outter_b);

  // f-g-a-b-c
  glColor3b(act_outter_r, act_outter_g, act_outter_b);
  glVertex3v16(P_F_X, P_F_Y, z);
  glVertex3v16(P_G_X, P_G_Y, z);
  glColor3b(act_center_r, act_center_g, act_center_b);
  glVertex3v16(P_A_X, P_A_Y, z);
  glColor3b(act_outter_r, act_outter_g, act_outter_b);
  glVertex3v16(P_B_X, P_B_Y, z);
  glVertex3v16(P_C_X, P_C_Y, z);

  // c-d-a-e-f
  glColor3b(act_outter_r, act_outter_g, act_outter_b);
  glVertex3v16(P_C_X, P_C_Y, z);
  glVertex3v16(P_D_X, P_D_Y, z);
  glColor3b(act_center_r, act_center_g, act_center_b);
  glVertex3v16(P_A_X, P_A_Y, z);
  glColor3b(act_outter_r, act_outter_g, act_outter_b);
  glVertex3v16(P_E_X, P_E_Y, z);
  glVertex3v16(P_F_X, P_F_Y, z);
  
  glEnd();
  glPopMatrix(1);

}
 

void 
draw_simple_hex_grid(int width, 
		     int height, 
		     u16 intensity) {

  uint32 myx, myy;
  int i, j;
  v16 myz;
  uint8 comb_center_r;
  uint8 comb_center_g;
  uint8 comb_center_b;
  float anim_rotate;
  float anim_fraction;

  //
  // simple intro animation
  //

  // crude method to stop z-fighting (no glDisable(GL_DEPTH_BUFFERING)?)
  // i.e. below myz++ happens for each hex rendered
  myz = floattov16(0.25f);

  // XXX: this is clumsy for now (i don't know or care what I'm doing)
  glPushMatrix();
  glTranslate3f32(model_width >> 1,
		  model_height >> 1,
		  0);

  //
  // intro animation
  //
  if (mode_ms < INTRO_ANIM_3DSPINZOOM_START) {
    glScalef(INTRO_ANIM_3DSPINZOOM_SCALE_FACTOR * 1.0f, 
	     INTRO_ANIM_3DSPINZOOM_SCALE_FACTOR * 1.0f, 
	     1.0f);
  } else if (mode_ms < (INTRO_ANIM_3DSPINZOOM_START + INTRO_ANIM_3DSPINZOOM_DURATION)) {

    // a float from 0.0 to 1.0 representing how far along during the 3DSPINZOOM animation
    anim_fraction = ((float)(mode_ms - INTRO_ANIM_3DSPINZOOM_START)) /
      (INTRO_ANIM_3DSPINZOOM_DURATION * 1.0f);
    
    glScalef(((INTRO_ANIM_3DSPINZOOM_SCALE_FACTOR * 1.0f) - 
	      anim_fraction * 
	      (INTRO_ANIM_3DSPINZOOM_SCALE_FACTOR - 1.0f)),
	     ((INTRO_ANIM_3DSPINZOOM_SCALE_FACTOR * 1.0f) - 
	      anim_fraction * 
	      (INTRO_ANIM_3DSPINZOOM_SCALE_FACTOR - 1.0f)),
	     1.0f);

    anim_rotate = anim_fraction * -360.0f;

    glRotatef(anim_rotate,
	      0.0f,0.0f,1.0f);
  }


  comb_center_r = 13;
  comb_center_g = 13;
  comb_center_b = 13;

  comb_center_r = cstate_wpy << 1;
  comb_center_g = 0;
  comb_center_b = cstate_wpx << 1;

  // iterate over the 2D hex grid, prepping and drawing cells
  for (j = 0 ; j < height ; j++) {
    for (i = 0 ; i < width ; i++) {

      comb_center_g = comb_charge[i][j];
      
      // offset odd rows a bit to the right
      if (j % 2) {
	myx = MODEL_XMIN + ((MODEL_XMAX - MODEL_XMIN) * COMB_XBORDER / 100) +
	  (i  * (((MODEL_XMAX - MODEL_XMIN) * (100 - 2 * COMB_XBORDER) / 100) / (width - 1)));
      } else {
	myx = MODEL_XMIN + ((MODEL_XMAX - MODEL_XMIN) * COMB_XBORDER / 100) +
	  ((2 * i + 1) * (((MODEL_XMAX - MODEL_XMIN) * (100 - 2 * COMB_XBORDER) / 100) / 2 / (width - 1)));
      }
      
      // this formula is a hack, but achieves reasonable packing (for me, now)
      // AHH, now I remember the old code.  The reason for the border may
      //      have been part of this hack, i.e. with a different value 
      //      than 10 / 6 below, and maybe better looking result (if
      //      my tired rememory is correct)
      myy = MODEL_YMIN + ((MODEL_YMAX - MODEL_YMIN) * COMB_YBORDER / 100) +
	(j * (((MODEL_YMAX - MODEL_YMIN) * (100 - 2 * COMB_YBORDER) / 100) / (height - 1)));
      
      // note: in the trenches pitfall of fixed point hazard
      //      myx = MODEL_XMIN + ((4 * i + 2) / 4) * (model_width / width);
      //      myy = MODEL_YMIN + ((4 * j + 2) / 4) * (model_height / height);
      // note: this works
      //      myx = MODEL_XMIN + (model_width / width * (4 * i + 2)) / 4;
      //      myy = MODEL_YMIN + (model_height / height * (4 * j + 2)) / 4;

      // draw the comb cell 
      // note: ... / 2 * 2, when was / 2 * 4, looked pretty overlappn cool
      draw_hex(myx - (model_width >> 1), 
	       myy - (model_height >> 1), 
	       myz++,
	       ((((MODEL_XMAX - MODEL_XMIN) + 
		  (MODEL_YMAX - MODEL_YMIN)) / 2) / 
		(width + height) * debug_tweak_var / 6),
	       0,
	       comb_center_r, 
	       comb_center_g, 
	       comb_center_b, 
	       13,
	       13, 197, 13, 231,
	       228, 228, 114, 253,
	       intensity);
    }
  }

  glPopMatrix(1);
}


void gzmcpc_flush_frame(void) {

  // XXX: maybe this should be made per mode
  glFlush(0);

  // wait for vblank
  swiWaitForVBlank();

}
