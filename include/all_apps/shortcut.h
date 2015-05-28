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



#ifndef _MENU_SCREEN_ALL_APPS_SHORTCUT_H_
#define _MENU_SCREEN_ALL_APPS_SHORTCUT_H_

#include <Elementary.h>
#include <stdbool.h>

extern Evas_Object *all_apps_shortcut_add(Evas_Object *scroller, long long rowid, const char *pkgname, const char *exec, const char *name, const char *icon, int type);
extern void all_apps_shortcut_remove(Evas_Object *item);

extern menu_screen_error_e all_apps_shortcut_add_all(Evas_Object *scroller);

extern bool all_apps_shortcut_init(Evas_Object *all_apps);
extern void all_apps_shortcut_fini(void);

#endif // _MENU_SCREEN_ALL_APPS_SHORTCUT_H_

// End of a file
