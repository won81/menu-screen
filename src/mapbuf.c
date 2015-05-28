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
#include "layout.h"
#include "mapbuf.h"
#include "util.h"



HAPI Evas_Object *mapbuf_get_mapbuf(Evas_Object *obj)
{
	Evas_Object *mapbuf;

	if (obj == NULL) return NULL;

	mapbuf = evas_object_data_get(obj, "mapbuf");
	if (!mapbuf && evas_object_data_get(obj, "page")) {
		mapbuf = obj;
	}

	return mapbuf;
}



HAPI Evas_Object *mapbuf_get_page(Evas_Object *obj)
{
	Evas_Object *page;

	if (obj == NULL) return NULL;

	page = evas_object_data_get(obj, "page");
	if (!page && evas_object_data_get(obj, "mapbuf")) {
		page = obj;
	}

	return page;
}



HAPI menu_screen_error_e mapbuf_enable(Evas_Object *obj, int force)
{
	Evas_Object *mapbuf;
	int cnt;

	mapbuf = mapbuf_get_mapbuf(obj);
	if (!mapbuf) {
		_D("Failed to get the mapbuf object");
		return MENU_SCREEN_ERROR_FAIL;
	}

	if (force) {
		evas_object_data_set(mapbuf, "mapbuf_enabled", (void*)0);
		//elm_mapbuf_enabled_set(mapbuf, 1); // Mapbuf has been disabled because of a mapbuf bug.
		elm_mapbuf_enabled_set(mapbuf, 0);
		return MENU_SCREEN_ERROR_OK;
	}

	cnt = (int)evas_object_data_get(mapbuf, "mapbuf_enabled");
	cnt ++;
	evas_object_data_set(mapbuf, "mapbuf_enabled", (void*)cnt);
	//_D("[%s] CNT = %d", __func__, cnt);

	if (cnt == 0) {
		if (!elm_mapbuf_enabled_get(mapbuf)) {
			//elm_mapbuf_enabled_set(mapbuf, 1); // Mapbuf has been disabled because of a mapbuf bug.
			elm_mapbuf_enabled_set(mapbuf, 0);
			//_D("[%s] mapbuf enabled", __func__);
		}
	}

	return MENU_SCREEN_ERROR_OK;
}



HAPI int mapbuf_is_enabled(Evas_Object *obj)
{
	Evas_Object *mapbuf;
	mapbuf = mapbuf_get_mapbuf(obj);
	if (!mapbuf) {
		return 0;
	}

	return elm_mapbuf_enabled_get(mapbuf);
}



HAPI int mapbuf_disable(Evas_Object *obj, int force)
{
	Evas_Object *mapbuf;
	int cnt;

	mapbuf = mapbuf_get_mapbuf(obj);
	if (!mapbuf) {
		_D("Failed to get the mapbuf object");
		return MENU_SCREEN_ERROR_FAIL;
	}

	if (force) {
		evas_object_data_set(mapbuf, "mapbuf_enabled", (void*)-1);
		elm_mapbuf_enabled_set(mapbuf, 0);
		return MENU_SCREEN_ERROR_OK;
	}

	cnt = (int)evas_object_data_get(mapbuf, "mapbuf_enabled");
	if (cnt == 0) {
		if (elm_mapbuf_enabled_get(mapbuf)) {
			elm_mapbuf_enabled_set(mapbuf, 0);
			//_D("[%s] disableld mapbuf", __func__);
		}
	}

	cnt --;
	evas_object_data_set(mapbuf, "mapbuf_enabled", (void*)cnt);
	//_D("[%s] CNT = %d", __func__, cnt);

	return MENU_SCREEN_ERROR_OK;
}



HAPI Evas_Object *mapbuf_bind(Evas_Object *box, Evas_Object *page)
{
	Evas_Object *mapbuf;

	mapbuf = elm_mapbuf_add(box);
	if (!mapbuf) {
		_D("Failed to create a new mapbuf");
		return NULL;
	}

	elm_mapbuf_smooth_set(mapbuf, EINA_TRUE);
	elm_mapbuf_alpha_set(mapbuf, EINA_TRUE);
	elm_object_content_set(mapbuf, page);

	evas_object_data_set(page, "mapbuf", mapbuf);
	evas_object_data_set(mapbuf, "page", page);
	mapbuf_disable(mapbuf, 1);
	evas_object_show(mapbuf);

	return mapbuf;
}



HAPI Evas_Object *mapbuf_unbind(Evas_Object *obj)
{
	Evas_Object *page;
	Evas_Object *mapbuf;

	page = evas_object_data_get(obj, "page");
	if (page) {
		mapbuf = obj;
	} else {
		page = obj;
		mapbuf = evas_object_data_get(obj, "mapbuf");
	}

	if (mapbuf) {
		elm_mapbuf_enabled_set(mapbuf, 0);
		evas_object_data_del(page, "mapbuf");
		evas_object_data_del(mapbuf, "page");
		evas_object_data_del(mapbuf, "mapbuf_enabled");
		page = elm_object_content_unset(mapbuf);
		evas_object_del(mapbuf);
	}
	return page;
}



// End of a file
