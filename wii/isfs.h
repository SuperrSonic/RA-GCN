/****************************************************************************
 * Copyright (C) 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
 * Copyright (C) 2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef _LIBISFS_H
#define _LIBISFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ogc/isfs.h>

#define ISFS_MAXPATHLEN (ISFS_MAXPATH + 1)

bool ISFS_Mount(s32 read_only);
bool ISFS_IsMounted();
bool ISFS_Unmount();

#ifdef __cplusplus
}
#endif

#endif /* _LIBISFS_H */
