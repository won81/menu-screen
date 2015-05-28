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

#include "menu_screen.h"
#include "conf.h"
#include "index.h"
#include "item.h"
#include "item_event.h"
#include "layout.h"
#include "mapbuf.h"
#include "page.h"
#include "page_scroller.h"
#include "util.h"

#define PAGE_GROUP_NAME "menu_bg"
#define PROP_PORTRAIT_HEIGHT (PAGE_PORTRAIT_HEIGHT / BASE_HEIGHT)



HAPI inline void page_mark_dirty(Evas_Object *page)
{
	int value;
	value = (int) evas_object_data_get(page, "dirty");
	evas_object_data_set(page, "dirty", (void*)(value + 1));
}



HAPI inline void page_unmark_dirty(Evas_Object *page)
{
	int value;
	value = (int) evas_object_data_get(page, "dirty");
	if (value > 0) {
		evas_object_data_set(page, "dirty", (void*)(value - 1));
	}
}



HAPI inline void page_clean_dirty(Evas_Object *page)
{
	evas_object_data_set(page, "dirty", 0);
}



HAPI inline int page_is_dirty(Evas_Object *page)
{
	return (int) evas_object_data_get(page, "dirty");
}



static menu_screen_error_e _insert_page_at(Evas_Object *scroller, Evas_Object *page, int index)
{
	unsigned int nr_of_pages;
	Evas_Object *mapbuf;
	Evas_Object *box;

	retv_if(NULL == page, MENU_SCREEN_ERROR_FAIL);

	if (index < 0) {
		_D("Out of range");
		index = 0;
	}

	box = evas_object_data_get(scroller, "box");
	retv_if(NULL == box, MENU_SCREEN_ERROR_FAIL);

	nr_of_pages = page_scroller_count_page(scroller);
	if (index >= nr_of_pages) {
		_D("Out of range. index : %d, total : %d", index, nr_of_pages);
		mapbuf = evas_object_data_get(page, "mapbuf");
		if (mapbuf) {
			elm_box_pack_end(box, mapbuf);
		} else {
			elm_box_pack_end(box, page);
		}
	} else {
		Evas_Object *current;
		Evas_Object *current_mapbuf;
		const Eina_List *page_list;

		page_list = elm_box_children_get(box);
		retv_if(NULL == page_list, MENU_SCREEN_ERROR_FAIL);

		current = eina_list_nth(page_list, index);
		retv_if(NULL == current, MENU_SCREEN_ERROR_FAIL);

		current_mapbuf = mapbuf_get_mapbuf(current);
		mapbuf = mapbuf_get_mapbuf(page);

		if (current_mapbuf && mapbuf) {
			elm_box_pack_before(box, mapbuf, current_mapbuf);
		} else if (!current_mapbuf && !mapbuf) {
			elm_box_pack_before(box, page, current);
		} else {
			_D("Page has mapbuf, invalid");
		}
	}

	page_mark_dirty(page);

	return MENU_SCREEN_ERROR_OK;
}



static void _dim_down_cb(void *data, Evas_Object *obj, const char* emission, const char* source)
{
	_D("Dim down");
}



static void _dim_up_cb(void *data, Evas_Object *obj, const char* emission, const char* source)
{
	_D("Dim up");
}



