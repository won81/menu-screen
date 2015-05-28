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



#include <ail.h>
#include <app.h>
#include <aul.h>
#include <Ecore_X.h>
#include <Elementary.h>
#include <stdbool.h>
#include <system_info.h>
#include <utilX.h>
#include <vconf.h>

#include "conf.h"
#include "item.h"
#include "key.h"
#include "layout.h"
#include "mapbuf.h"
#include "menu_screen.h"
#include "mouse.h"
#include "page.h"
#include "page_scroller.h"
#include "util.h"

#define MENU_SCREEN_ENGINE "file/private/org.tizen.menu-screen/engine"

#define LAYOUT_EDJE_PORTRAIT EDJEDIR"/layout_portrait.edj"
#define LAYOUT_GROUP_NAME "layout"



// Define prototype of the "hidden API of AUL"
extern int aul_listen_app_dead_signal(int (*func)(int signal, void *data), void *data);
static struct {
	int state;
	int root_width;
	int root_height;
	int is_tts;
	Evas *evas;
	Ecore_Evas *ee;
	Evas_Object *win;
	Elm_Theme *theme;
	bool is_done;
} menu_screen_info = {
	.state = APP_STATE_PAUSE,
	.is_tts = false,
	.evas = NULL,
	.ee = NULL,
	.win = NULL,
	.theme = NULL,
	.is_done = false,
};



HAPI Evas *menu_screen_get_evas(void)
{
	return menu_screen_info.evas;
}



HAPI int menu_screen_get_root_width(void)
{
	return menu_screen_info.root_width;
}



HAPI int menu_screen_get_root_height(void)
{
	return menu_screen_info.root_height;
}



HAPI Evas_Object *menu_screen_get_win(void)
{
	return menu_screen_info.win;
}



HAPI Elm_Theme *menu_screen_get_theme(void)
{
	return menu_screen_info.theme;
}



HAPI bool menu_screen_get_done(void)
{
	return menu_screen_info.is_done;
}



HAPI void menu_screen_set_done(bool is_done)
{
	menu_screen_info.is_done = is_done;
}



HAPI int menu_screen_get_state(void)
{
	return menu_screen_info.state;
}



HAPI int menu_screen_is_tts(void)
{
	return menu_screen_info.is_tts;
}



static menu_screen_error_e _create_canvas(char *name, char *title)
{
	Ecore_X_Atom ATOM_WM_WINDOW_ROLE;

	menu_screen_info.win = elm_win_add(NULL, name, ELM_WIN_BASIC);
	retv_if(NULL == menu_screen_info.win, MENU_SCREEN_ERROR_FAIL);

	if (title) {
		elm_win_title_set(menu_screen_info.win, title);
	}
	elm_win_borderless_set(menu_screen_info.win, EINA_TRUE);

	ecore_x_icccm_name_class_set(elm_win_xwindow_get(menu_screen_info.win), "MENU_SCREEN", "MENU_SCREEN");
	ATOM_WM_WINDOW_ROLE = ecore_x_atom_get("WM_WINDOW_ROLE");
	if (ATOM_WM_WINDOW_ROLE) {
		ecore_x_window_prop_string_set(elm_win_xwindow_get(menu_screen_info.win), ATOM_WM_WINDOW_ROLE, "MENU_SCREEN");
	} else {
		_D("Failed to set the window role as MENU_SCREEN");
	}

	menu_screen_info.evas = evas_object_evas_get(menu_screen_info.win);
	if (!menu_screen_info.evas) {
		_E("[%s] Failed to get the evas object", __func__);
	}

	menu_screen_info.ee = ecore_evas_ecore_evas_get(menu_screen_info.evas);
	if (!menu_screen_info.ee) {
		_E("[%s] Failed to get ecore_evas object", __func__);
	}

	evas_object_size_hint_min_set(menu_screen_info.win, menu_screen_info.root_width, menu_screen_info.root_height);
	evas_object_size_hint_max_set(menu_screen_info.win, menu_screen_info.root_width, menu_screen_info.root_height);
	evas_object_resize(menu_screen_info.win, menu_screen_info.root_width, menu_screen_info.root_height);
	evas_object_show(menu_screen_info.win);

	return MENU_SCREEN_ERROR_OK;
}



static void _destroy_canvas(void)
{
	evas_object_del(menu_screen_info.win);
}



