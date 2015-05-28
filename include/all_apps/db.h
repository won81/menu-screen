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



#ifndef __MENU_SCREEN_ALL_APPS_DB_H__
#define __MENU_SCREEN_ALL_APPS_DB_H__

#include "Elementary.h"
#include "util.h"

typedef struct _db_info {
	long long rowid;
	int type;

	char *appid;
	char *name;
	char *content_info;
	char *icon;
} db_info;

extern menu_screen_error_e all_apps_db_init(void);
extern void all_apps_db_fini(void);

extern Eina_List *all_apps_db_retrieve_all_info(void);
extern void all_apps_db_unretrieve_info(db_info *info);
extern void all_apps_db_unretrieve_all_info(Eina_List *list);

extern long long all_apps_db_insert_shortcut(const char *appid, const char *name, int type, const char *content_info, const char *icon);
extern menu_screen_error_e all_apps_db_delete_shortcut(long long rowid);
extern int all_apps_db_count_shortcut(const char *appid, const char *name);

#endif // __MENU_SCREEN_ALL_APPS_DB_H__
// End of file
