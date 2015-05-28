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



#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "db.h"
#include "util.h"
#include "all_apps/db.h"

#define QUERY_LEN 1024

#define MENU_SCREEN_DB_FILE "/opt/usr/apps/com.samsung.menu-screen/data/dbspace/.menu_screen.db"
#define SHORTCUT_TABLE "shortcut"
#define QUERY_INSERT_SHORTCUT "INSERT INTO "SHORTCUT_TABLE" ("\
	"appid,"\
	"name,"\
	"type,"\
	"content_info,"\
	"icon"\
	") VALUES ("\
	"'%s', '%s', %d, '%s', '%s');"
#define QUERY_DELETE_SHORTCUT "DELETE FROM "SHORTCUT_TABLE" WHERE ROWID=%lld"
#define QUERY_GET_ALL "SELECT ROWID, appid, name, type, content_info, icon FROM "SHORTCUT_TABLE
#define QUERY_COUNT_SHORTCUT "SELECT COUNT(*) FROM "SHORTCUT_TABLE" WHERE appid='%s' AND name='%s'"



HAPI menu_screen_error_e all_apps_db_init(void)
{
	return db_open(MENU_SCREEN_DB_FILE);
}



HAPI void all_apps_db_fini(void)
{
	db_close();
}



HAPI Eina_List *all_apps_db_retrieve_all_info(void)
{
	stmt_h *st;
	Eina_List *list = NULL;

	retv_if(MENU_SCREEN_ERROR_OK != db_open(MENU_SCREEN_DB_FILE), NULL);

	st = db_prepare(QUERY_GET_ALL);
	retv_if(NULL == st, NULL);

	menu_screen_error_e ret = MENU_SCREEN_ERROR_FAIL;
	for (ret = db_next(st); MENU_SCREEN_ERROR_FAIL != ret && MENU_SCREEN_ERROR_NO_DATA != ret; ret = db_next(st)) {
		db_info *info;
		info = calloc(1, sizeof(db_info));
		break_if(NULL == info);

		info->rowid = db_get_long_long(st, 0); // 0 : ROWID

		char *tmp = NULL;
		tmp = (char *) db_get_str(st, 1); // 1 : appid
		if (tmp && strlen(tmp)) {
			info->appid = strdup(tmp);
			goto_if(NULL == info->appid, APP_ERROR);
		}

		tmp = (char *) db_get_str(st, 2); // 2 : name
		if (tmp && strlen(tmp)) {
			info->name = strdup(tmp);
			goto_if(NULL == info->name, APP_ERROR);
		}

		info->type = db_get_int(st, 3); // 3 : type

		tmp = (char *) db_get_str(st, 4); // 4 : content_info
		if (tmp && strlen(tmp)) {
			info->content_info = strdup(tmp);
			goto_if(NULL == info->content_info, APP_ERROR);
		}

		tmp = (char *) db_get_str(st, 5); // 5 : icon
		if (tmp && strlen(tmp)) {
			info->icon = strdup(tmp);
			goto_if(NULL == info->icon, APP_ERROR);
		}

		list = eina_list_append(list, info);

		continue;
APP_ERROR:
		if (info->appid) free(info->appid);
		if (info->name) free(info->name);
		if (info->content_info) free(info->content_info);
		if (info->icon) free(info->icon);
		if (info) free(info);
	}

	db_finalize(st);

	return list;
}



HAPI void all_apps_db_unretrieve_info(db_info *info)
{
	ret_if(NULL == info);
	if (info->appid) free(info->appid);
	if (info->name) free(info->name);
	if (info->content_info) free(info->content_info);
	if (info->icon) free(info->icon);
	if (info) free(info);
}



HAPI void all_apps_db_unretrieve_all_info(Eina_List *list)
{
	db_info *info = NULL;

	EINA_LIST_FREE(list, info) {
		if (NULL == info) break;
		if (info->appid) free(info->appid);
		if (info->name) free(info->name);
		if (info->content_info) free(info->content_info);
		if (info->icon) free(info->icon);
		if (info) free(info);
	}

	eina_list_free(list);
}



HAPI int all_apps_db_count_shortcut(const char *appid, const char *name)
{
	retv_if(MENU_SCREEN_ERROR_OK != db_open(MENU_SCREEN_DB_FILE), -1);

	char q[QUERY_LEN];
	snprintf(q, sizeof(q), QUERY_COUNT_SHORTCUT, appid, name);

	stmt_h *st;
	st = db_prepare(q);
	retv_if(NULL == st, -1);

	menu_screen_error_e ret = MENU_SCREEN_ERROR_FAIL;
	ret = db_next(st);
	retv_if(MENU_SCREEN_ERROR_FAIL == ret, -1);

	int count = -1;
	count = db_get_int(st, 0);

	db_finalize(st);

	return count;
}



HAPI long long all_apps_db_insert_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon)
{
	char q[QUERY_LEN];

	retv_if(MENU_SCREEN_ERROR_OK != db_open(MENU_SCREEN_DB_FILE), -1l);

	snprintf(q, sizeof(q), QUERY_INSERT_SHORTCUT, appid, name, type, content_info, icon);
	retv_if(db_exec(q) < 0, -1l);

	long long id = -1l;
	id = db_last_insert_rowid();

	return id;
}



HAPI menu_screen_error_e all_apps_db_delete_shortcut(long long rowid)
{
	char q[QUERY_LEN];

	retv_if(MENU_SCREEN_ERROR_OK != db_open(MENU_SCREEN_DB_FILE), MENU_SCREEN_ERROR_FAIL);

	snprintf(q, sizeof(q), QUERY_DELETE_SHORTCUT, rowid);
	retv_if(db_exec(q) < 0, MENU_SCREEN_ERROR_FAIL);

	return MENU_SCREEN_ERROR_OK;
}



// END
