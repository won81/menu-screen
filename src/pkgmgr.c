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
#include <package-manager.h>
#include <pkgmgr-info.h>

#include "conf.h"
#include "index.h"
#include "item.h"
#include "list.h"
#include "page.h"
#include "page_scroller.h"
#include "mapbuf.h"
#include "pkgmgr.h"
#include "util.h"



struct pkgmgr_handler {
	const char *key;
	int (*func)(const char *package, const char *val, void *data);
};



static struct {
	pkgmgr_client *listen_pc;
} pkg_mgr_info = {
	.listen_pc = NULL,
};



HAPI inline menu_screen_error_e pkgmgr_uninstall(Evas_Object *item)
{
	int ret = MENU_SCREEN_ERROR_OK;

	retv_if(NULL == item, MENU_SCREEN_ERROR_FAIL);

	char *pkgid = NULL;
	char *appid = item_get_package(item);
	retv_if(NULL == appid, MENU_SCREEN_ERROR_FAIL);

	pkgmgr_client *req_pc = NULL;
	req_pc = pkgmgr_client_new(PC_REQUEST);
	retv_if(NULL == req_pc, MENU_SCREEN_ERROR_FAIL);

	pkgmgrinfo_appinfo_h handle;
	if (PMINFO_R_OK != pkgmgrinfo_appinfo_get_appinfo(appid, &handle)) {
		if (PKGMGR_R_OK != pkgmgr_client_free(req_pc)) {
			_E("cannot free pkgmgr_client for request.");
		}
		return MENU_SCREEN_ERROR_FAIL;
	}

	if (PMINFO_R_OK != pkgmgrinfo_appinfo_get_pkgid(handle, &pkgid)) {
		if (PMINFO_R_OK != pkgmgrinfo_appinfo_destroy_appinfo(handle)) {
			_E("cannot destroy the appinfo");
		}

		if (PKGMGR_R_OK != pkgmgr_client_free(req_pc)) {
			_E("cannot free pkgmgr_client for request.");
		}

		return MENU_SCREEN_ERROR_FAIL;
	}

	if (!pkgid) pkgid = appid;

	_D("Uninstall a package[%s] from an app[%s]", pkgid, appid);
	if (pkgmgr_client_uninstall(req_pc, NULL, pkgid, PM_QUIET, NULL, NULL) < 0) {
		_E("cannot uninstall %s.", item_get_package(item));
		ret = MENU_SCREEN_ERROR_FAIL;
	}

	if (PMINFO_R_OK != pkgmgrinfo_appinfo_destroy_appinfo(handle)) {
		_E("cannot destroy the appinfo");
		ret = MENU_SCREEN_ERROR_FAIL;
	}

	if (PMINFO_R_OK != pkgmgr_client_free(req_pc)) {
		_E("cannot free pkgmgr_client");
		ret = MENU_SCREEN_ERROR_FAIL;
	}

	return ret;
}



