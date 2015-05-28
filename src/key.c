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



#include <stdlib.h>
#include <Elementary.h>
#include <utilX.h>
#include <Ecore_X.h>

#include "conf.h"
#include "key.h"
#include "menu_screen.h"
#include "page_scroller.h"
#include "popup.h"
#include "util.h"
#include "all_apps/layout.h"



static struct {
	Eina_Bool pressed;
	Ecore_Event_Handler *press_handler;
	Ecore_Event_Handler *release_handler;
	Eina_Bool register_handler;
	Ecore_Timer *long_press;
	Eina_Bool home_grabbed;
} key_info = {
	.pressed = 0,
	.press_handler = NULL,
	.release_handler = NULL,
	.register_handler = EINA_FALSE,
	.long_press = NULL,
	.home_grabbed = EINA_FALSE,
};



#define KEY_LEFT "Left"
#define KEY_RIGHT "Right"
#define KEY_UP "Up"
#define KEY_DOWN "Down"
static Eina_Bool _key_release_cb(void *data, int type, void *event)
{
	Evas_Event_Key_Up *ev = event;

	retv_if(EINA_FALSE == key_info.register_handler, ECORE_CALLBACK_CANCEL);
	retv_if(NULL == ev, ECORE_CALLBACK_CANCEL);

	_D("Key(%s) released %d", ev->keyname, key_info.pressed);

	if (key_info.pressed == EINA_FALSE) return ECORE_CALLBACK_CANCEL;

	do {
		Evas_Object *win;
		win = menu_screen_get_win();
		break_if(NULL == win);

		Evas_Object *layout;
		layout = evas_object_data_get(win, "layout");
		break_if(NULL == layout);

		Evas_Object *all_apps;
		all_apps = evas_object_data_get(layout, "all_apps");
		break_if(NULL == all_apps);

		Evas_Object *scroller = evas_object_data_get(all_apps, "scroller");
		break_if(NULL == scroller);

		if (!strcmp(ev->keyname, KEY_SELECT) || !strcmp(ev->keyname, KEY_BACK)) {
			if (popup_exist()) {
				popup_destroy_all();
				break;
			}

			if (all_apps_layout_is_edited(all_apps)) {
				all_apps_layout_unedit(all_apps);
			}
		} else if (!strcmp(ev->keyname, KEY_LEFT) ||
					!strcmp(ev->keyname, KEY_RIGHT) ||
					!strcmp(ev->keyname, KEY_UP) ||
					!strcmp(ev->keyname, KEY_DOWN))
		{
			int cur_idx = page_scroller_get_current_page_no(scroller);
			int idx = 0, x = 0, w = 0;
			elm_scroller_region_get(scroller, &x, NULL, &w, NULL);

			if (w) idx = x / w;
			if (cur_idx != idx) {
				page_scroller_bring_in(scroller, idx);
				break;
			}

			/* If there are no items to be focused after pressing keys,
			   Menu-screen forces to focus the first item of the other page */
			int rest = x % w;
			if (rest) {
				page_scroller_focus_into_vector(scroller, rest > w / 2 ? -1 : 1);
			}
		}
	} while (0);

	key_info.pressed = EINA_FALSE;

	return ECORE_CALLBACK_CANCEL;
}



static Eina_Bool _key_press_cb(void *data, int type, void *event)
{
	Evas_Event_Key_Down *ev = event;

	retv_if(EINA_FALSE == key_info.register_handler, ECORE_CALLBACK_CANCEL);
	retv_if(NULL == ev, ECORE_CALLBACK_CANCEL);

	key_info.pressed = EINA_TRUE;
	_D("Key pressed %d", key_info.pressed);

	return ECORE_CALLBACK_CANCEL;
}



HAPI void key_register(void)
{
	if (!key_info.release_handler) {
		key_info.release_handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, _key_release_cb, NULL);
		if (!key_info.release_handler) {
			_E("Failed to register a key up event handler");
		}
	}

	if (!key_info.press_handler) {
		key_info.press_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_press_cb, NULL);
		if (!key_info.press_handler) {
			_E("Failed to register a key down event handler");
		}
	}

	key_info.pressed = EINA_FALSE;
	key_info.register_handler = EINA_TRUE;
}



HAPI void key_unregister(void)
{
	if (key_info.long_press) {
		ecore_timer_del(key_info.long_press);
		key_info.long_press = NULL;
	}

	if (key_info.release_handler) {
		ecore_event_handler_del(key_info.release_handler);
		key_info.release_handler = NULL;
	}

	if (key_info.press_handler) {
		ecore_event_handler_del(key_info.press_handler);
		key_info.press_handler = NULL;
	}

	key_info.register_handler = EINA_FALSE;
}



HAPI void key_grab_home(void)
{
	if (EINA_TRUE == key_info.home_grabbed) return;

	Ecore_X_Window win = elm_win_xwindow_get(menu_screen_get_win());
	Display* dpy = ecore_x_display_get();

	int ret = utilx_grab_key(dpy, win, KEY_SELECT, TOP_POSITION_GRAB);
	ret_if(0 != ret);

	ret = utilx_grab_key(dpy, win, KEY_BACK, TOP_POSITION_GRAB);
	ret_if(0 != ret);

	key_info.home_grabbed = EINA_TRUE;
}



HAPI void key_ungrab_home(void)
{
	if (key_info.home_grabbed == EINA_FALSE) return;

	Ecore_X_Window win = elm_win_xwindow_get(menu_screen_get_win());
	Display* dpy = ecore_x_display_get();

	int ret = utilx_ungrab_key(dpy, win, KEY_SELECT);
	ret_if(0 != ret);

	ret = utilx_ungrab_key(dpy, win, KEY_BACK);
	ret_if(0 != ret);

	key_info.home_grabbed = EINA_FALSE;
}



// End of a file