static int _dead_cb(int pid, void *data)
{
	utilx_hide_fake_effect(
		ecore_x_display_get(),
		ecore_x_window_root_get(ecore_evas_window_get(menu_screen_info.ee))
	);

	return EXIT_SUCCESS;
}



static void _get_window_size(void)
{
	Ecore_X_Window focus_win;
	Ecore_X_Window root_win;

	focus_win = ecore_x_window_focus_get();
	root_win = ecore_x_window_root_get(focus_win);
	ecore_x_window_size_get(root_win, &menu_screen_info.root_width, &menu_screen_info.root_height);

	_D("width:%d, height:%d", menu_screen_info.root_width, menu_screen_info.root_height);
}



static void _create_bg(void)
{
	char *buf;
	Evas_Coord w;
	Evas_Coord h;
	Evas_Object *bg;
	double f, wf, hf;
	static int trigger = 0;
	const char *key;
	int width;
	int height;

	buf = vconf_get_str(VCONFKEY_BGSET);
	ret_if(NULL == buf);

	width = menu_screen_get_root_width();
	height = menu_screen_get_root_height();

	bg = evas_object_data_get(menu_screen_get_win(), "bg");
	if (NULL == bg) {
		Evas_Object *rect;

		rect = evas_object_rectangle_add(menu_screen_get_evas());
		ret_if(NULL == rect);
		evas_object_data_set(menu_screen_get_win(), "rect", rect);
		evas_object_color_set(rect, 0, 0, 0, 255);
		evas_object_size_hint_min_set(rect, width, height);
		evas_object_size_hint_max_set(rect, width, height);
		evas_object_resize(rect, width, height);
		evas_object_show(rect);

		bg = evas_object_image_add(menu_screen_get_evas());
		if (NULL == bg) {
			free(buf);
			return;
		}
		evas_object_image_load_orientation_set(bg, EINA_TRUE);
		evas_object_data_set(menu_screen_get_win(), "bg", bg);
	}

	if (trigger == 0) {
		key = "/";
		trigger = 1;
	} else {
		key = NULL;
		trigger = 0;
	}

	evas_object_image_file_set(bg, buf, key);
	evas_object_image_size_get(bg, &w, &h);
	evas_object_image_filled_set(bg, 1);

	wf = (double) width / (double) w;
	hf = (double) height / (double) h;

	f = wf < hf ? hf : wf;

	w = (int) ((double) f * (double) w);
	h = (int) ((double) f * (double) h);

	evas_object_image_load_size_set(bg, w, h);
	evas_object_image_fill_set(bg, 0, 0, w, h);
	evas_object_move(bg, (width - w) / 2, (height - h) / 2);
	evas_object_resize(bg, w, h);
	evas_object_show(bg);

	free(buf);
}




static void _destroy_bg()
{
	Evas_Object *rect;
	Evas_Object *bg;

	rect = evas_object_data_del(menu_screen_get_win(), "rect");
	evas_object_del(rect);

	bg = evas_object_data_del(menu_screen_get_win(), "bg");
	evas_object_del(bg);
}



static void _change_bg_cb(keynode_t *node, void *data)
{
	_D("Background image is changed.");
	_create_bg();
}



static void _init_theme(void)
{
	menu_screen_info.theme = elm_theme_new();
	elm_theme_ref_set(menu_screen_info.theme, NULL);
	elm_theme_extension_add(menu_screen_info.theme, EDJEDIR"/index.edj");
}



static void _fini_theme(void)
{
	elm_theme_extension_del(menu_screen_info.theme, EDJEDIR"/index.edj");
	elm_theme_free(menu_screen_info.theme);
	menu_screen_info.theme = NULL;

}



static Evas_Object *_create_conformant(Evas_Object *win)
{
	Evas_Object *conformant;

	conformant = elm_conformant_add(win);
	retv_if(NULL == conformant, NULL);

	elm_object_style_set(conformant, "nokeypad");
	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_data_set(conformant, "win", win);
	evas_object_show(conformant);

	elm_win_resize_object_add(win, conformant);
	elm_win_conformant_set(win, EINA_TRUE);

	return conformant;
}



static void _destroy_conformant(Evas_Object *conformant)
{
	evas_object_data_del(conformant, "win");
	evas_object_del(conformant);
}



