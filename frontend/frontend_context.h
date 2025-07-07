/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2014 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __FRONTEND_CONTEXT_H
#define __FRONTEND_CONTEXT_H

#include <stddef.h>
#include <boolean.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#if defined(ANDROID)
#include "platform/platform_android.h"
#define main_entry android_app_entry
#define args_type() struct android_app*
#define signature() void* data
#define signature_expand() data
#define returntype void
#else
#if defined(__APPLE__) || defined(HAVE_BB10) || defined(EMSCRIPTEN)
#define main_entry rarch_main
#else
#define main_entry main
#endif
#define args_type() void*
#define signature() int argc, char *argv[]
#define signature_expand() argc, argv
#define returntype int
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*environment_get_t)(int *argc, char *argv[], void *args,
   void *params_data);
typedef void (*process_args_t)(int *argc, char *argv[]);

typedef struct frontend_ctx_driver
{
   environment_get_t environment_get;
   void (*init)(void *data);
   void (*deinit)(void *data);
   void (*exitspawn)(char *core_path, size_t sizeof_core_path);

   process_args_t process_args;
   int  (*process_events)(void *data);
   void (*exec)(const char *, bool);
   void (*set_fork)(bool exitspawn, bool start_game);
   void (*shutdown)(bool);
   void (*get_name)(char *, size_t);
   int  (*get_rating)(void);
   void (*content_loaded)(void);

   const char *ident;

   const struct video_driver *(*get_video_driver)(void);
} frontend_ctx_driver_t;

extern const frontend_ctx_driver_t frontend_ctx_gx;
extern const frontend_ctx_driver_t frontend_ctx_ps3;
extern const frontend_ctx_driver_t frontend_ctx_xdk;
extern const frontend_ctx_driver_t frontend_ctx_qnx;
extern const frontend_ctx_driver_t frontend_ctx_apple;
extern const frontend_ctx_driver_t frontend_ctx_android;
extern const frontend_ctx_driver_t frontend_ctx_psp;
extern const frontend_ctx_driver_t frontend_ctx_null;

/* Finds driver with ident. Does not initialize. */
const frontend_ctx_driver_t *frontend_ctx_find_driver(const char *ident);

/* Finds first suitable driver and initialize. */
const frontend_ctx_driver_t *frontend_ctx_init_first(void);

int main_entry_decide(signature(), args_type() args);
int main_entry_iterate_clear_input(signature(), args_type() args);
int main_entry_iterate_load_content(signature(), args_type() args);
int main_entry_iterate_content(signature(), args_type() args);
#ifdef HAVE_MENU
int main_entry_iterate_menu_preinit(signature(), args_type() args);
int main_entry_iterate_menu(signature(), args_type() args);
#endif

#ifdef __cplusplus
}
#endif

#endif