HAPI Evas_Object *page_create(Evas_Object *scroller, int idx, int rotate)
{
	Evas_Object *page;
	Evas_Object *bg;
	Evas_Object *index;
	Evas_Object *tab;
	Evas_Object *mapbuf;
	Evas_Object *box;

	char *page_edje;
	bool enable_bg_image;

	unsigned int count;
	int page_height;
	int page_width;

	_D("Create a new page[%d]", idx);

	page_edje = evas_object_data_get(scroller, "page_edje");
	enable_bg_image = (bool) evas_object_data_get(scroller, "enable_bg_image");

	page = layout_load_edj(scroller, page_edje, PAGE_GROUP_NAME);
	retv_if(!page, NULL);

	edje_object_signal_callback_add(_EDJ(page), "dim,down", "menu", _dim_down_cb, NULL);
	edje_object_signal_callback_add(_EDJ(page), "dim,up", "menu", _dim_up_cb, NULL);

	box = evas_object_data_get(scroller, "box");
	mapbuf = mapbuf_bind(box, page);
	if (!mapbuf) {
		_D("Failed to bind the object with mapbuf");
	}

	bg = evas_object_rectangle_add(menu_screen_get_evas());
	if (!bg) {
		evas_object_del(page);
		return NULL;
	}
	evas_object_color_set(bg, 0, 0, 0, 0);

	page_width = menu_screen_get_root_width();
	page_height = (int) ((double) PROP_PORTRAIT_HEIGHT * ((double) menu_screen_get_root_height()));

	evas_object_size_hint_min_set(bg, page_width, page_height);
	evas_object_size_hint_max_set(bg, page_width, page_height);

	evas_object_size_hint_min_set(page, page_width, page_height);
	evas_object_size_hint_max_set(page, page_width, page_height);
	evas_object_resize(page, page_width, page_height);

	elm_object_part_content_set(page, "bg", bg);

	evas_object_data_set(page, "win", evas_object_data_get(scroller, "win"));
	evas_object_data_set(page, "layout", evas_object_data_get(scroller, "layout"));
	evas_object_data_set(page, "controlbar", evas_object_data_get(scroller, "controlbar"));
	evas_object_data_set(page, "tab", evas_object_data_get(scroller, "tab"));
	evas_object_data_set(page, "scroller", scroller);
	evas_object_data_set(page, "page_edje", page_edje);
	evas_object_data_set(page, "page_max_app", evas_object_data_get(scroller, "page_max_app"));
	evas_object_data_set(page, "bg", bg);
	evas_object_data_set(page, "pending,list", NULL);
	evas_object_data_set(page, "dirty", (void *) 0);

	if (_insert_page_at(scroller, page, idx) != MENU_SCREEN_ERROR_OK) {
		evas_object_del(bg);
		evas_object_del(page);
		return NULL;
	}

	index = evas_object_data_get(scroller, "index");
	if (index) {
		tab = evas_object_data_get(scroller, "tab");
		count = page_scroller_count_page(scroller);
		index = index_update(tab, index, count);
		evas_object_data_set(scroller, "index", index);
	}

	return page;
}



HAPI void page_destroy(Evas_Object *scroller, Evas_Object *page)
{
	Evas_Object *mapbuf;
	Evas_Object *bg;
	Evas_Object *item;
	Evas_Object *box;
	Evas_Object *index;
	Evas_Object *tab;
	Eina_List *pending_list;

	register unsigned int i;
	int page_max_app;
	unsigned int count;

	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");
	for (i = 0; i < page_max_app; i ++) {
		item = page_unpack_item_at(page, i);
		if (!item) continue;

		item_destroy(item);
	}

	pending_list = evas_object_data_get(page, "pending,list");
	eina_list_free(pending_list);

	bg = evas_object_data_get(page, "bg");
	evas_object_del(bg);

	box = evas_object_data_get(scroller, "box");
	ret_if(NULL == box);

	mapbuf = mapbuf_get_mapbuf(page);
	if (mapbuf) {
		elm_box_unpack(box, mapbuf);
		mapbuf_unbind(mapbuf);
	} else {
		elm_box_unpack(box, page);
	}

	index = evas_object_data_get(scroller, "index");
	tab = evas_object_data_get(scroller, "tab");
	if (index && tab) {
		count = page_scroller_count_page(scroller);
		if (count) {
			index = index_update(tab, index, count);
			evas_object_data_set(scroller, "index", index);
		}
		else {
			index_destroy(index);
			evas_object_data_set(scroller, "index", NULL);
		}
	}

	evas_object_data_del(page, "win");
	evas_object_data_del(page, "layout");
	evas_object_data_del(page, "controlbar");
	evas_object_data_del(page, "tab");
	evas_object_data_del(page, "scroller");
	evas_object_data_del(page, "page_edje");
	evas_object_data_del(page, "page_max_app");
	evas_object_data_del(page, "bg");
	evas_object_data_del(page, "pending,list");
	evas_object_data_del(page, "dirty");
	layout_unload_edj(page);
}



