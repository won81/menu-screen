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
#include <vconf.h>

#include "item_badge.h"
#include "conf.h"
#include "index.h"
#include "item.h"
#include "item_event.h"
#include "layout.h"
#include "list.h"
#include "mapbuf.h"
#include "menu_screen.h"
#include "page.h"
#include "page_scroller.h"
#include "pkgmgr.h"
#include "util.h"

#define BUFSZE 1024
#define PAGE_SCROLL_SENSITIVE 0.2
#define PROP_PORTRAIT_HEIGHT (PAGE_PORTRAIT_HEIGHT / BASE_HEIGHT)



HAPI void page_scroller_bring_in(Evas_Object *scroller, int idx)
{
	Evas_Object *index;
	int w, h;

	_D("BRING IN TO %d", idx);

	evas_object_data_set(scroller, "current_idx", (void *) idx);

	evas_object_geometry_get(scroller, NULL, NULL, &w, &h);
	elm_scroller_region_bring_in(scroller, idx * w, 0, w, h);

	index = evas_object_data_get(scroller, "index");
	if (!index) {
		_E("cannot find index.");
	}
	_D("page index bring in to %d", idx);
	index_bring_in(index, idx);
}



HAPI void page_scroller_show_region(Evas_Object *scroller, int idx)
{
	Evas_Object *index;
	int w, h;

	evas_object_geometry_get(scroller, NULL, NULL, &w, &h);
	elm_scroller_region_show(scroller, idx * w, 0, w, h);

	index = evas_object_data_get(scroller, "index");
	if (!index) {
		_E("cannot find index.");
	}
	_D("page index bring in to %d", idx);
	index_bring_in(index, idx);
}



static void _anim_stop_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("stop the scroller(%p) animation", scroller);

	/* page_scroller_focus & index_bring_in in _drag_stop_cb & _anim_stop_cb */
	do {
		Evas_Coord x, y, w, h;
		elm_scroller_region_get(scroller, &x, &y, &w, &h);

		if (x % w) return;

		int cur_idx = page_scroller_get_current_page_no(scroller);
		int idx = 0;
		if (w) idx = x / w;
		if (cur_idx == idx) return;

		page_scroller_bring_in(scroller, idx);
		page_scroller_focus(scroller);
	} while (0);

}



static void _anim_start_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("start the scroller(%p) animation", scroller);

	int drag_start = (int) evas_object_data_get(scroller, "drag_start");
	if (drag_start == 0) return;
	evas_object_data_set(scroller, "drag_start", (void *) 0);
}






static void _drag_start_cb(void *data, Evas_Object *scroller, void *event_info)
{
	int previous_x;

	_D("Invoked");

	elm_scroller_region_get(scroller, &previous_x, NULL, NULL, NULL);
	evas_object_data_set(scroller, "previous_x", (void *) previous_x);
	evas_object_data_set(scroller, "drag_start", (void *) 1);
}



static void _drag_stop_cb(void *data, Evas_Object *scroller, void *event_info)
{
	Evas_Coord x, y, w, h;
	int previous_x;

	_D("Invoked");

	elm_scroller_region_get(scroller, &x, &y, &w, &h);
	previous_x = (int) evas_object_data_get(scroller, "previous_x");
	if (x == previous_x) {
		_D("[33mHold scroller (previous) %d (current) %d[0m", previous_x, x);
	}

	/* page_scroller_focus & index_bring_in in _drag_stop_cb & _anim_stop_cb */
	do {
		if (x % w) return;

		int cur_idx = page_scroller_get_current_page_no(scroller);
		int idx = 0;
		if (w) idx = x / w;

		if (cur_idx == idx) return;

		page_scroller_bring_in(scroller, idx);
		page_scroller_focus(scroller);
	} while (0);
}



