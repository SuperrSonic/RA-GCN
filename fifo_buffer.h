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

#ifndef __FIFO_BUFFER_H
#define __FIFO_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

struct fifo_buffer
{
   uint8_t *buffer;
   size_t bufsize;
   size_t first;
   size_t end;
};

typedef struct fifo_buffer fifo_buffer_t;

fifo_buffer_t *fifo_new(size_t size);

void fifo_write(fifo_buffer_t *buffer, const void *in_buf, size_t size);

void fifo_read(fifo_buffer_t *buffer, void *in_buf, size_t size);

void fifo_free(fifo_buffer_t *buffer);

size_t fifo_read_avail(fifo_buffer_t *buffer);

size_t fifo_write_avail(fifo_buffer_t *buffer);

#ifdef __cplusplus
}
#endif

#endif

