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



#ifndef __MENU_SCREEN_PAGE_H__
#define __MENU_SCREEN_PAGE_H__

#include <Evas.h>
#include "util.h"
#include "list.h"

extern void page_mark_dirty(Evas_Object *page);
extern void page_clean_dirty(Evas_Object *page);
extern void page_unmark_dirty(Evas_Object *page);
extern int page_is_dirty(Evas_Object *page);

extern Evas_Object *page_get_item_at(Evas_Object *menu, unsigned int idx);

extern menu_screen_error_e page_unpack_item(Evas_Object *page, Evas_Object *item);
extern Evas_Object *page_unpack_item_at(Evas_Object *page, int idx);

extern void page_pack_item(Evas_Object *menu, int idx, Evas_Object *item);
extern void page_set_item(Evas_Object *page, int idx, Evas_Object *item);

extern unsigned int page_count_item(Evas_Object *page);

extern Evas_Object *page_create(Evas_Object *scroller, int idx, int rotate);
extern void page_destroy(Evas_Object *scroller, Evas_Object *page);

extern int page_find_empty_near(Evas_Object *menu_edje, int pivot);
extern int page_find_first_empty(Evas_Object *page, int pivot);
extern void page_trim_items(Evas_Object *page);

#endif //__MENU_SCREEN_PAGE_H__
// End of a file

