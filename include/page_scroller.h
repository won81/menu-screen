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



#ifndef __MENU_SCREEN_PAGE_SCROLLER_H__
#define __MENU_SCREEN_PAGE_SCROLLER_H__

#include <stdbool.h>
#include <Eina.h>
#include <Evas.h>
#include "list.h"

typedef enum {
	PAGE_SCROLLER_SORT_BY_DEFAULT = 0,
	PAGE_SCROLLER_SORT_BY_PACKAGE,
	PAGE_SCROLLER_SORT_BY_NAME,
	PAGE_SCROLLER_SORT_MAX,
} page_scroller_sort_type_e;

extern Evas_Object *page_scroller_create(Evas_Object *tab, Evas_Object *index, page_scroller_sort_type_e sort_type, int rotate);
extern void page_scroller_destroy(Evas_Object *scroller);
extern void page_scroller_clean(Evas_Object *scroller);

extern Evas_Object *page_scroller_get_page_at(Evas_Object *scroller, unsigned int idx);
extern unsigned int page_scroller_count_page(Evas_Object *scroller);
extern int page_scroller_get_page_no(Evas_Object* scroller, Evas_Object *page);

extern Evas_Object *page_scroller_push_item(Evas_Object *scroller, app_info_t *ai);
extern int page_scroller_get_current_page_no(Evas_Object *scroller);
extern Evas_Object *page_scroller_find_item_by_package(Evas_Object *scroller, const char *package, int *page_no);
extern void page_scroller_trim_items(Evas_Object *scroller);
extern void page_scroller_bring_in(Evas_Object *scroller, int idx);
extern void page_scroller_show_region(Evas_Object *scroller, int idx);

extern void page_scroller_edit(Evas_Object *scroller);
extern void page_scroller_unedit(Evas_Object *scroller);
extern bool page_scroller_is_edited(Evas_Object *scroller);

extern void page_scroller_focus(Evas_Object *scroller);
extern void page_scroller_focus_into_vector(Evas_Object *scroller, int vector);

#endif //__MENU_SCREEN_PAGE_SCROLLER_H__

// End of a file
