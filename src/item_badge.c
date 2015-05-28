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



#include <badge.h>
#include <Elementary.h>
#include <stdbool.h>

#include "menu_screen.h"
#include "conf.h"
#include "item.h"
#include "page_scroller.h"
#include "util.h"



HAPI int item_badge_count(char *package)
{
	unsigned int is_display = 0;
	unsigned int count = 0;
	badge_error_e err = BADGE_ERROR_NONE;

	err = badge_get_display(package, &is_display);
	if (BADGE_ERROR_NONE != err) _D("cannot get badge display");

	if (0 == is_display) return 0;

	err = badge_get_count(package, &count);
	if (BADGE_ERROR_NONE != err) _D("cannot get badge count");

	_D("Badge for package %s : %u", package, count);

	return (int) count;
}



static Eina_Bool _idler_cb(void *data)
{
	char *package;
	int count;
	Evas_Object *item = data;

	package = item_get_package(item);
	if (!package) {
		_D("Failed to get a package name");
		evas_object_data_del(item, "idle_timer");
		return ECORE_CALLBACK_CANCEL;
	}

	count = item_badge_count(package);
	if (count) item_show_badge(item, count);
	else item_hide_badge(item);

	evas_object_data_del(item, "idle_timer");

	return ECORE_CALLBACK_CANCEL;
}



HAPI bool item_badge_is_registered(Evas_Object *item)
{
	const char *pkgname;
	badge_error_e err;
	bool existing = false;

	pkgname = item_get_package(item);
	retv_if(NULL == pkgname, false);

	err = badge_is_existing(pkgname, &existing);
	if (BADGE_ERROR_NONE != err) _E("cannot know whether the badge for %s is or not.", pkgname);

	return existing? true : false;
}



HAPI menu_screen_error_e item_badge_register(Evas_Object *item)
{
	Ecore_Idler *idle_timer;
	bool is_registered;

	is_registered = item_badge_is_registered(item);
	if (false == is_registered) return MENU_SCREEN_ERROR_OK;

	idle_timer = ecore_idler_add(_idler_cb, item);
	retv_if(NULL == idle_timer, MENU_SCREEN_ERROR_FAIL);
	evas_object_data_set(item, "idle_timer", idle_timer);

	return MENU_SCREEN_ERROR_OK;
}



HAPI void item_badge_unregister(Evas_Object *item)
{
	char *package;
	Ecore_Idler *idle_timer;

	package = item_get_package(item);
	if (!package) return;

	idle_timer = evas_object_data_get(item, "idle_timer");
	if (idle_timer) {
		_D("Badge handler for package %s is not yet ready", package);
		evas_object_data_del(item, "idle_timer");
		ecore_idler_del(idle_timer);
		return;
	}

	item_hide_badge(item);
}



static void _badge_change_cb(unsigned int action, const char *pkgname, unsigned int count, void *data)
{
	Evas_Object *scroller = data;
	Evas_Object *item;
	unsigned int is_display = 0;
	badge_error_e err;

	_D("Badge changed, action : %u, pkgname : %s, count : %u", action, pkgname, count);

	ret_if(NULL == pkgname);

	if (BADGE_ACTION_REMOVE == action) {
		count = 0;
		is_display = false;
	} else {
		err = badge_get_display(pkgname, &is_display);
		if (BADGE_ERROR_NONE != err) _D("cannot get badge display");
		if (0 == is_display) count = 0;
	}

	item = page_scroller_find_item_by_package(scroller, pkgname, NULL);
	if (NULL == item) return;

	if (count) item_show_badge(item, count);
	else item_hide_badge(item);
}



HAPI void item_badge_register_changed_cb(Evas_Object *scroller)
{
	badge_error_e err;

	err = badge_register_changed_cb(_badge_change_cb, scroller);
	ret_if(BADGE_ERROR_NONE != err);
}



HAPI void item_badge_unregister_changed_cb(void)
{
	badge_error_e err;

	err = badge_unregister_changed_cb(_badge_change_cb);
	ret_if(BADGE_ERROR_NONE != err);
}



// End of a file
