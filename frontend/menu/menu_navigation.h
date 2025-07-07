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

#ifndef _MENU_NAVIGATION_H
#define _MENU_NAVIGATION_H

#include "menu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void menu_navigation_clear(menu_handle_t *menu, bool pending_push);

void menu_navigation_decrement(menu_handle_t *menu);

void menu_navigation_increment(menu_handle_t *menu);

void menu_navigation_set(menu_handle_t *menu, size_t i);

void menu_navigation_set_last(menu_handle_t *menu);

void menu_navigation_descend_alphabet(menu_handle_t *menu, size_t *ptr_out);

void menu_navigation_ascend_alphabet(menu_handle_t *menu, size_t *ptr_out);

#ifdef __cplusplus
}
#endif

#endif
