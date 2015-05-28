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



#include <shortcut.h>
#include <stdbool.h>

#include "item.h"
#include "list.h"
#include "page_scroller.h"
#include "util.h"
#include "all_apps/db.h"



HAPI Evas_Object *all_apps_shortcut_add(
		Evas_Object *scroller,
		long long rowid,
		const char *pkgname,
		const char *exec,
		const char *name,
		const char *content_info,
		const char *icon,
		int type)
{
	_D("Shortcut : pkgname(%s) exec(%s) name(%s) icon(%s)", pkgname, exec, name, icon);

	app_info_t ai = {0, };
	ai.package = (char *) pkgname;
	ai.exec = (char *) exec;
	ai.name = (char *) name;
	ai.icon = (char *) icon;
	ai.nodisplay = 0;
	ai.enabled = 1;
	ai.x_slp_removable = 1;
	ai.x_slp_taskmanage = 0;

	if (ADD_TO_HOME_IS_LIVEBOX(type)) {
		_D("This is a livebox");
		return NULL;
	}

	Evas_Object *item = NULL;
	retv_if(NULL == (item = page_scroller_push_item(scroller, &ai)), NULL);

	long long *tmp;
	tmp = calloc(1, sizeof(long long));
	if (NULL == tmp) {
		item_destroy(item);
		return NULL;
	}

	if (LAUNCH_BY_PACKAGE == type) {
		_D("This is a package");
		evas_object_data_set(item, "shortcut_launch_package", (void *) true);
	} else {
		_D("This is a shortcut");
		evas_object_data_set(item, "shortcut_launch_package", (void *) false);
	}

	*tmp = rowid;
	evas_object_data_set(item, "is_shortcut", (void *) true);
	evas_object_data_set(item, "rowid", tmp);
	evas_object_data_set(item, "type", (void *) type);
	evas_object_data_set(item, "content_info", content_info);

	return item;
}



static Eina_Bool _push_items_idler_cb(void *data)
{
	Evas_Object *scroller = data;
	Eina_List *list = evas_object_data_get(scroller, "list");
	Eina_List *n;
	Eina_List *t;
	db_info *info = NULL;
	EINA_LIST_FOREACH_SAFE(list, n, t, info) {
		goto_if(NULL == info, ERROR);
		Evas_Object *item = NULL;
		item = all_apps_shortcut_add(
			scroller,
			info->rowid,
			info->appid,
			NULL,
			info->name,
			info->content_info,
			info->icon,
			info->type);

		list = eina_list_remove(list, info);
		evas_object_data_set(scroller, "list", list);

		all_apps_db_unretrieve_info(info);
		break_if(NULL == item);
	}

	return ECORE_CALLBACK_RENEW;
ERROR:
	list = evas_object_data_del(scroller, "list");
	all_apps_db_unretrieve_all_info(list);
	return ECORE_CALLBACK_CANCEL;
}



HAPI menu_screen_error_e all_apps_shortcut_add_all(Evas_Object *scroller)
{
	Eina_List *list;

	list = all_apps_db_retrieve_all_info();
	if (NULL == list) {
		_D("There is no shortcut");
		return MENU_SCREEN_ERROR_OK;
	}

	evas_object_data_set(scroller, "list", list);

	Ecore_Idler *idle_timer = NULL;
	idle_timer = ecore_idler_add(_push_items_idler_cb, scroller);
	retv_if(NULL == idle_timer, MENU_SCREEN_ERROR_FAIL);

	return MENU_SCREEN_ERROR_OK;
}



HAPI void all_apps_shortcut_remove(Evas_Object *item)
{
	long long *tmp;
	tmp = evas_object_data_del(item, "rowid");
	if (tmp) {
		all_apps_db_delete_shortcut(*tmp);
		free(tmp);
	}

	Evas_Object *scroller;
	scroller = evas_object_data_get(item, "scroller");

	evas_object_data_del(item, "type");
	evas_object_data_del(item, "content_info");
	evas_object_data_del(item, "shortcut_launch_package");
	evas_object_data_del(item, "is_shortcut");
	item_destroy(item);

	if (scroller) page_scroller_trim_items(scroller);
}



static int _shorcut_request_cb(
		const char *pkgname,
		const char *name,
		int type,
		const char *content_info,
		const char *icon,
		int pid,
		double period,
		int allow_duplicate,
		void *data)
{
	Evas_Object *scroller = data;

	retv_if(NULL == pkgname, -1);

	_D("Package name: %s", pkgname);
	_D("Name: %s", name);
	_D("Type: %d", type);
	_D("Content: %s", content_info);
	_D("Icon: %s", icon);
	_D("Requested from: %d", pid);
	_D("period : %.2f", period);
	_D("CBDATA: %p", data);

	if (!allow_duplicate) {
		int count = 0;
		count = all_apps_db_count_shortcut(pkgname, name);
		if (0 < count) {
			_D("There is already a package(%s:%s) in the Menu-screen", pkgname, name);
			return -1;
		}
	}

	long long rowid = -1l;
	rowid = all_apps_db_insert_shortcut(pkgname, name, type, content_info, icon);
	retv_if(0l > rowid, -1);

	Evas_Object *item = NULL;
	item = all_apps_shortcut_add(scroller, rowid, pkgname, NULL, name, content_info, icon, type);
	retv_if(NULL == item, -1);

	return 0;
}



HAPI bool all_apps_shortcut_init(Evas_Object *all_apps)
{
	retv_if(NULL == all_apps, false);
	retv_if(MENU_SCREEN_ERROR_OK != all_apps_db_init(), false);

	Evas_Object *scroller;
	scroller = evas_object_data_get(all_apps, "scroller");
	retv_if(NULL == scroller, false);

	int ret;
	ret = shortcut_set_request_cb(_shorcut_request_cb, scroller);

	return 0 == ret ? true : false;
}



HAPI void all_apps_shortcut_fini(void)
{
	all_apps_db_fini();
}



// End of file.
