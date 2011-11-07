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
## gzmcpc::mode__main_menu: the main menu(/command-tree) system header file
#
#############################################################################
##
## Copyright 2008-2009 Douglas McClendon <dmc AT filteredperception DOT org>
##
#############################################################################
#############################################################################
#
*/

#ifndef _NDS_CLIENT_MODE__MAIN_MENU_H
#define _NDS_CLIENT_MODE__MAIN_MENU_H



#define MENU_LABEL_MAXLEN 256

#define FS_PATH_MAXLEN 1023

#define MM_TXT_DEFAULTCOLOR (RGB15(31 / 2, 31, 31 / 2))
#define MM_HILITE_DEFAULTCOLOR (RGB15(31 / 3, 0, 0))
#define MM_BG_BMP dlavaBitmap
#define MM_BG_PAL dlavaPal

#define MM_HILITE_COLOR_INDEX 253
#define MM_FONT_COLOR_INDEX 187

#define MAIN_MENU__TOP_BG_FADE_IN_START_MS 0
#define MAIN_MENU__TOP_BG_FADE_IN_DURATION_MS 1000
#define MAIN_MENU__TOP_BG_FADE_OUT_DURATION_MS 1000

#define MAIN_MENU__BOT_BG_FADE_IN_START_MS 500
#define MAIN_MENU__BOT_BG_FADE_IN_DURATION_MS 500
#define MAIN_MENU__BOT_BG_FADE_OUT_DURATION_MS 1000

#define MAIN_MENU__BOT_TXT_FADE_IN_START_MS 750
#define MAIN_MENU__BOT_TXT_FADE_IN_DURATION_MS 750
#define MAIN_MENU__BOT_TXT_FADE_OUT_DURATION_MS 500

#define MAIN_MENU__BOT_TXT_INTRAMODE_FADE_DURATION_MS 250










typedef struct gzmcp_menu_node gzmcp_menu_node;
struct gzmcp_menu_node {

  // the name of this node/item
  char label[MENU_LABEL_MAXLEN + 1];

  // whether or not the item is selectable
  int selectable;

  // may want to add type(file_ogg, file_nds, dir, setting,...)
  // may want to add descriptive text and other metadata to display on top

  // menu node selection callback function will be run if this
  //     item is selected
  void (*select_cb)(void);

  // menu node hilite callback function
  // note: for now, I'll let top reindeer handle most things that might
  //       apply to this
  //  void (*hilite_cb)(void);

  // the next node in this level/directory's linked list
  gzmcp_menu_node *next;

};




void mode__main_menu___init(void);
void mode__main_menu___top_renderer(void);
void mode__main_menu___bot_renderer(void);
void mode__main_menu___input_handler(void);
void mode__main_menu___idle(void);
void mode__main_menu___exit(void);


void mm_set_doublemode(int mode);

void render_hilite(int bgid,
		   int color,
		   int y_offset,
		   int width,
		   int height,
		   int roundness,
		   int unrender);

void mm_add_node(const char *label,
		 int selectable,
		 void(*select_cb)(void));

void mm_free(void);

void mm_alphasort(gzmcp_menu_node *head, 
		  int from_index, 
		  int to_index);

gzmcp_menu_node *mm_get_node_by_index(int index);

int mm_get_index_by_label(const char *label);

void mm_set_selection_by_index(int index);

void mm_set_selection_by_label(const char *label);

void mm_set_selection_by_hist(void);

void mm_print(void);

void mm_hist_push(const char *label);
char *mm_hist_peek(void);
void mm_hist_pop(void);

void mm_go_whammypad(void);
void mm_go_settings(void);
void mm_go_files(void);
void mm_go_usermanual(void);
void mm_go_misc(void);
void mm_go_shutdown(void);

void mm_go_get_update(void);
void mm_go_get_and_burn_update(void);
void mm_go_ssid_input(void);
void mm_go_show_credits(void);
void mm_go_show_ssid_list(void);

void mm_go_back(void);
void mm_go_misc_back(void);
void mm_go_files_back(void);

void mm_go_files_entry(void);

void mm_create(void (*create_func)(void));
void mm_create_last(void);
void mm_create_root(void);
void mm_create_misc(void);
void mm_create_files(void);




extern gzmcp_menu_node *mm_list;
extern gzmcp_menu_node *mm_hist;

#endif // _NDS_CLIENT_MODE__MAIN_MENU_H
