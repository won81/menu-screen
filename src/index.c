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



#include <Elementary.h>

#include "conf.h"
#include "util.h"
#include "page.h"
#include "layout.h"
#include "menu_screen.h"

#define PAGE_CONTROLLER_GROUP_NAME "index"



HAPI inline void index_bring_in(Evas_Object *index, int idx)
{
	Elm_Object_Item *idx_it;

	idx_it = elm_index_item_find(index, (void *) idx);
	ret_if(NULL == idx_it);
	elm_index_item_selected_set(idx_it, EINA_TRUE);
}



HAPI void index_destroy(Evas_Object *index)
{
	ret_if(NULL == index);

	evas_object_data_del(index, "win");
	evas_object_data_del(index, "layout");
	evas_object_data_del(index, "controlbar");
	evas_object_data_del(index, "tab");
	evas_object_data_del(index, "count");
	elm_index_item_clear(index);
	evas_object_del(index);
}



HAPI Evas_Object *index_create(Evas_Object *tab, unsigned int count,unsigned int current_idx)
{
	Evas_Object *index;
	Evas_Object *scroller;
	register int i;
	char *number;

	retv_if(NULL == tab, NULL);
	index = elm_index_add(tab);
	retv_if(NULL == index, NULL);

	elm_object_theme_set(index, menu_screen_get_theme());
	elm_object_style_set(index, "menuscreen/index");
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);

	number = malloc(((size_t) log10((double) MAX_PAGE_NO)) + 2);
	retv_if(NULL == number, NULL);

	for (i = 0; i < count; i++) {
		sprintf(number, "%d", i + 1);
		elm_index_item_append(index, number, NULL, (void *) i);
	}
	elm_index_level_go(index, 0);

	if (count > 0) {
		index_bring_in(index, current_idx);
	}

	elm_object_part_content_set(tab, PAGE_CONTROLLER_GROUP_NAME, index);

	// FIXME : This part will be removed when fixing Elementary API for index_update.
	scroller = evas_object_data_get(tab, "scroller");
	if (scroller) {
		evas_object_data_set(scroller, "index", index);
	}

	evas_object_data_set(index, "win", evas_object_data_get(tab, "win"));
	evas_object_data_set(index, "layout", evas_object_data_get(tab, "layout"));
	evas_object_data_set(index, "controlbar", evas_object_data_get(tab, "controlbar"));
	evas_object_data_set(index, "tab", tab);
	evas_object_data_set(index, "count", (void *) count);

	evas_object_data_set(tab, "index", index);
	_D("COUNT:%d", count);

	evas_object_show(index);
	free(number);

	return index;
}



HAPI Evas_Object *index_update(Evas_Object *layout, Evas_Object *index, unsigned int count)
{
	int idx;
	Elm_Object_Item *idx_it;
	idx_it = elm_index_selected_item_get(index, 0);
	idx = (int) elm_object_item_data_get(idx_it);

	if(count == idx)
	{
		idx--;
	}
	if(idx == -1)
	{
		idx = 0;
	}
	_D("Current index %d, Current count: %d", idx,count);
	index_destroy(index);
	return index_create(layout, count,idx);
}



HAPI void index_hide(Evas_Object *index)
{
	edje_object_signal_emit(_EDJ(index), "hide", "index");
}



HAPI void index_show(Evas_Object *index)
{
	edje_object_signal_emit(_EDJ(index), "show", "index");
}



// End of the file
