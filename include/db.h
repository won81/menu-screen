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

#ifndef __MENU_SCREEN_DB_H__
#define __MENU_SCREEN_DB_H__

#include <stdbool.h>

#include "util.h"

typedef struct stmt stmt_h;

extern stmt_h *db_prepare(const char *query);
extern menu_screen_error_e db_bind_bool(stmt_h *handle, int idx, bool value);
extern menu_screen_error_e db_bind_int(stmt_h *handle, int idx, int value);
extern menu_screen_error_e db_bind_str(stmt_h *handle, int idx, const char *str);
extern menu_screen_error_e db_next(stmt_h *handle);
extern bool db_get_bool(stmt_h *handle, int index);
extern int db_get_int(stmt_h *handle, int index);
extern long long db_get_long_long(stmt_h *handle, int index);
extern const char *db_get_str(stmt_h *handle, int index);
extern menu_screen_error_e db_reset(stmt_h *handle);
extern menu_screen_error_e db_finalize(stmt_h *handle);
extern menu_screen_error_e db_exec(const char *query);
extern long long db_last_insert_rowid(void);

extern menu_screen_error_e db_open(const char *db_file);
extern void db_close(void);

extern menu_screen_error_e db_begin_transaction(void);
extern menu_screen_error_e db_end_transaction(bool success);

#endif // __MENU_SCREEN_DB_H__
// End of file