static void _tts_cb(keynode_t *node, void *data)
{
	_D("change tts");

	int val = -1;
	if (0 == vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &val) &&
			menu_screen_info.is_tts != val)
	{
		menu_screen_info.is_tts = val;
	}
}



static bool _create_cb(void *data)
{
	Evas_Object *conformant;

	_get_window_size();
	_init_theme();
	retv_if(MENU_SCREEN_ERROR_FAIL == _create_canvas(PACKAGE, PACKAGE), false);
	elm_win_indicator_mode_set(menu_screen_info.win, ELM_WIN_INDICATOR_SHOW);

	if (vconf_notify_key_changed(VCONFKEY_BGSET, _change_bg_cb, NULL) < 0) {
		_E("Failed to register a vconf cb for %s\n", VCONFKEY_BGSET);
	}
	_create_bg();

	conformant = _create_conformant(menu_screen_info.win);
	retv_if(NULL == conformant, false);
	evas_object_data_set(menu_screen_info.win, "conformant", conformant);

	Evas_Object *layout;
	layout = layout_create(conformant, LAYOUT_EDJE_PORTRAIT,
				LAYOUT_GROUP_NAME, MENU_SCREEN_ROTATE_PORTRAIT);
	if (NULL == layout) {
		_E("Failed to load an edje object");
		evas_object_del(menu_screen_info.win);
		return false;
	}
	evas_object_data_set(menu_screen_info.win, "layout", layout);

if (vconf_notify_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, _tts_cb, NULL) < 0) {
		_E("Failed to register the tts callback");
	}
	retv_if(vconf_get_bool(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, &menu_screen_info.is_tts) < 0, MENU_SCREEN_ERROR_FAIL);

	elm_object_content_set(conformant, layout);
	mouse_register();
	aul_listen_app_dead_signal(_dead_cb, NULL);
	key_register();

	return true;
}



static void _terminate_cb(void *data)
{
	Evas_Object *conformant;
	Evas_Object *layout;

	if (vconf_ignore_key_changed(VCONFKEY_BGSET, _change_bg_cb) < 0) {
		_E("Failed to remove bgset [%s]\n", VCONFKEY_BGSET);
	}

	if (vconf_ignore_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_TTS, _tts_cb) < 0) {
		_E("Failed to ignore the alpha callback");
	}

	evas_object_hide(menu_screen_info.win);

	key_unregister();
	mouse_unregister();

	layout = evas_object_data_del(menu_screen_info.win, "layout");
	if (layout) layout_destroy(layout);

	conformant = evas_object_data_del(menu_screen_info.win, "conformant");
	if (conformant) _destroy_conformant(conformant);

	_destroy_bg();
	_destroy_canvas();
	_fini_theme();
	evas_object_del(menu_screen_info.win);
}



static void _pause_cb(void *data)
{
	_D("Pause start");

	if (vconf_set_int(VCONFKEY_IDLE_SCREEN_TOP, VCONFKEY_IDLE_SCREEN_TOP_FALSE) < 0) {
		_E("Failed to set %s to 0", VCONFKEY_IDLE_SCREEN_TOP);
	}

	menu_screen_info.state = APP_STATE_PAUSE;
}



static void _resume_cb(void *data)
{
	_D("START RESUME");

	if (vconf_set_int(VCONFKEY_IDLE_SCREEN_TOP, VCONFKEY_IDLE_SCREEN_TOP_TRUE) < 0) {
		_E("Failed to set %s to 1", VCONFKEY_IDLE_SCREEN_TOP);
	}

	utilx_hide_fake_effect(
		ecore_x_display_get(),
		ecore_x_window_root_get(ecore_evas_window_get(menu_screen_info.ee))
	);

	do { // Focus
		Evas_Object *layout = evas_object_data_get(menu_screen_info.win, "layout");
		break_if(NULL == layout);

		Evas_Object *all_apps = evas_object_data_get(layout, "all_apps");
		break_if(NULL == all_apps);

		Evas_Object *scroller = elm_object_part_content_get(all_apps, "content");
		break_if(NULL == scroller);

		page_scroller_focus(scroller);
	} while (0);

	menu_screen_info.state = APP_STATE_RESUME;
}



