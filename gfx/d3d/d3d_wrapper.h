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

#ifndef _D3D_WRAPPER_H
#define _D3D_WRAPPER_H

#include "../context/win32_common.h"
#include "d3d_defines.h"

void d3d_swap(void *data, LPDIRECT3DDEVICE dev);

LPDIRECT3DVERTEXBUFFER d3d_vertex_buffer_new(LPDIRECT3DDEVICE dev,
      unsigned length, unsigned usage, unsigned fvf,
      D3DPOOL pool, void *handle);

void *d3d_vertex_buffer_lock(LPDIRECT3DVERTEXBUFFER vertbuf);
void d3d_vertex_buffer_unlock(LPDIRECT3DVERTEXBUFFER vertbuf);

void d3d_vertex_buffer_free(LPDIRECT3DVERTEXBUFFER buf);

LPDIRECT3DTEXTURE d3d_texture_new(LPDIRECT3DDEVICE dev,
      const char *path, unsigned width, unsigned height,
      unsigned miplevels, unsigned usage, D3DFORMAT format,
      D3DPOOL pool, unsigned filter, unsigned mipfilter,
      D3DCOLOR color_key, D3DXIMAGE_INFO *src_info, 
      PALETTEENTRY *palette);

void d3d_set_stream_source(LPDIRECT3DDEVICE dev, unsigned stream_no,
      LPDIRECT3DVERTEXBUFFER stream_vertbuf, unsigned offset_bytes,
      unsigned stride);

void d3d_texture_free(LPDIRECT3DTEXTURE tex);

void d3d_set_transform(LPDIRECT3DDEVICE dev,
      D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX *matrix);

void d3d_set_sampler_address_u(LPDIRECT3DDEVICE dev,
      unsigned sampler, unsigned value);

void d3d_set_sampler_address_v(LPDIRECT3DDEVICE dev,
      unsigned sampler, unsigned value);

void d3d_set_sampler_minfilter(LPDIRECT3DDEVICE dev,
      unsigned sampler, unsigned value);

void d3d_set_sampler_magfilter(LPDIRECT3DDEVICE dev,
      unsigned sampler, unsigned value);

void d3d_draw_primitive(LPDIRECT3DDEVICE dev,
      D3DPRIMITIVETYPE type, unsigned start, unsigned count);

void d3d_clear(LPDIRECT3DDEVICE dev,
      unsigned count, const D3DRECT *rects, unsigned flags,
      D3DCOLOR color, float z, unsigned stencil);

void d3d_lockrectangle_clear(void *data, 
      LPDIRECT3DTEXTURE tex,
      unsigned level, D3DLOCKED_RECT *lock_rect, RECT *rect,
      unsigned flags);

void d3d_set_texture(LPDIRECT3DDEVICE dev, unsigned sampler,
      LPDIRECT3DTEXTURE tex);

HRESULT d3d_set_vertex_shader(LPDIRECT3DDEVICE dev, unsigned index,
      void *data);

void d3d_texture_blit(void *data, void *renderchain_data,
      LPDIRECT3DTEXTURE tex,
      D3DLOCKED_RECT *lr, const void *frame,
      unsigned width, unsigned height, unsigned pitch);

void d3d_set_viewport(LPDIRECT3DDEVICE dev, D3DVIEWPORT *vp);

#endif