HAPI Evas_Object *page_get_item_at(Evas_Object *page, unsigned int idx)
{
	Eina_List *pending_list;
	Eina_List *n;
	Eina_List *t;

	Evas_Object *pending_item;

	char swallow_name[PATH_MAX];
	int pending_idx;

	pending_list = evas_object_data_get(page, "pending,list");
	EINA_LIST_FOREACH_SAFE(pending_list, n, t, pending_item) {
		pending_idx = (int) evas_object_data_get(pending_item, "pending,idx");
		if (pending_idx == idx) {
			return pending_item;
		}
	}

	snprintf(swallow_name, sizeof(swallow_name), "menu_swallow_%d", idx);

	return edje_object_part_swallow_get(_EDJ(page), swallow_name);
}



HAPI menu_screen_error_e page_unpack_item(Evas_Object *page, Evas_Object *item)
{
	char tmp[PATH_MAX];
	Evas_Object *check_item;

	Eina_List *pending_list;
	Eina_List *n;
	Eina_List *t;
	Evas_Object *pend_item;
	int pending_idx = -1;

	retv_if(NULL == item, MENU_SCREEN_ERROR_FAIL);

	pending_list = evas_object_data_get(page, "pending,list");
	EINA_LIST_FOREACH_SAFE(pending_list, n, t, pend_item) {
		if (pend_item == item) {
			pending_idx = (int)evas_object_data_get(pend_item, "pending,idx");
			pending_list = eina_list_remove(pending_list, pend_item);
			evas_object_data_set(page, "pending,list", pending_list);
		}
	}

	retv_if(pending_idx == -1, MENU_SCREEN_ERROR_FAIL);
	sprintf(tmp, "menu_swallow_%d", pending_idx);

	check_item = edje_object_part_swallow_get(_EDJ(page), tmp);
	retv_if(check_item != item, MENU_SCREEN_ERROR_FAIL);

	check_item = elm_object_part_content_unset(page, tmp);
	if (check_item) {
		elm_object_part_content_set(page, tmp, NULL);
		page_mark_dirty(page);
	}

	mapbuf_disable(page, 1);
	mapbuf_enable(page, 1);

	return MENU_SCREEN_ERROR_OK;
}



HAPI Evas_Object *page_unpack_item_at(Evas_Object *page, int idx)
{
	Eina_List *pending_list;
	Eina_List *n;
	Eina_List *t;

	Evas_Object *object;
	Evas_Object *pending_item;

	char tmp[PATH_MAX];
	int pending_idx;

	pending_list = evas_object_data_get(page, "pending,list");
	EINA_LIST_FOREACH_SAFE(pending_list, n, t, pending_item) {
		pending_idx = (int) evas_object_data_get(pending_item, "pending,idx");
		if (pending_idx == idx) {
			pending_list = eina_list_remove(pending_list, pending_item);
			evas_object_data_set(page, "pending,list", pending_list);
		}
	}

	object = page_get_item_at(page, idx);
	if (object) {
		Evas_Object *check_object;
		sprintf(tmp, "menu_swallow_%d", idx);

		check_object = elm_object_part_content_unset(page, tmp);
		if (check_object != object) {
			elm_object_part_content_set(page, tmp, NULL);
			_D("page_unpack_item_at - different object is picked up");
		}

		page_mark_dirty(page);
	}

	mapbuf_disable(page, 1);
	mapbuf_enable(page, 1);

	return object;
}



