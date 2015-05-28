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
#include <Ecore_X.h>
#include <utilX.h>
#include <vconf.h>

#include "menu_screen.h"
#include "conf.h"
#include "item_event.h"
#include "layout.h"
#include "mapbuf.h"
#include "page.h"
#include "item.h"
#include "util.h"
#include "all_apps/layout.h"



HAPI Evas_Object *layout_create(Evas_Object *conformant, const char *file, const char *group, int rotate)
{
	Evas_Object *layout;

	do {
		int width;
		int height;

		layout = layout_load_edj(conformant, file, group);
		retv_if(NULL == layout, NULL);

		width = menu_screen_get_root_width();
		height = menu_screen_get_root_height() - INDEX_HEIGHT;

		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_min_set(layout, width, height);
		evas_object_size_hint_max_set(layout, width, height);
		evas_object_resize(layout, width, height);
		evas_object_show(layout);

		evas_object_data_set(layout, "win", menu_screen_get_win());
		evas_object_data_set(layout, "rotate", (void *) rotate);
		evas_object_data_set(layout, "width", (void *) width);
		evas_object_data_set(layout, "height", (void *) height);
	} while (0);

	do {
		Evas_Object *all_apps;

		all_apps = all_apps_layout_create(layout, rotate);
		if (NULL == all_apps) {
			_E("Failed to create scroller");
			layout_destroy(layout);
			return NULL;
		}
		evas_object_data_set(layout, "all_apps", all_apps);
		elm_object_part_content_set(layout, "content", all_apps);
	} while (0);

	return layout;
}



HAPI void layout_destroy(Evas_Object *layout)
{
	Evas_Object *all_apps;

	all_apps = evas_object_data_del(layout, "all_apps");
	all_apps_layout_destroy(all_apps);

	evas_object_data_del(layout, "win");
	evas_object_data_del(layout, "rotate");
	evas_object_data_del(layout, "width");
	evas_object_data_del(layout, "height");

	layout_unload_edj(layout);
}



HAPI void layout_enable_block(Evas_Object *layout)
{
	ret_if(NULL == layout);

	_D("Enable layout blocker");
	edje_object_signal_emit(_EDJ(layout), "block", "layout");
}



HAPI void layout_disable_block(Evas_Object *layout)
{
	ret_if(NULL == layout);

	_D("Disable layout blocker");
	edje_object_signal_emit(_EDJ(layout), "unblock", "layout");
}



HAPI Evas_Object* layout_load_edj(Evas_Object *parent, const char *edjname, const char *grpname)
{
	Evas_Object *eo;

	retv_if(NULL == parent, NULL);

	eo = elm_layout_add(parent);
	retv_if(NULL == eo, NULL);
	retv_if(EINA_FALSE == elm_layout_file_set(eo, edjname, grpname), NULL);

	evas_object_data_set(_EDJ(eo), "evas_object", eo);
	evas_object_show(eo);

	return eo;
}



HAPI void layout_unload_edj(Evas_Object *layout)
{
	Evas_Object *evas_object;

	ret_if(NULL == layout);

	evas_object = evas_object_data_get(_EDJ(layout), "evas_object");
	if (evas_object) {
		evas_object_data_del(_EDJ(layout), "evas_object");
	}

	evas_object_del(layout);
}



// End of file
