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



#ifndef __MENU_SCREEN_BADGE_H__
#define __MENU_SCREEN_BADGE_H__

#include <stdbool.h>

#include "util.h"

extern menu_screen_error_e item_badge_register(Evas_Object *item);
extern void item_badge_unregister(Evas_Object *item);
extern bool item_badge_is_registered(Evas_Object *item);
extern int item_badge_count(char *package);

extern void item_badge_register_changed_cb(Evas_Object *scroller);
extern void item_badge_unregister_changed_cb(void);

#endif
// End of a file
