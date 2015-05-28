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



#ifndef __MENU_SCREEN_PKGMGR_H__
#define __MENU_SCREEN_PKGMGR_H__

#include <Evas.h>

#include "list.h"
#include "util.h"



enum package_install_status {
	UNKNOWN = 0x00,
	DOWNLOAD_BEGIN,
	DOWNLOADING,
	DOWNLOAD_END,
	INSTALL_BEGIN,
	INSTALLING,
	INSTALL_END,
	UNINSTALL_BEGIN,
	UNINSTALLING,
	UNINSTALL_END,
	UPDATE_BEGIN,
	UPDATING,
	UPDATE_END,
	MAX_STATUS,
};



struct package_info {
	enum package_install_status status;
	app_info_t ai;
	Evas_Object *item;
	Evas_Object *page;
	Eina_Bool desktop_file_found;
	int error_count;
};



extern menu_screen_error_e pkgmgr_init(Evas_Object *scroller);
extern void pkgmgr_fini(void);
extern Evas_Object *pkgmgr_find_pended_object(const char *package, int with_desktop_file, Evas_Object *scroller, Evas_Object **page);
extern menu_screen_error_e pkgmgr_uninstall(Evas_Object *obj);

#endif //__MENU_SCREEN_PKGMGR_H__

// End of a file
