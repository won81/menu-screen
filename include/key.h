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

#ifndef __MENU_SCREEN_INPUT_KEY_H__
#define __MENU_SCREEN_INPUT_KEY_H__

#include "util.h"

HAPI void key_register(void);
HAPI void key_unregister(void);
HAPI void key_grab_home(void);
HAPI void key_ungrab_home(void);

#endif //__MENU_SCREEN_INPUT_KEY_H__

// End of a file