static menu_screen_error_e _start_download(const char *package, void *scroller)
{
	struct package_info *pi;
	Eina_List *install_list;

	install_list = evas_object_data_get(scroller, "install_list");
	pi = calloc(1, sizeof(struct package_info));
	retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

	pi->status = DOWNLOAD_BEGIN;
	pi->ai.package = strdup(package);
	pi->ai.name = strdup("Download");

	install_list = eina_list_append(install_list, pi);
	evas_object_data_set(scroller, "install_list", install_list);
	_D("Package [%s] is jump into the downloading phase", package);

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _start_uninstall(const char *package, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	int page_no = 0;
	struct package_info *pi = NULL;
	Eina_List *install_list;

	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}
		pi = NULL;
	}

	retv_if(pi, MENU_SCREEN_ERROR_FAIL);

	pi = calloc(1, sizeof(struct package_info));
	retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

	pi->status = UNINSTALL_BEGIN;
	pi->ai.package = strdup(package);
	pi->ai.nodisplay = false;
	pi->ai.enabled = true;
	pi->item = page_scroller_find_item_by_package(scroller, package, &page_no);
	pi->page = page_scroller_get_page_at(scroller, page_no);

	install_list = eina_list_append(install_list, pi);
	evas_object_data_set(scroller, "install_list", install_list);

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _start_update(const char *package, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi = NULL;
	Eina_List *install_list;

	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}
		pi = NULL;
	}

	if (!pi) {
		int page_no = 0;
		_D("Package [%s] is starting update phase directly (without downloading phase)", package);

		pi = calloc(1, sizeof(struct package_info));
		retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

		pi->ai.package = strdup(package);
		pi->item = page_scroller_find_item_by_package(scroller, package, &page_no);
		if (pi->item) {
			pi->page = page_scroller_get_page_at(scroller, page_no);
		}

		if (pi->item && pi->page) {
			pi->ai.nodisplay = false;
			pi->ai.enabled = true;
		}

		install_list = eina_list_append(install_list, pi);
		evas_object_data_set(scroller, "install_list", install_list);
	} else {
		if (pi->status != DOWNLOAD_END && pi->status != INSTALL_END) {
			_D("Package [%s] is in invalid state (%d), cancel this", package, pi->status);
			install_list = eina_list_remove(install_list, pi);
			evas_object_data_set(scroller, "install_list", install_list);
			if (pi->item) {
				page_unpack_item(pi->page, pi->item);
				page_scroller_trim_items(scroller);
				item_destroy(pi->item);
			}

			list_free_values(&pi->ai);
			free(pi);
			return MENU_SCREEN_ERROR_FAIL;
		}
	}

	pi->status = UPDATE_BEGIN;
	pi->ai.name = strdup("Update");
	if (pi->item) {
		item_set_name(pi->item, pi->ai.name, 0);
	}

	_D("Package [%s] is jump into the updating phase", package);

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _start_recover(const char *package, void *scroller)
{
	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _start_install(const char *package, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi = NULL;
	Eina_List *install_list;

	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}
		pi = NULL;
	}

	if (!pi) {
		int page_no = 0;
		_D("Package [%s] is starting install phase directly (without downloading phase)", package);
		pi = calloc(1, sizeof(struct package_info));
		retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

		pi->ai.package = strdup(package);
		if (!pi->ai.package) {
			free(pi);
			return MENU_SCREEN_ERROR_FAIL;
		}

		pi->ai.icon = strdup(DEFAULT_ICON);
		if (!pi->ai.icon) {
			free(pi->ai.package);
			free(pi);
			return MENU_SCREEN_ERROR_FAIL;
		}

		pi->item = page_scroller_find_item_by_package(scroller, package, &page_no);
		if (!pi->item) {
			if (NULL == page_scroller_push_item(scroller, &pi->ai)) _E("Cannot push an item");
		}
		pi->item = page_scroller_find_item_by_package(scroller, package, &page_no);
		pi->page = page_scroller_get_page_at(scroller, page_no);

		if (pi->item && pi->page) {
			pi->ai.nodisplay = false;
			pi->ai.enabled = true;
		}

		install_list = eina_list_append(install_list, pi);
		evas_object_data_set(scroller, "install_list", install_list);
	} else {
		if (pi->status != DOWNLOAD_END && pi->status != INSTALL_END) {
			_D("Package [%s] is in invalid state (%d), cancel this", package, pi->status);
			install_list = eina_list_remove(install_list, pi);
			evas_object_data_set(scroller, "install_list", install_list);
			if (pi->item) {
				page_unpack_item(pi->page, pi->item);
				page_scroller_trim_items(scroller);
				item_destroy(pi->item);
			}

			list_free_values(&pi->ai);
			free(pi);
			return MENU_SCREEN_ERROR_FAIL;
		}
	}

	pi->status = INSTALL_BEGIN;
	pi->ai.name = strdup("Install");
	if (pi->item) {
		// Update app name to install
		item_set_name(pi->item, pi->ai.name, 0);
	}
	_D("Package [%s] is jump into the installing phase", package);

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _start(const char *package, const char *val, void *scroller)
{
	struct start_cb_set {
		const char *name;
		int (*handler)(const char *package, void *scroller);
	} start_cb[] = {
		{
			.name = "download",
			.handler = _start_download,
		},
		{
			.name = "uninstall",
			.handler = _start_uninstall,
		},
		{
			.name = "install",
			.handler = _start_install,
		},
		{
			.name = "update",
			.handler = _start_update,
		},
		{
			.name = "recover",
			.handler = _start_recover,
		},
		{
			.name = NULL,
			.handler = NULL,
		},
	};

	register unsigned int i;

	_D("package [%s]", package);

	for (i = 0; start_cb[i].name; i ++) {
		if (!strcasecmp(val, start_cb[i].name) && start_cb[i].handler) {
			return start_cb[i].handler(package, scroller);
		}
	}

	_D("Unknown status for starting phase signal'd from package manager");

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _icon_path(const char *package, const char *val, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi = NULL;
	Eina_List *install_list;

	install_list = evas_object_data_get(scroller, "install_list");
	retv_if(!package, MENU_SCREEN_ERROR_FAIL);
	retv_if(!val, MENU_SCREEN_ERROR_FAIL);

	_D("package [%s] with %s", package, val);

	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}
		pi = NULL;
	}
	retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

	if (strlen(val)) {
		pi->ai.icon = strdup(val);
		retv_if (NULL == pi->ai.icon, MENU_SCREEN_ERROR_OUT_OF_MEMORY);

		if (!pi->item) {
			_D("There is no item for [%s]", package);
			pi->ai.nodisplay = false;
			pi->ai.enabled = true;

			if (NULL == page_scroller_push_item(scroller, &pi->ai)) {
				_E("Failed to create a new item, remove this package from the installing list");
				list_free_values(&pi->ai);
				install_list = eina_list_remove(install_list, pi);
				evas_object_data_set(scroller, "install_list", install_list);
				free(pi);
			}
		} else {
			_D("There is an item for [%s:%p]", package, pi->item);
			item_update(pi->item, &pi->ai);
		}
	}

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _download_percent(const char *package, const char *val, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi = NULL;
	Eina_List *install_list;

	_D("package [%s] with %s", package, val);

	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}
		pi = NULL;
	}

	retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

	if (pi->status == DOWNLOAD_BEGIN) {
		pi->status = DOWNLOADING;
	} else if (pi->status == DOWNLOADING) {
		// Do nothing, just we are in proper state
	} else {
		_D("Invalid state for %s, This is not started from the download_begin state(%s)", package, val);
	}

	if (!pi->ai.nodisplay && pi->ai.enabled && pi->item) {
		if (!item_is_enabled_progress(pi->item)) {
			item_enable_progress(pi->item);
		}

		item_update_progress(pi->item, atoi(val));
	}

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _install_percent(const char *package, const char *val, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi;
	int progress;
	Eina_List *install_list;

	_D("package [%s] with %s", package, val);

	pi = NULL;
	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}

		pi = NULL;
	}

	retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

	progress = atoi(val);

	if (pi->status == INSTALL_BEGIN) {
		pi->status = INSTALLING;
	} else if (pi->status == UNINSTALL_BEGIN) {
		progress = 100 - progress;
		pi->status = UNINSTALLING;
	} else if (pi->status == UPDATE_BEGIN) {
		pi->status = UPDATING;
	} else if (pi->status == INSTALLING) {
	} else if (pi->status == UNINSTALLING) {
		progress = 100 - progress;
	} else if (pi->status == UPDATING) {
	} else {
		_D("Invalid state for %s, This is not the uninstall or install_begin state(%s)", package, val);
	}

	if (!pi->ai.nodisplay && pi->ai.enabled && pi->item) {
		if (!item_is_enabled_progress(pi->item)) {
			item_enable_progress(pi->item);
		}

		item_update_progress(pi->item, progress);
	}

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _error(const char *package, const char *val, void *scroller)
{
	struct package_info *pi = NULL;
	Eina_List *l;
	Eina_List *tmp;
	Eina_List *install_list;

	_D("package [%s] with %s", package, val);

	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}

		pi = NULL;
	}

	retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

	pi->error_count ++;
	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _end_downloading(const char *package, struct package_info *pi, void *scroller)
{
	pi->status = DOWNLOAD_END;
	_D("Package downloading is complete, waiting install progress");
	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _end_installing(const char *package, struct package_info *pi, void *scroller)
{
	// This status will not be referenced from the others.
	// because it will be freed right after set it. ;)
	Eina_List *install_list;

	pi->status = INSTALL_END;
	_D("Package installing is complete");

	install_list = evas_object_data_get(scroller, "install_list");
	if (pi->desktop_file_found == 1) {
		install_list = eina_list_remove(install_list, pi);
		evas_object_data_set(scroller, "install_list", install_list);
		list_free_values(&pi->ai);
		free(pi);
	}


	// TODO: Need to register a timer callback
	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _end_updating(const char *package, struct package_info *pi, void *scroller)
{
	// This status will not be referenced from the others.
	// because it will be freed right after set it. ;)
	Eina_List *install_list;

	pi->status = UPDATE_END;
	_D("Package updating is complete");

	install_list = evas_object_data_get(scroller, "install_list");
	if (pi->desktop_file_found == 1) {
		install_list = eina_list_remove(install_list, pi);
		evas_object_data_set(scroller, "install_list", install_list);
		list_free_values(&pi->ai);
		free(pi);
	}


	// TODO: Need to register a timer callback
	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _end_uninstalling(const char *package, struct package_info *pi, void *scroller)
{
	Eina_List *install_list;

	pi->status = UNINSTALL_END;
	_D("Package uninstalling is complete");

	install_list = evas_object_data_get(scroller, "install_list");
	if (pi->desktop_file_found == 1) {
		install_list = eina_list_remove(install_list, pi);
		evas_object_data_set(scroller, "install_list", install_list);
		list_free_values(&pi->ai);
		free(pi);
	}

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _end_unknown(const char *package, struct package_info *pi, void *scroller)
{
	Eina_List *install_list;

	install_list = evas_object_data_get(scroller, "install_list");
	install_list = eina_list_remove(install_list, pi);
	evas_object_data_set(scroller, "install_list", install_list);

	if (pi->item) {
		// Remove an item only if it is installing.
		if (
			pi->status == INSTALL_BEGIN || pi->status == INSTALLING || pi->status == INSTALL_END ||
			pi->status == DOWNLOAD_BEGIN || pi->status == DOWNLOADING || pi->status == DOWNLOAD_END
		)
		{
			if (pi->page) {
				page_unpack_item(pi->page, pi->item);
				page_scroller_trim_items(scroller);
			} else {
				_D("Page is not valid (%s)", package);
			}
			item_destroy(pi->item);
			page_scroller_trim_items(scroller);
		}
	}

	list_free_values(&pi->ai);
	free(pi);

	return MENU_SCREEN_ERROR_OK;
}



static menu_screen_error_e _end(const char *package, const char *val, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi;
	register int i;
	struct end_cb_set {
		int status;
		int (*handler)(const char *package, struct package_info *pi, void *scroller);
	} end_cb[] = {
		{
			.status = DOWNLOADING,
			.handler = _end_downloading,
		},
		{
			.status = INSTALLING,
			.handler = _end_installing,
		},
		{
			.status = UNINSTALLING,
			.handler = _end_uninstalling,
		},
		{
			.status = UPDATING,
			.handler = _end_updating,
		},
		{
			.status = UNKNOWN,
			.handler = _end_unknown,
		},
		{
			.status = MAX_STATUS,
			.handler = NULL,
		},
	};
	Eina_List *install_list;

	_D("package [%s], val [%s]", package, val);

	pi = NULL;
	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			break;
		}
		pi = NULL;
	}

	retv_if(NULL == pi, MENU_SCREEN_ERROR_FAIL);

	list_free_values(&pi->ai);
	if (MENU_SCREEN_ERROR_OK != list_get_values(package, &pi->ai)) _E("Cannot get values");
	item_update(pi->item, &pi->ai);

	if (item_is_enabled_progress(pi->item)) {
		item_disable_progress(pi->item);
	}

	// NOTE: Don't release the 'pi'
	//       Release it from each handler
	for (i = 0; end_cb[i].handler; i ++) {
		if (end_cb[i].status == pi->status && end_cb[i].handler) {
			int ret;

			if (strcasecmp(val, "ok")) {
				ret = _end_unknown(package, pi, scroller);
			} else {
				ret = end_cb[i].handler(package, pi, scroller);
			}

			return ret;
		}
	}

	return _end_unknown(package, pi, scroller);
}



static menu_screen_error_e _change_pkg_name(const char *package, const char *val, void *scroller)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi;
	Eina_List *install_list;

	_D("package [%s]", package);

	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (!pi) {
			continue;
		}
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			_D("Replace package name %s to %s", pi->ai.package, val);
			free(pi->ai.package);
			pi->ai.package = strdup(val);
			if (!pi->ai.package) {
				_E("cannot strdup");
				return MENU_SCREEN_ERROR_FAIL;
			}
			if (pi->item) {
				item_set_package(pi->item, (char*) val, 0);
			}
			return MENU_SCREEN_ERROR_OK;
		}
	}

	return MENU_SCREEN_ERROR_FAIL;
}