static void _scroller_register(Evas_Object *scroller)
{
	evas_object_smart_callback_add(scroller, "scroll,anim,start", _anim_start_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,anim,stop", _anim_stop_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,drag,start", _drag_start_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,drag,stop", _drag_stop_cb, NULL);
}



static void _scroller_unregister(Evas_Object *scroller)
{
	evas_object_smart_callback_del(scroller, "scroll,anim,start", _anim_start_cb);
	evas_object_smart_callback_del(scroller, "scroll,anim,stop", _anim_stop_cb);
	evas_object_smart_callback_del(scroller, "scroll,drag,start", _drag_start_cb);
	evas_object_smart_callback_del(scroller, "scroll,drag,stop", _drag_stop_cb);
}



static menu_screen_error_e _find_position_by_default(Evas_Object *scroller, int *candidate_page, int *candidate_pos, void *data)
{
	Evas_Object *page;
	Evas_Object *item;
	register unsigned int page_no;
	register unsigned int position_no;
	unsigned int nr_of_pages;
	int page_max_app;

	retv_if(NULL == scroller, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == candidate_page, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == candidate_pos, MENU_SCREEN_ERROR_INVALID_PARAMETER);

	*candidate_page = 0;
	*candidate_pos = 0;
	nr_of_pages = page_scroller_count_page(scroller);
	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");
	for (page_no = 0; page_no < nr_of_pages; page_no ++) {
		page = page_scroller_get_page_at(scroller, page_no);
		if (!page) {
			_D("Page is not found at %d", page_no);
			return MENU_SCREEN_ERROR_FAIL;
		}

		for (position_no = 0; position_no < page_max_app; position_no ++) {
			item = page_get_item_at(page, position_no);
			if (!item) {
				*candidate_page = page_no;
				*candidate_pos = position_no;
				return MENU_SCREEN_ERROR_OK;
			}
		}
	}

	*candidate_page = page_no;
	*candidate_pos = 0;

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _find_position_by_package(Evas_Object *scroller, int *candidate_page, int *candidate_pos, void *data)
{
	Evas_Object *page;
	Evas_Object *item;
	register int page_no;
	register int position_no;
	unsigned int nr_of_pages;
	int page_max_app;
	app_info_t *ai = data;

	retv_if(NULL == scroller, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == candidate_page, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == candidate_pos, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == data, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == ai->package, MENU_SCREEN_ERROR_INVALID_PARAMETER);

	*candidate_page = 0;
	*candidate_pos = 0;
	nr_of_pages = page_scroller_count_page(scroller);
	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");
	for (page_no = 0; page_no < nr_of_pages; page_no ++) {
		page = page_scroller_get_page_at(scroller, page_no);
		if (!page) {
			_D("Page is not found at %d", page_no);
			return MENU_SCREEN_ERROR_FAIL;
		}

		for (position_no = 0; position_no < page_max_app; position_no ++) {
			char *package;

			item = page_get_item_at(page, position_no);
			if (!item) {
				*candidate_page = page_no;
				*candidate_pos = position_no;
				return MENU_SCREEN_ERROR_OK;
			} else if ((package = item_get_package(item)) && strcmp(package, ai->package) > 0) {
				*candidate_page = page_no;
				*candidate_pos = position_no;
				return MENU_SCREEN_ERROR_OK;
			}
		}
	}

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _find_position_by_name(Evas_Object *scroller, int *candidate_page, int *candidate_pos, void *data)
{
	Evas_Object *page;
	Evas_Object *item;
	register int page_no;
	register int position_no;
	unsigned int nr_of_pages;
	int page_max_app;
	app_info_t *ai = data;

	retv_if(NULL == scroller, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == candidate_page, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == candidate_pos, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == data, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == ai->name, MENU_SCREEN_ERROR_INVALID_PARAMETER);

	*candidate_page = 0;
	*candidate_pos = 0;
	nr_of_pages = page_scroller_count_page(scroller);
	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");
	for (page_no = 0; page_no < nr_of_pages; page_no ++) {
		page = page_scroller_get_page_at(scroller, page_no);
		if (!page) {
			_D("Page is not found at %d", page_no);
			return MENU_SCREEN_ERROR_FAIL;
		}

		for (position_no = 0; position_no < page_max_app; position_no ++) {
			char *name;

			item = page_get_item_at(page, position_no);
			if (!item) {
				*candidate_page = page_no;
				*candidate_pos = position_no;
				return MENU_SCREEN_ERROR_OK;
			} else if ((name = item_get_name(item)) && strcmp(name, ai->name) > 0) {
				*candidate_page = page_no;
				*candidate_pos = position_no;
				return MENU_SCREEN_ERROR_OK;
			}
		}
	}

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _animated_pack_item(Evas_Object *item, Evas_Object *scroller, Evas_Object *page, int from)
{
	Evas_Object *item_out_page = NULL;
	char buf[32];
	int to;
	int page_no;

	retv_if(NULL == item, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == scroller, MENU_SCREEN_ERROR_INVALID_PARAMETER);
	retv_if(NULL == page, MENU_SCREEN_ERROR_INVALID_PARAMETER);

	page_no = page_scroller_get_page_no(scroller, page);
	do {
		to = page_find_empty_near(page, from);
		if (to < 0) {
			int page_max_app;
			page_max_app = (int) evas_object_data_get(page, "page_max_app");
			to = page_max_app - 1;
			item_out_page = page_unpack_item_at(page, to);
		}

		for (to --; to >= from; to --) {
			Evas_Object *item_in_page;
			item_in_page = page_unpack_item_at(page, to);
			page_pack_item(page, to + 1, item_in_page);
			snprintf(buf, 32, "menu%d", to + 1);
			edje_object_signal_emit(_EDJ(page), STR_MOVE_PREV, buf);
			edje_object_signal_emit(_EDJ(page), STR_ANI_RETURN, buf);
		}

		page_pack_item(page, from, item);

		if (!item_out_page) break;

		page_no ++;
		page = page_scroller_get_page_at(scroller, page_no);
		if (!page) {
			int rotate;
			rotate = (int) evas_object_data_get(scroller, "rotate");
			page = page_create(scroller, page_no, rotate);
			retv_if(NULL == page, MENU_SCREEN_ERROR_FAIL);
			mapbuf_enable(page, 0);
		}

		from = 0;
		item = item_out_page;
		item_out_page = NULL;
	} while (page && item);

	return MENU_SCREEN_ERROR_OK;
}



static Evas_Object *_animated_unpack_item(Evas_Object *scroller, Evas_Object *page, unsigned int pos)
{
	Evas_Object *out = NULL;
	Evas_Object *item;
	Evas_Object *next_page;

	char buf[32];
	unsigned int page_max_app;
	unsigned int page_no;
	page_scroller_sort_type_e sort_type;

	out = page_unpack_item_at(page, pos);
	retv_if(NULL == out, NULL);

	page_no = page_scroller_get_page_no(scroller, page);
	page_max_app = (unsigned int) evas_object_data_get(scroller, "page_max_app");
	sort_type = (page_scroller_sort_type_e) evas_object_data_get(scroller, "sort_type");

	pos ++;
	while (page && page_no < MAX_PAGE_NO) {
		if (0 == page_count_item(page)) {
			page_destroy(scroller, page);
			break;
		}

		for (; pos < page_max_app; pos ++) {
			item = page_unpack_item_at(page, pos);
			if (NULL == item) continue;

			page_pack_item(page, pos - 1, item);
			snprintf(buf, 32, "menu%d", pos - 1);
			edje_object_signal_emit(_EDJ(page), STR_MOVE_NEXT, buf);
			edje_object_signal_emit(_EDJ(page), STR_ANI_RETURN, buf);
		}

		if (sort_type == PAGE_SCROLLER_SORT_MAX) {
			return NULL;
		}

		page_no ++;
		next_page = page_scroller_get_page_at(scroller, page_no);
		if (next_page) {
			item = page_unpack_item_at(next_page, 0);
			if (NULL == item) continue;

			page_pack_item(page, page_max_app - 1, item);
		} else break;

		pos = 1;
		page = next_page;
	}

	return out;
}



HAPI Evas_Object *page_scroller_push_item(Evas_Object *scroller, app_info_t *ai)
{
	Evas_Object *page;
	Evas_Object *item;
	unsigned int nr_of_pages;
	int candidate_page = -1;
	int candidate_pos = 0;
	int sort_type;
	register int i;

	struct {
		page_scroller_sort_type_e sort_type;
		menu_screen_error_e (*sort_func)(Evas_Object *scroller, int *candidate_page, int *candidate_pos, void *data);
	} sort_type_map[] = {
		{
			.sort_type = PAGE_SCROLLER_SORT_BY_DEFAULT,
			.sort_func = _find_position_by_default,
		},
		{
			.sort_type = PAGE_SCROLLER_SORT_BY_PACKAGE,
			.sort_func = _find_position_by_package,
		},
		{
			.sort_type = PAGE_SCROLLER_SORT_BY_NAME,
			.sort_func = _find_position_by_name,
		},
		{
			.sort_type = PAGE_SCROLLER_SORT_MAX,
			.sort_func = NULL,
		},
	};

	sort_type = (int) evas_object_data_get(scroller, "sort_type");
	sort_type_map[sort_type].sort_func(scroller, &candidate_page, &candidate_pos, ai);

	nr_of_pages = page_scroller_count_page(scroller);

	for (i = nr_of_pages; i <= candidate_page; i ++) {
		Evas_Object *new_page;
		int rotate;

		rotate = (int) evas_object_data_get(scroller, "rotate");
		new_page = page_create(scroller, nr_of_pages, rotate);
		retv_if(NULL == new_page, NULL);
		mapbuf_enable(new_page, 0);
	}

	item = item_create(scroller, ai);
	retv_if(NULL == item, NULL);

	page = page_scroller_get_page_at(scroller, candidate_page);
	if (!page) {
		_D("Impossible, page is not found");
		item_destroy(item);
		return NULL;
	}

	retv_if(MENU_SCREEN_ERROR_OK !=
			_animated_pack_item(item, scroller, page, candidate_pos),
			NULL);

	return item;
}



static inline menu_screen_error_e _create_cb(const char *package, void *data)
{
	app_info_t ai = {0,};
	Evas_Object *item;
	Evas_Object *scroller = data;

	if (MENU_SCREEN_ERROR_FAIL == list_get_values(package, &ai)) {
		list_free_values(&ai);
		return MENU_SCREEN_ERROR_FAIL;
	}

	do {
		if (!scroller) {
			_D("Scroller is NULL.");
			break;
		}

		if (ai.nodisplay || !ai.enabled)
		{
			Evas_Object *page;

			_D("package %s is not visible", package);
			item = pkgmgr_find_pended_object(ai.package, 1, scroller, &page);
			if (item) {
				if (page) {
					page_unpack_item(page, item);
					page_scroller_trim_items(scroller);
				}

				item_destroy(item);
			}

			break;
		}

		item = pkgmgr_find_pended_object(ai.package, 1, scroller, NULL);
		if (!item) {
			item = page_scroller_find_item_by_package(scroller, ai.package, NULL);
			if (!item) {
				Evas_Object *item;
				_D("package %s is installed directly", package);
				item = page_scroller_push_item(scroller, &ai);
				if (item) {
					break;
				} else {
					list_free_values(&ai);
					retv_if(1, MENU_SCREEN_ERROR_FAIL);
				}
			}
		}

		_D("Package %s is found, update it!", package);
		item_update(item, &ai);
	} while(0);

	list_free_values(&ai);
	return MENU_SCREEN_ERROR_OK;
}



static inline menu_screen_error_e _update_cb(const char *package, void *data)
{
	Evas_Object *scroller = data;
	Evas_Object *item;
	int page_no = 0;
	app_info_t ai = {0,};

	if (MENU_SCREEN_ERROR_FAIL == list_get_values(package, &ai)) {
		list_free_values(&ai);
		return MENU_SCREEN_ERROR_FAIL;
	}

	do {
		if (!scroller) {
			_D("Scroller is NULL.");
			break;
		}

		item = page_scroller_find_item_by_package(scroller, package, &page_no);
		if (!item) {
			Evas_Object *page;
			_D("There is no loaded item is found");
			item = pkgmgr_find_pended_object(package, 1, scroller, &page);
			if (item) {
				if (!ai.nodisplay && ai.enabled) {
					_D("Item is found for updating from the install list, Ignore this.");
					item_update(item, &ai);
				} else {
					_D("Item is found for updating from the install list, But nodisplay");
					if (page) {
						page_unpack_item(page, item);
						page_scroller_trim_items(scroller);
					}
					item_destroy(item);
				}
			} else {
				Evas_Object *item;
				_D("Item is not found. Create a new one");
				item = ((!ai.nodisplay && ai.enabled) ? page_scroller_push_item(scroller, &ai) : NULL);
				if (item) {
					break;
				} else {
					list_free_values(&ai);
					retv_if(1, MENU_SCREEN_ERROR_FAIL);
				}
			}
		} else {
			Evas_Object *page;
			Evas_Object *pended_item;

			pended_item = pkgmgr_find_pended_object(package, 1, scroller, &page);
			if (!pended_item ) {
				_D("Cannot find package in the install list");
			}

			if (!ai.nodisplay && ai.enabled) {
				item_update(item, &ai);
				break;
			}

			page = page_scroller_get_page_at(scroller, page_no);
			if (page) {
				page_unpack_item(page, item);
				page_scroller_trim_items(scroller);
			}
			item_destroy(item);
		}
	} while (0);

	list_free_values(&ai);
	return MENU_SCREEN_ERROR_OK;
}



static inline menu_screen_error_e _delete_cb(const char *package, void *data)
{
	Evas_Object *item;
	Evas_Object *page;
	Evas_Object *scroller = data;
	Evas_Object *tmp;
	int page_no = 0;
	register unsigned int i;
	unsigned int page_max_app;

	do {
		if (!scroller) {
			_D("Scroller is NULL.");
			break;
		}

		tmp = pkgmgr_find_pended_object(package, 1, scroller, NULL);

		item = page_scroller_find_item_by_package(scroller, package, &page_no);
		retv_if(NULL == item, MENU_SCREEN_ERROR_FAIL);

		if (tmp != item) {
			_D("Pended item is %p, Found item is %p", tmp, item);
		}

		page = page_scroller_get_page_at(scroller, page_no);
		retv_if(NULL == page, MENU_SCREEN_ERROR_FAIL);

		page_max_app = (unsigned int) evas_object_data_get(scroller, "page_max_app");
		for (i = 0; i < page_max_app; i++) {
			if (item == page_get_item_at(page, i)) {
				break;
			}
		}

		item = _animated_unpack_item(scroller, page, i);
		retv_if(NULL == item, MENU_SCREEN_ERROR_FAIL);
		item_destroy(item);
	} while (0);

	return MENU_SCREEN_ERROR_OK;
}



static struct pkg_event_handler {
	const char *event_name;
	int (*event_handler)(const char *package, void *data);
} event_table[] = {
	{
		.event_name = "create",
		.event_handler = _create_cb,
	},
	{
		.event_name = "update",
		.event_handler = _update_cb,
	},
	{
		.event_name = "delete",
		.event_handler = _delete_cb,
	},
	{
		.event_name = NULL,
		.event_handler = NULL,
	},
};



static void _desktop_cb(keynode_t *node, void *data)
{
	char *event;
	char type[10];
	char package[BUFSZE];
	register int i;

	event = vconf_get_str(vconf_keynode_get_name(node));
	ret_if(NULL == event);

	if (sscanf(event, "%10[^:]:%1023s", type, package) != 2) {
		_D("Failed to parse the event format : [%s], [%s]", type, package);
	}

	_D("command[%s], package[%s]", type, package);

	for (i = 0; event_table[i].event_name; i ++) {
		if (!strcasecmp(type, event_table[i].event_name)) {
			if (event_table[i].event_handler(package, data) == MENU_SCREEN_ERROR_FAIL) {
				_E("Failed to handles the desktop notification event");
			}

			free(event);
			return;
		}
	}

	_E("Failed to find a proper event handler");
	free(event);
}



static void _mapbuf_cb(keynode_t *node, void *data)
{
	int value;
	int nr_of_pages;
	register int i;
	Evas_Object *page;

	if (vconf_get_int("memory/menuscreen/mapbuf", &value) < 0) {
		_D("Failed to get mapbuf status");
		return;
	}

	nr_of_pages = page_scroller_count_page(data);
	if (value) {
		for (i = 0; i < nr_of_pages; i ++) {
			page = page_scroller_get_page_at(data, i);
			if (!page) continue;

			if (!mapbuf_is_enabled(page)) {
				_D("Enable mapbuf %d", i);
				mapbuf_enable(page, 1);
			}
		}
	} else {
		for (i = 0; i < nr_of_pages; i ++) {
			page = page_scroller_get_page_at(data, i);
			if (!page) continue;

			if (mapbuf_is_enabled(page)) {
				_D("Disable mapbuf %d", i);
				mapbuf_disable(page, 1);
			}
		}
	}
}



static void _mouse_wheel_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Wheel *ei = event_info;
	Evas_Object *scroller = data;
	int x, y, w, h;
	int idx = -1;

	_D("Wheel's up or down(%d)", ei->z);

	elm_scroller_region_get(scroller, &x, &y, &w, &h);
	if (ei->z > 0) { // Wheel's up
		idx = x / w;
		idx ++;
	} else if (ei->z < 0) { // Wheel's down
		idx = x / w; // Scroller got ECORE events at first, then Menu-screen gets EVAS events.
	} else { // Wheel's not moving.
		_D("Wheel's not moving");
	}

	if (idx >= page_scroller_count_page(scroller) || idx < 0) return;
	page_scroller_bring_in(scroller, idx);
}



HAPI Evas_Object *page_scroller_create(Evas_Object *tab, Evas_Object *index, page_scroller_sort_type_e sort_type, int rotate)
{
	Evas_Object *box;
	Evas_Object *scroller;
	int width;
	int height;

	scroller = elm_scroller_add(tab);
	retv_if(NULL == scroller, NULL);

	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_FALSE);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);

	width = menu_screen_get_root_width();
	height = (int) ((double) PROP_PORTRAIT_HEIGHT * ((double) menu_screen_get_root_height()));
	elm_scroller_page_size_set(scroller, width, height);
	elm_scroller_page_scroll_limit_set(scroller, 1, 1);

	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_min_set(scroller, width, height);
	evas_object_size_hint_max_set(scroller, width, height);

	_scroller_register(scroller);

	box = elm_box_add(scroller);
	if (!box) {
		_D("Failed to create box");
		evas_object_del(scroller);
		return NULL;
	}
	elm_box_horizontal_set(box, 1);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_data_set(scroller, "win", evas_object_data_get(tab, "win"));
	evas_object_data_set(scroller, "layout", evas_object_data_get(tab, "layout"));
	evas_object_data_set(scroller, "controlbar", evas_object_data_get(tab, "controlbar"));
	evas_object_data_set(scroller, "tab", tab);
	evas_object_data_set(scroller, "page_edje", evas_object_data_get(tab, "page_edje"));
	evas_object_data_set(scroller, "page_max_app", evas_object_data_get(tab, "page_max_app"));
	evas_object_data_set(scroller, "item_edje", evas_object_data_get(tab, "item_edje"));
	evas_object_data_set(scroller, "item_width", evas_object_data_get(tab, "item_width"));
	evas_object_data_set(scroller, "item_height", evas_object_data_get(tab, "item_height"));
	evas_object_data_set(scroller, "item_enable_long_press", evas_object_data_get(tab, "item_enable_long_press"));
	evas_object_data_set(scroller, "item_text_dual_line", evas_object_data_get(tab, "item_text_dual_line"));
	evas_object_data_set(scroller, "enable_bg_image", evas_object_data_get(tab, "enable_bg_image"));
	evas_object_data_set(scroller, "box", box);
	evas_object_data_set(scroller, "drag_start", (void *) 0);
	evas_object_data_set(scroller, "previous_x", (void *) 0);
	evas_object_data_set(scroller, "index", index);
	evas_object_data_set(scroller, "sort_type", (void *) sort_type);
	evas_object_data_set(scroller, "install_list", (void *) 0);
	evas_object_data_set(scroller, "rotate", (void *) rotate);
	evas_object_data_set(scroller, "is_edited", (void *) false);
	elm_object_content_set(scroller, box);

	evas_object_event_callback_add(box, EVAS_CALLBACK_DEL, _evas_object_event_del_cb, "BOX");
	evas_object_event_callback_add(scroller, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb, scroller);

	evas_object_show(box);
	evas_object_show(scroller);

	if (vconf_notify_key_changed(VCONFKEY_AIL_INFO_STATE, _desktop_cb, scroller) < 0) {
		_E("Failed to register a desktop change event handler");
	}

	if (vconf_notify_key_changed("memory/menuscreen/mapbuf", _mapbuf_cb, scroller) < 0) {
		_E("Failed to register a vconf cb for %s", "memory/menuscreen/mapbuf");
	}

	// FIXME : This will be enabled after rebuilding the routine for appid <-> pkgid.
	//pkgmgr_init(scroller);
	item_badge_register_changed_cb(scroller);

	return scroller;
}



HAPI void page_scroller_destroy(Evas_Object *scroller)
{
	Evas_Object *box;
	Evas_Object *page;
	Evas_Object *tmp;

	const Eina_List *page_list;
	const Eina_List *l;
	const Eina_List *ln;

	ret_if(NULL == scroller);
	ret_if(NULL == (box = evas_object_data_get(scroller, "box")));
	ret_if(NULL == (page_list = elm_box_children_get(box)));

	item_badge_unregister_changed_cb();

	// FIXME : This will be enabled after rebuilding the routine for appid <-> pkgid.
	//pkgmgr_fini();

	EINA_LIST_FOREACH_SAFE(page_list, l, ln, page) {
		int count;

		if (!page) {
			_D("page list contains nil item");
			continue;
		}

		count = eina_list_count(page_list);
		_D("page_list count : %d", count);
		if (count < 1) {
			elm_box_unpack(box, page);
		}

		tmp = mapbuf_get_page(page);
		if (tmp) page = tmp;

		page_destroy(scroller, page);
	}

	box = elm_object_content_unset(scroller);
	evas_object_del(box);

	_scroller_unregister(scroller);
	evas_object_data_del(scroller, "win");
	evas_object_data_del(scroller, "layout");
	evas_object_data_del(scroller, "controlbar");
	evas_object_data_del(scroller, "tab");
	evas_object_data_del(scroller, "page_edje");
	evas_object_data_del(scroller, "page_max_app");
	evas_object_data_del(scroller, "item_edje");
	evas_object_data_del(scroller, "item_width");
	evas_object_data_del(scroller, "item_height");
	evas_object_data_del(scroller, "item_enable_long_press");
	evas_object_data_del(scroller, "item_text_dual_line");
	evas_object_data_del(scroller, "enable_bg_image");
	evas_object_data_del(scroller, "box");
	evas_object_data_del(scroller, "drag_start");
	evas_object_data_del(scroller, "previous_x");
	evas_object_data_del(scroller, "index");
	evas_object_data_del(scroller, "sort_type");
	evas_object_data_del(scroller, "rotate");
	evas_object_data_del(scroller, "is_edited");
	evas_object_data_del(scroller, "install_list");

	evas_object_del(scroller);
	evas_object_event_callback_del(box, EVAS_CALLBACK_DEL, _evas_object_event_del_cb);
	evas_object_event_callback_del(scroller, EVAS_CALLBACK_MOUSE_WHEEL, _mouse_wheel_cb);

	if (vconf_ignore_key_changed(VCONFKEY_AIL_INFO_STATE, _desktop_cb) < 0) {
		_E("Failed to ignore the desktop event");
	}

	if (vconf_ignore_key_changed("memory/menuscreen/mapbuf", _mapbuf_cb) < 0) {
		_E("Failed to remove vconf %s", "memory/menuscreen/mapbuf");
	}
}



HAPI void page_scroller_clean(Evas_Object *scroller)
{
	Evas_Object *page;
	Evas_Object *item;

	register unsigned int i;
	unsigned int page_max_app;
	unsigned int count;

	count = page_scroller_count_page(scroller);
	if (count == 0) return;

	for (i = 1; i < MAX_PAGE_NO; i++) {
		page = page_scroller_get_page_at(scroller, i);
		if (NULL == page) break;

		page_destroy(scroller, page);
	}

	page = page_scroller_get_page_at(scroller, 0);
	ret_if(NULL == page);

	page_max_app = (unsigned int) evas_object_data_get(scroller, "page_max_app");
	for (i = 0; i < page_max_app; i++) {
		item = page_get_item_at(page, i);
		if (item) {
			item_destroy(item);
		} else break;
	}
}



HAPI Evas_Object *page_scroller_get_page_at(Evas_Object *scroller, unsigned int idx)
{
	const Eina_List *page_list;
	Evas_Object *item;
	Evas_Object *box;

	retv_if(idx >= MAX_PAGE_NO, NULL);

	box = evas_object_data_get(scroller, "box");
	retv_if(NULL == box, NULL);

	page_list = elm_box_children_get(box);
	retv_if(NULL == page_list, NULL);

	item = eina_list_nth(page_list, idx);
	if (item) {
		item = mapbuf_get_page(item);
	}

	return item;
}



HAPI unsigned int page_scroller_count_page(Evas_Object *scroller)
{
	const Eina_List *page_list;
	Evas_Object *box;

	box = evas_object_data_get(scroller, "box");
	retv_if(NULL == box, 0);

	page_list = elm_box_children_get(box);
	retv_if(NULL == page_list, 0);

	return eina_list_count(page_list);
}



HAPI int page_scroller_get_page_no(Evas_Object* scroller, Evas_Object *page)
{
	Evas_Object *item;
	Evas_Object *box;
	register int idx = 0;
	const Eina_List *page_list;
	const Eina_List *pos;

	box = evas_object_data_get(scroller, "box");
	retv_if(NULL == box, 0);

	page_list = elm_box_children_get(box);
	EINA_LIST_FOREACH(page_list, pos, item) {
		if (!item) {
			_D("page list contains nil item");
			continue;
		}

		item = mapbuf_get_page(item);
		if (item == page) {
			return idx;
		}

		idx ++;
	}

	return -1;
}



HAPI int page_scroller_get_current_page_no(Evas_Object *scroller)
{
	return (int) evas_object_data_get(scroller, "current_idx");
}



HAPI Evas_Object *page_scroller_find_item_by_package(Evas_Object *scroller, const char *package, int *page_no)
{
	register int i;
	register int j;
	Evas_Object *page;
	Evas_Object *item;
	const char *tmp;
	int local_page_no;
	int page_max_app;

	if (!page_no) page_no = &local_page_no;

	retv_if(NULL == package, NULL);

	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");
	for (i = 0; i < page_scroller_count_page(scroller); i ++) {
		page = page_scroller_get_page_at(scroller, i);
		if (!page) continue;

		for (j = 0; j < page_max_app; j ++) {
			item = page_get_item_at(page, j);
			if (!item) continue;

			tmp = item_get_package(item);
			if (!tmp) {
				_D("Something goes wrong, this package has no name?");
				continue;
			}

			if (!strcmp(tmp, package)) {
				*page_no = i;
				return item;
			}
		}
	}

	return NULL;
}



HAPI void page_scroller_trim_items(Evas_Object *scroller)
{
	register unsigned int i;
	register unsigned int j;
	int page_max_app;
	int pos = 0;

	Evas_Object *page;
	Evas_Object *item;
	Eina_List *list = NULL;

	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");
	for (i = 0; i < MAX_PAGE_NO; i ++) {
		page = page_scroller_get_page_at(scroller, i);
		if (!page) break;

		for (j = 0; j < page_max_app; j++) {
			item = page_unpack_item_at(page, j);
			if (item) {
				list = eina_list_append(list, item);
				_D("LIST APPEND : %s", item_get_package(item));
			}
		}

	}

	for (i = 0; i < eina_list_count(list); i++) {
		if (i % page_max_app == 0) {
			page = page_scroller_get_page_at(scroller, i / page_max_app);
			_D("PAGE GET : 0x%x", page);
			if (NULL == page) {
				_E("Cannot get page");
				break;
			}
		}

		item = eina_list_nth(list, i);
		if (NULL == item) {
			_E("Cannot get item");
			break;
		}
		_D("LIST GET : [%d] %s", pos, item_get_package(item));

		page_pack_item(page, pos % page_max_app, item);
		pos++;
	}

	for (i = pos / page_max_app; i < MAX_PAGE_NO; i++) {
		int count;

		page = page_scroller_get_page_at(scroller, i);
		if (NULL == page) {
			break;
		}
		count = page_count_item(page);
		if (count == 0) {
			Evas_Coord w;
			Evas_Coord h;

			_D("PAGE IS EMPTY : 0x%x", page);

			page_destroy(scroller, page);
			evas_object_geometry_get(scroller, NULL, NULL, &w, &h);
			elm_scroller_region_show(scroller, 0, 0, w, h);
		}
	}
	pos --;
	eina_list_free(list);
}



HAPI void page_scroller_edit(Evas_Object *scroller)
{
	Evas_Object *page;
	Evas_Object *item;
	register unsigned int page_no;
	register unsigned int position_no;
	unsigned int nr_of_pages;
	int page_max_app;

	nr_of_pages = page_scroller_count_page(scroller);
	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");
	for (page_no = 0; page_no < nr_of_pages; page_no ++) {
		page = page_scroller_get_page_at(scroller, page_no);
		ret_if(NULL == page);

		for (position_no = 0; position_no < page_max_app; position_no ++) {
			item = page_get_item_at(page, position_no);
			if (!item) {
				continue;
			}

			item_edit(item);
		}
	}
	evas_object_data_set(scroller, "is_edited", (void *) true);
}



HAPI void page_scroller_unedit(Evas_Object *scroller)
{
	Evas_Object *all_apps;
	Evas_Object *page;
	Evas_Object *item;
	register int page_no;
	register unsigned int position_no;
	unsigned int nr_of_pages;
	int page_max_app;

	ret_if(NULL == scroller);

	all_apps = evas_object_data_get(scroller, "tab");
	ret_if(NULL == all_apps);

	nr_of_pages = page_scroller_count_page(scroller);
	page_max_app = (int) evas_object_data_get(scroller, "page_max_app");

	for (page_no = nr_of_pages - 1; page_no >= 0; page_no --) {
		int count;

		page = page_scroller_get_page_at(scroller, page_no);
		if (NULL == page) break;
		count = page_count_item(page);

		page_scroller_trim_items(scroller);

		for (position_no = 0; position_no < page_max_app; position_no ++) {
			item = page_get_item_at(page, position_no);
			if (!item) {
				break;
			}

			item_unedit(item);
		}
	}

	evas_object_data_set(scroller, "is_edited", (void *) false);
}



HAPI bool page_scroller_is_edited(Evas_Object *scroller)
{
	return (bool) evas_object_data_get(scroller, "is_edited");
}



HAPI void page_scroller_focus(Evas_Object *scroller)
{
	int idx = 0;
	idx = page_scroller_get_current_page_no(scroller);

	Evas_Object *page = NULL;
	page = page_scroller_get_page_at(scroller, (unsigned int) idx);
	ret_if(NULL == page);

	Evas_Object *item = NULL;
	item = page_get_item_at(page, 0);
	ret_if(NULL == item);

	Evas_Object *focus_button = NULL;
	focus_button = elm_object_part_content_get(item, "focus");
	ret_if(NULL == focus_button);

	_D("Focus set scroller(%p), page:%d, item:%s", scroller, idx, item_get_name(item));
	elm_object_focus_set(focus_button, EINA_TRUE);
}



HAPI void page_scroller_focus_into_vector(Evas_Object *scroller, int vector)
{
	int idx = 0;
	idx = page_scroller_get_current_page_no(scroller);
	idx += vector;

	ret_if(0 > idx);

	Evas_Object *page = NULL;
	page = page_scroller_get_page_at(scroller, (unsigned int) idx);
	ret_if(NULL == page);

	Evas_Object *item = NULL;
	item = page_get_item_at(page, 0);
	ret_if(NULL == item);

	Evas_Object *focus_button = NULL;
	focus_button = elm_object_part_content_get(item, "focus");
	ret_if(NULL == focus_button);

	_D("Focus set scroller(%p), page:%d, item:%s", scroller, idx, item_get_name(item));
	elm_object_focus_set(focus_button, EINA_TRUE);

	page_scroller_bring_in(scroller, idx);
}



// End of a file
