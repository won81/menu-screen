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



#include <Evas.h>

#include "util.h"



HAPI void _evas_object_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	_D("%s is resized to (%d, %d, %d, %d)", data, x, y, w, h);
}



HAPI void _evas_object_event_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_D("%s IS REMOVED!", (const char *) data);
}



HAPI void _evas_object_event_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	_D("%s's GEOMETRY : [%d, %d, %d, %d]", (const char *) data, x, y, w, h);
}



HAPI void _evas_object_event_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x;
	Evas_Coord y;
	Evas_Coord w;
	Evas_Coord h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);
	_D("%s's GEOMETRY : [%d, %d, %d, %d]", (const char *) data, x, y, w, h);
}



// End of a file