static void _service_cb(service_h service, void *data)
{
	_D("START RESET : %d", menu_screen_info.state);

	if (vconf_set_int(VCONFKEY_IDLE_SCREEN_TOP, VCONFKEY_IDLE_SCREEN_TOP_TRUE) < 0) {
		_E("Failed to set %s to 1", VCONFKEY_IDLE_SCREEN_TOP);
	}

	utilx_hide_fake_effect(
		ecore_x_display_get(),
		ecore_x_window_root_get(ecore_evas_window_get(menu_screen_info.ee))
	);

	do { // Focus
		Evas_Object *layout = evas_object_data_get(menu_screen_info.win, "layout");
		break_if(NULL == layout);

		Evas_Object *all_apps = evas_object_data_get(layout, "all_apps");
		break_if(NULL == all_apps);

		Evas_Object *scroller = elm_object_part_content_get(all_apps, "content");
		break_if(NULL == scroller);

		page_scroller_focus(scroller);
	} while (0);
}



static void _language_changed_cb(void *data)
{
	register unsigned int i;
	register unsigned int j;
	unsigned int count;
	Evas_Object *layout;
	Evas_Object *all_apps;
	Evas_Object *scroller;
	Evas_Object *page;
	Evas_Object *item;
	unsigned int page_max_app;

	_D("Language is changed");

	if (false == menu_screen_info.is_done) {
		elm_exit();
	}

	layout = evas_object_data_get(menu_screen_info.win, "layout");
	ret_if(NULL == layout);
	all_apps = evas_object_data_get(layout, "all_apps");
	ret_if(NULL == all_apps);
	scroller = elm_object_part_content_get(all_apps, "content");
	ret_if(NULL == scroller);

	count = page_scroller_count_page(scroller);
	page_max_app = (unsigned int) evas_object_data_get(scroller, "page_max_app");
	for (i = 0; i < count; i ++) {
		page = page_scroller_get_page_at(scroller, i);
		if (!page) continue;
		if (mapbuf_is_enabled(page)) {
			mapbuf_disable(page, 1);
		}

		for (j = 0; j < page_max_app; j ++) {
			ail_appinfo_h ai;
			char *name;

			item = page_get_item_at(page, j);
			if (!item) continue;

			if (ail_get_appinfo(item_get_package(item), &ai) < 0) continue;
			if (ail_appinfo_get_str(ai, AIL_PROP_NAME_STR, &name) < 0) {
				ail_destroy_appinfo(ai);
				continue;
			}

			if (!name) {
				_D("Failed to get name for %s", item_get_package(item));
				ail_destroy_appinfo(ai);
				continue;
			}

			item_set_name(item, name, 0);
			ail_destroy_appinfo(ai);
		}

		mapbuf_enable(page, 1);
	}
}



static void _init(app_event_callback_s *event_callback)
{
	event_callback->create = _create_cb;
	event_callback->terminate = _terminate_cb;
	event_callback->pause = _pause_cb;
	event_callback->resume = _resume_cb;
	event_callback->service = _service_cb;
	event_callback->low_memory = NULL;
	event_callback->low_battery = NULL;
	event_callback->device_orientation = NULL;
	event_callback->language_changed = _language_changed_cb;
	event_callback->region_format_changed = NULL;
}



static void _fini(void)
{
}



#define QP_EMUL_STR		"Emulator"
static bool _is_emulator_on(void)
{
	char *info;

	if (system_info_get_value_string(SYSTEM_INFO_KEY_MODEL, &info) == 0) {
		if (info == NULL) return false;
		if (!strncmp(QP_EMUL_STR, info, strlen(info))) {
			return true;
		}
	}

	return false;
}



int main(int argc, char *argv[])
{
	char *buf;
	app_event_callback_s event_callback;

	if (_is_emulator_on()) {
		_D("ELM_ENGINE is set as [software_x11]");
		setenv("ELM_ENGINE", "software_x11", 1);
	} else {
		buf = vconf_get_str(MENU_SCREEN_ENGINE);
		if (buf) {
			_D("ELM_ENGINE is set as [%s]", buf);
			setenv("ELM_ENGINE", buf, 1);
			free(buf);
		} else {
			_D("ELM_ENGINE is set as [gl]");
			setenv("ELM_ENGINE", "gl", 1);
		}
	}

	_init(&event_callback);
	app_efl_main(&argc, &argv, &event_callback, NULL);
	_fini();

	return EXIT_SUCCESS;
}





// End of a file