static struct pkgmgr_handler pkgmgr_cbs[] = {
	{ "start", _start },
	{ "icon_path", _icon_path },
	{ "download_percent", _download_percent },
	{ "command", NULL },
	{ "install_percent", _install_percent },
	{ "error", _error },
	{ "end", _end },
	{ "change_pkg_name", _change_pkg_name },
};



static menu_screen_error_e _pkgmgr_cb(int req_id, const char *pkg_type, const char *package, const char *key, const char *val, const void *pmsg, void *data)
{
	register unsigned int i;
	Evas_Object *scroller = data;

	_D("pkgmgr request [%s] for %s, val(%s)", key, package, val);

	for (i = 0; i < sizeof(pkgmgr_cbs) / sizeof(struct pkgmgr_handler); i++) {
		if (!strcasecmp(pkgmgr_cbs[i].key, key)) {
			if (pkgmgr_cbs[i].func) {
				if (MENU_SCREEN_ERROR_OK != pkgmgr_cbs[i].func(package, val, scroller)) {
					_E("pkgmgr_cbs[%u].func has errors.", i);
				}
			} else {
				_E("Cannot find pkgmgr_cbs[%u].func.", i);
			}
			return MENU_SCREEN_ERROR_OK;
		}
	}

	return MENU_SCREEN_ERROR_FAIL;
}



