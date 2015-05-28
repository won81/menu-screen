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

#include "conf.h"
#include "item.h"
#include "menu_screen.h"
#include "pkgmgr.h"
#include "util.h"
#include "all_apps/shortcut.h"

#define BUFSZE 1024



static struct {
	Evas_Object *popup;
} popup_info = {
	.popup = NULL,
};



HAPI Evas_Object *popup_exist(void)
{
	return popup_info.popup;
}



HAPI void popup_destroy_all(void)
{
	void (*_destroy_popup)(void *data, Evas_Object *obj, void *event_info);

	if (NULL == popup_info.popup) return;

	_destroy_popup = evas_object_data_get(popup_info.popup, "func_destroy_popup");
	if (_destroy_popup) _destroy_popup(popup_info.popup, NULL, NULL);

	popup_info.popup = NULL;
}



static void _response_cb(void *data, Evas_Object *obj, void *event_info)
{
	ret_if(NULL == data);

	Evas_Object *popup = data;
	evas_object_data_del(popup, "func_destroy_popup");
	popup_info.popup = NULL;

	evas_object_del(evas_object_data_del(popup, "button"));
	evas_object_del(popup);
}



HAPI Evas_Object *popup_create_confirm(Evas_Object *parent, const char *warning)
{
	Evas_Object *popup;
	Evas_Object *btn;

	retv_if(NULL == warning, NULL);

	popup_destroy_all();

	popup = elm_popup_add(parent);
	retv_if(NULL == popup, NULL);

	btn = elm_button_add(popup);
	if (NULL == btn) {
		evas_object_del(popup);
		return NULL;
	}

	elm_object_text_set(btn, D_("IDS_COM_SK_OK"));
	evas_object_data_set(popup, "button", btn);

	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_cb, popup);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_text_set(popup, "title,text", D_("IDS_COM_POP_WARNING"));
	elm_object_text_set(popup, warning);
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
	evas_object_show(popup);

	evas_object_data_set(popup, "func_destroy_popup", _response_cb);
	popup_info.popup = popup;

	return popup;
}



static void _uninstall_no_cb(void *data, Evas_Object *obj, void *event_info)
{
	ret_if(NULL == data);

	Evas_Object *popup = data;
	evas_object_data_del(popup, "func_destroy_popup");
	popup_info.popup = NULL;

	evas_object_del(evas_object_data_del(popup, "button1"));
	evas_object_del(evas_object_data_del(popup, "button2"));
	evas_object_data_del(popup, "item");
	evas_object_del(popup);
}



static void _uninstall_yes_cb(void *data, Evas_Object *obj, void *event_info)
{
	ret_if(NULL == data);

	Evas_Object *popup = data;
	evas_object_data_del(popup, "func_destroy_popup");
	popup_info.popup = NULL;

	Evas_Object *item;
	item = evas_object_data_del(popup, "item");

	evas_object_del(evas_object_data_del(popup, "button1"));
	evas_object_del(evas_object_data_del(popup, "button2"));
	evas_object_del(popup);

	bool is_shortcut = false;
	is_shortcut = (bool) evas_object_data_get(item, "is_shortcut");

	if (is_shortcut) {
		all_apps_shortcut_remove(item);
	} else {
		if (MENU_SCREEN_ERROR_OK != pkgmgr_uninstall(item)) {
			_E("Cannot communicate with the pkgmgr-server.");
		}
	}
}



#define IDS_AT_POP_UNINSTALL_PS_Q "IDS_AT_POP_UNINSTALL_PS_Q"
HAPI Evas_Object *popup_create_uninstall(Evas_Object *parent, Evas_Object *item)
{
	Evas_Object *popup;
	Evas_Object *btn1;
	Evas_Object *btn2;
	char warning[BUFSZE];

	popup_destroy_all();

	popup = elm_popup_add(parent);
	retv_if(NULL == popup, NULL);

	evas_object_data_set(popup, "item", item);

	btn1 = elm_button_add(popup);
	if (NULL == btn1) {
		evas_object_del(popup);
		return NULL;
	}
	elm_object_style_set(btn1, "popup_button/default");
	elm_object_text_set(btn1, D_("IDS_COM_SK_CANCEL"));
	evas_object_data_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _uninstall_no_cb, popup);

	btn2 = elm_button_add(popup);
	if (NULL == btn2) {
		evas_object_del(popup);
		return NULL;
	}
	elm_object_style_set(btn2, "popup_button/default");
	elm_object_text_set(btn2, D_("IDS_COM_SK_OK"));
	evas_object_data_set(popup, "button2", btn2);
	elm_object_part_content_set(popup, "button2", btn2);
	evas_object_smart_callback_add(btn2, "clicked", _uninstall_yes_cb, popup);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_data_set(popup, "func_destroy_popup", _uninstall_no_cb);
	popup_info.popup = popup;
	evas_object_show(popup);

	char *name= item_get_name(item);
	retv_if(NULL == name, popup);

	char *markup_name = elm_entry_utf8_to_markup(name);
	retv_if(NULL == markup_name, popup);

	snprintf(warning, sizeof(warning), _(IDS_AT_POP_UNINSTALL_PS_Q), markup_name);
	free(markup_name);

	elm_object_text_set(popup, warning);

	return popup;
}



// End of a file
