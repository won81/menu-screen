 /*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.1 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://floralicense.org/license/
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */



#ifndef __MENU_SCREEN_ITEM_H__
#define __MENU_SCREEN_ITEM_H__

#include <Evas.h>

#include "list.h"
#include "util.h"

extern char *item_get_icon(Evas_Object *item);
extern void item_set_icon(Evas_Object *item, char *icon, int sync);

extern char *item_get_name(Evas_Object *item);
extern void item_set_name(Evas_Object *item, char *name, int sync);

extern char *item_get_package(Evas_Object *item);
extern void item_set_package(Evas_Object *item, char *package, int sync);

extern int item_get_removable(Evas_Object *item);
extern void item_set_removable(Evas_Object *item, int removable, int sync);

extern Evas_Object *item_get_page(Evas_Object *edje);
extern void item_set_page(Evas_Object *edje, Evas_Object *page, int sync);

extern char *item_get_desktop(Evas_Object *item);
extern void item_set_desktop(Evas_Object *item, char *name, int sync);

extern int item_get_type(Evas_Object *item);
extern void item_set_type(Evas_Object *edje, int type, int sync);

extern void item_enable_delete(Evas_Object *obj);
extern void item_disable_delete(Evas_Object *item);

extern void item_show_badge(Evas_Object *obj, int value);
extern void item_hide_badge(Evas_Object *obj);
extern int item_is_enabled_badge(Evas_Object *obj);

extern void item_edit(Evas_Object *item);
extern void item_unedit(Evas_Object *item);

extern Evas_Object *item_create(Evas_Object *scroller, app_info_t* ai);
extern void item_update(Evas_Object *item, app_info_t *ai);
extern void item_destroy(Evas_Object *obj);

extern void item_launch(Evas_Object *obj);

extern void item_enable_progress(Evas_Object *obj);
extern int item_is_enabled_progress(Evas_Object *obj);
extern void item_update_progress(Evas_Object *obj, int value);
extern void item_disable_progress(Evas_Object *obj);

extern menu_screen_error_e item_is_edje_icon(const char *icon);
extern int item_get_position(Evas_Object *item);

extern void item_mark_dirty(Evas_Object *item);
extern void item_unmark_dirty(Evas_Object *item);
extern int item_is_dirty(Evas_Object *item);

#endif //__MENU_SCREEN_ITEM_H__

// End of a file