HAPI menu_screen_error_e pkgmgr_init(Evas_Object *scroller)
{
	if (NULL != pkg_mgr_info.listen_pc) {
		return MENU_SCREEN_ERROR_OK;
	}

	pkg_mgr_info.listen_pc = pkgmgr_client_new(PC_LISTENING);
	retv_if(NULL == pkg_mgr_info.listen_pc, MENU_SCREEN_ERROR_FAIL);
	retv_if(pkgmgr_client_listen_status(pkg_mgr_info.listen_pc,
			_pkgmgr_cb, scroller) != PKGMGR_R_OK, MENU_SCREEN_ERROR_FAIL);

	return MENU_SCREEN_ERROR_OK;
}



HAPI void pkgmgr_fini(void)
{
	ret_if(NULL == pkg_mgr_info.listen_pc);
	if (pkgmgr_client_free(pkg_mgr_info.listen_pc) != PKGMGR_R_OK) {
		_E("cannot free pkgmgr_client for listen.");
	}
	pkg_mgr_info.listen_pc = NULL;
}



HAPI Evas_Object *pkgmgr_find_pended_object(const char *package, int with_desktop_file, Evas_Object *scroller, Evas_Object **page)
{
	Eina_List *l;
	Eina_List *tmp;
	struct package_info *pi;
	Eina_List *install_list;

	_D("package [%s]", package);

	retv_if(NULL == package, NULL);
	install_list = evas_object_data_get(scroller, "install_list");
	EINA_LIST_FOREACH_SAFE(install_list, l, tmp, pi) {
		if (!pi) {
			continue;
		}
		if (pi->ai.package && !strcmp(pi->ai.package, package)) {
			Evas_Object *item;
			_D("Installing(Downloading) package is found (%p)", pi->item);

			item = pi->item;

			if (with_desktop_file) {
				pi->desktop_file_found = 1;

				if (pi->status == INSTALL_END || pi->status == UNINSTALL_END || pi->status == UPDATE_END) {
					install_list = eina_list_remove(install_list, pi);
					evas_object_data_set(scroller, "install_list", install_list);
					list_free_values(&pi->ai);
					free(pi);
					pi = NULL;
				}
			}

			if (page) {
				*page = pi ? pi->page : NULL;
			}
			return item;
		}
	}

	_D("Failed to find a installing/downloading package");
	return NULL;
}



// End of a file