HAPI void page_pack_item(Evas_Object *page, int idx, Evas_Object *item)
{
	char tmp[PATH_MAX];
	Evas_Object *object;
	Eina_List *list;

	ret_if(NULL == page);
	ret_if(NULL == item);

	list = evas_object_data_get(page, "pending,list");
	if (NULL == eina_list_data_find(list, item)) {
		list = eina_list_append(list, item);
		evas_object_data_set(page, "pending,list", list);
	}
	evas_object_data_set(item, "pending,idx", (void*)idx);

	mapbuf_disable(page, 0);

	snprintf(tmp, sizeof(tmp), "menu_swallow_%d", idx);
	object = elm_object_part_content_unset(page, tmp);
	if (object) {
		elm_object_part_content_set(page, tmp, NULL);
		item_destroy(object);
	}

	item_set_page(item, page, 1);
	item_mark_dirty(item);
	elm_object_part_content_set(page, tmp, item);
	page_mark_dirty(page);

	mapbuf_disable(page, 1);
	mapbuf_enable(page, 1);
}



HAPI void page_set_item(Evas_Object *page, int idx, Evas_Object *item)
{
	Eina_List *list;

	ret_if(NULL == page);
	ret_if(NULL == item);

	list = evas_object_data_get(page, "pending,list");
	if (NULL == eina_list_data_find(list, item)) {
		list = eina_list_append(list, item);
		evas_object_data_set(page, "pending,list", list);
	}
	evas_object_data_set(item, "pending,idx", (void *) idx);

	item_set_page(item, page, 1);

	mapbuf_disable(page, 1);
	mapbuf_enable(page, 1);
}



HAPI inline unsigned int page_count_item(Evas_Object *page)
{
	register unsigned int i;
	unsigned int count = 0;
	int page_max_app;

	page_max_app = (int) evas_object_data_get(page, "page_max_app");
	for (i = 0; i < page_max_app; i++) {
		if (page_get_item_at(page, i)) {
			count ++;
		}
	}

	return count;
}



HAPI int page_find_empty_near(Evas_Object *page, int pivot)
{
	int pivot_saved = pivot;
	Evas_Object *obj;
	int page_max_app;

	retv_if(NULL == page, -1);

	obj = page_get_item_at(page, pivot);
	if (NULL == obj) return pivot;

	for (pivot --; pivot >= 0; pivot --) {
		obj = page_get_item_at(page, pivot);
		if (!obj) {
			break;
		}
	}

	if (pivot >= 0) {
		return (int) pivot;
	}

	page_max_app = (int) evas_object_data_get(page, "page_max_app");
	for (pivot = pivot_saved + 1; pivot < page_max_app; pivot ++) {
		obj = page_get_item_at(page, pivot);
		if (!obj) break;
	}

	if (pivot < page_max_app) {
		return pivot;
	}

	return -1;
}



HAPI int page_find_first_empty(Evas_Object *page, int pivot)
{
	Evas_Object *item;
	Evas_Object *scroller;
	int idx;
	int page_max_app;

	scroller = evas_object_data_get(page, "tab");
	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");

	for (idx = pivot; idx < page_max_app; idx ++) {
		item = page_get_item_at(page, idx);
		if (!item) {
			break;
		}
	}

	return idx;
}



HAPI void page_trim_items(Evas_Object *page)
{
	Evas_Object *item;
	register unsigned int i;
	char buf[32];
	int to = -1;
	int page_max_app;

	page_max_app = (int) evas_object_data_get(page, "page_max_app");
	for (i = 0; i < page_max_app; i ++) {
		item = page_get_item_at(page, i);
		if (!item) {
			if (to < 0) {
				to = i;
			}
			continue;
		}

		if (to >= 0) {
			item = page_unpack_item_at(page, i);
			page_pack_item(page, to, item);
			snprintf(buf, 32, "menu%d", to);
			edje_object_signal_emit(_EDJ(page), STR_MOVE_NEXT, buf);
			edje_object_signal_emit(_EDJ(page), STR_ANI_RETURN, buf);
			to ++;
		}
	}
}




// End of a file
