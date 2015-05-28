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



#ifndef __MENU_SCREEN_LAYOUT_H__
#define __MENU_SCREEN_LAYOUT_H__

#include <Evas.h>
#include "util.h"

extern Evas_Object *layout_create(Evas_Object *conformant, const char *file, const char *group, int rotate);
extern void layout_destroy(Evas_Object *layout);

extern void layout_enable_block(Evas_Object *layout);
extern void layout_disable_block(Evas_Object *layout);

extern Evas_Object* layout_load_edj(Evas_Object *parent, const char *edjname, const char *grpname);
extern void layout_unload_edj(Evas_Object *layout);

#endif //__MENU_SCREEN_LAYOUT_H__

// End of a file
