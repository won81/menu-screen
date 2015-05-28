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



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <Elementary.h>

#include "menu_screen.h"
#include "conf.h"
#include "item.h"
#include "mouse.h"
#include "page.h"
#include "page_scroller.h"
#include "util.h"

#define CHAGNE_POSITION_TIME 0.1f
#define CHANGE_POSITION_COUNT 5
#define MOVE_THRESHOLD 10

#define VCONFKEY_APP_RELAY     "db/private/org.tizen.menu-screen/app_relay"

static struct {
	Ecore_Event_Handler *mouse_down;
	Ecore_Event_Handler *mouse_up;
	Ecore_Event_Handler *mouse_move;
	bool pressed;

	Evas_Coord pre_x;
	Evas_Coord pre_y;
	bool is_initialized;

	Evas_Coord down_x;
	Evas_Coord down_y;
	Evas_Coord move_x;
	Evas_Coord move_y;
} mouse_info = {
	.mouse_down = NULL,
	.mouse_up = NULL,
	.mouse_move = NULL,
	.pressed = false,

	.pre_x = 0,
	.pre_y = 0,
	.is_initialized = false,

	.down_x = 0,
	.down_y = 0,
	.move_x = 0,
	.move_y = 0,
};



HAPI bool mouse_is_scrolling(void)
{
	bool scroll_x = false;
	bool scroll_y = false;

	if (mouse_info.move_x > mouse_info.down_x + MOVE_THRESHOLD
		|| mouse_info.move_x < mouse_info.down_x - MOVE_THRESHOLD)
	{
		scroll_x = true;
	}

	if (mouse_info.move_y > mouse_info.down_y + MOVE_THRESHOLD
		|| mouse_info.move_y < mouse_info.down_y - MOVE_THRESHOLD)
	{
		scroll_y = true;
	}

	return scroll_x || scroll_y;
}

static int _set_app_relay_value(int b_val)
{
	if (vconf_set_bool(VCONFKEY_APP_RELAY, b_val))
	{
		_D("APP_RELAY: Fail to set VCONFKEY_APP_RELAY\n");
		return -1;
	}

	return 0;
}

static int _get_app_relay_value(int *b_val)
{
	if (vconf_get_bool(VCONFKEY_APP_RELAY, b_val))
	{
		_D("APP_RELAY: Fail to get VCONFKEY_APP_RELAY\n");

		if (vconf_set_bool(VCONFKEY_APP_RELAY, FALSE))
		{
			_D("APP_RELAY: Fail to set MP_VCONFKEY_MUSIC_SHUFFLE\n");
			return -1;
		}
		*b_val = FALSE;
	}
	return 0;
}

static Eina_Bool _down_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Button *move = event;

	retv_if(true == mouse_info.pressed, ECORE_CALLBACK_RENEW);

	_D("Mouse down (%d,%d)", move->root.x, move->root.y);

	if ((move->root.x > 900 && move->root.x < 1000) &&
			(move->root.y > 0 && move->root.y < 50)) {
		int b_val;

		_get_app_relay_value(&b_val);
		_set_app_relay_value((b_val == 0)? TRUE : FALSE);
		_D("APP_RELAY: Trigger App Relay\n");
	}

	mouse_info.pressed = true;
	mouse_info.is_initialized = false;

	mouse_info.down_x = move->root.x;
	mouse_info.down_y = move->root.y;

	return ECORE_CALLBACK_RENEW;
}



static Eina_Bool _up_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Button *move = event;

	_D("Mouse up (%d,%d)", move->root.x, move->root.y);

	retv_if(mouse_info.pressed == false, ECORE_CALLBACK_RENEW);
	mouse_info.pressed = false;
	mouse_info.pre_x = 0;
	mouse_info.pre_y = 0;

	return ECORE_CALLBACK_RENEW;
}



static Eina_Bool _move_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Move *move = event;

	mouse_info.move_x = move->root.x;
	mouse_info.move_y = move->root.y;

	if (mouse_info.pressed == false) {
		return ECORE_CALLBACK_RENEW;
	}

	return ECORE_CALLBACK_RENEW;
}



HAPI void mouse_register(void)
{
	mouse_info.mouse_down = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, _down_cb, NULL);
	if (!mouse_info.mouse_down) {
		_D("Failed to register the mouse down event callback");
	}

	mouse_info.mouse_move = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _move_cb, NULL);
	if (!mouse_info.mouse_move) {
		_D("Failed to register the mouse move event callback");
		ecore_event_handler_del(mouse_info.mouse_down);
		mouse_info.mouse_down = NULL;
	}

	mouse_info.mouse_up = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, _up_cb, NULL);
	if (!mouse_info.mouse_up) {
		_D("Failed to register the mouse up event callback");
		ecore_event_handler_del(mouse_info.mouse_down);
		ecore_event_handler_del(mouse_info.mouse_move);

		mouse_info.mouse_down = NULL;
		mouse_info.mouse_move = NULL;
	}
}



HAPI void mouse_unregister(void)
{
	if (mouse_info.mouse_down) {
		ecore_event_handler_del(mouse_info.mouse_down);
		mouse_info.mouse_down = NULL;
	}

	if (mouse_info.mouse_up) {
		ecore_event_handler_del(mouse_info.mouse_up);
		mouse_info.mouse_up = NULL;
	}

	if (mouse_info.mouse_move) {
		ecore_event_handler_del(mouse_info.mouse_move);
		mouse_info.mouse_move = NULL;
	}
}



// End of a file
