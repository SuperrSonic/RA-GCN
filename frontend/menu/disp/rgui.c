/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2014 - Daniel De Matteis
 *  Copyright (C) 2012-2014 - Michael Lelli
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

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "../menu_driver.h"
#include "../menu_list.h"
#include "../menu_common.h"
#include "../../../general.h"
#include "../../../config.def.h"
#include "../../../dynamic.h"
#include <compat/posix_string.h>
#include <file/file_path.h>

#include "../../../settings_data.h"
#include "../../../gfx/fonts/bitmap.h"

#ifdef USE_ESP
#include "shared_esp.h"
#else
#include "shared.h"
#endif

typedef struct rgui_handle
{
   uint16_t *menu_framebuf;
} rgui_handle_t;

//bool snow_enable = true;
//float snowflakes[1024];

enum rgui_particle_animation_effect
{
   RGUI_PARTICLE_EFFECT_NONE = 0,
   RGUI_PARTICLE_EFFECT_SNOW,
   RGUI_PARTICLE_EFFECT_SNOW_ALT,
   RGUI_PARTICLE_EFFECT_RAIN,
   RGUI_PARTICLE_EFFECT_VORTEX,
   RGUI_PARTICLE_EFFECT_STARFIELD,
   RGUI_PARTICLE_EFFECT_LAST
};

#define NUM_PARTICLES 256

#ifndef PI
#define PI 3.14159265359f
#endif

unsigned particle_effect = RGUI_PARTICLE_EFFECT_NONE; //RGUI_PARTICLE_EFFECT_SNOW;
uint16_t particle_color = 55000; //0xFFFF;
bool fb_size_changed = false;

#define DEFAULT_RGUI_PARTICLE_EFFECT_SPEED 1.0f

/* Particle effect animations update at a base rate
 * of 60Hz (-> 16.666 ms update period) */
//static const float particle_effect_period = (1.0f / 60.0f) * 1000.0f;

/* A 'particle' is just 4 float variables that can
 * be used for any purpose - e.g.:
 * > a = x pos
 * > b = y pos
 * > c = x velocity
 * or:
 * > a = radius
 * > b = theta
 * etc. */
typedef struct
{
   float a;
   float b;
   float c;
   float d;
} rgui_particle_t;

static rgui_particle_t particles[NUM_PARTICLES] = {{ 0.0f }};

#define RGUI_TERM_START_X (driver.menu->width / 21)
#define RGUI_TERM_START_Y (driver.menu->height / 9)
#define RGUI_TERM_WIDTH (((driver.menu->width - RGUI_TERM_START_X - RGUI_TERM_START_X) / (FONT_WIDTH_STRIDE)))
#define RGUI_TERM_HEIGHT (((driver.menu->height - RGUI_TERM_START_Y - RGUI_TERM_START_X) / (FONT_HEIGHT_STRIDE)) - 1)

static void rgui_copy_glyph(uint8_t *glyph, const uint8_t *buf)
{
   int y, x;
   for (y = 0; y < FONT_HEIGHT; y++)
   {
      for (x = 0; x < FONT_WIDTH; x++)
      {
         uint32_t col =
            ((uint32_t)buf[3 * (-y * 256 + x) + 0] << 0) |
            ((uint32_t)buf[3 * (-y * 256 + x) + 1] << 8) |
            ((uint32_t)buf[3 * (-y * 256 + x) + 2] << 16);

         uint8_t rem = 1 << ((x + y * FONT_WIDTH) & 7);
         unsigned offset = (x + y * FONT_WIDTH) >> 3;

         if (col != 0xff)
            glyph[offset] |= rem;
      }
   }
}

static uint16_t gray_filler(unsigned x, unsigned y)
{
   x >>= 1;
   y >>= 1;
   unsigned col = ((x + y) & 1) + 1;
#if defined(GEKKO) || defined(PSP)
   return (6 << 12) | (col << 8) | (col << 4) | (col << 0);
#else
   return (col << 13) | (col << 9) | (col << 5) | (12 << 0);
#endif
}

static uint16_t green_filler(unsigned x, unsigned y)
{
   x >>= 1;
   y >>= 1;
   unsigned col = ((x + y) & 1) + 1;
#if defined(GEKKO) || defined(PSP)
   return (6 << 12) | (col << 8) | (col << 4) | (col << 1);
#else
   return (col << 13) | (col << 10) | (col << 5) | (12 << 0);
#endif
}

static uint16_t custom_filler(unsigned x, unsigned y)
{
   //ZSNES BG
   if(g_settings.theme_preset == 7) {
	   if(!g_extern.libretro_dummy)
		   return 18527;
	   return 41136;
   }
   //better opacity 18527
	   //return 17464; //with opacity looks bad

   return g_settings.menu_bg_clr;
}

static uint16_t custom_msg_filler(unsigned x, unsigned y)
{
   return g_settings.menu_msg_clr;
}

static void fill_rect(uint16_t *buf, unsigned pitch,
      unsigned x, unsigned y,
      unsigned width, unsigned height,
      uint16_t (*col)(unsigned x, unsigned y))
{
   unsigned j, i;
   for (j = y; j < y + height; j++)
      for (i = x; i < x + width; i++)
         buf[j * (pitch >> 1) + i] = col(i, j);
}

static void blit_line(int x, int y, const char *message, bool green)
{
   int j, i;
   unsigned hov_col = 32767;
   unsigned tex_col = 54965;

   if (!driver.menu)
      return;

   if (g_settings.theme_preset == 0) {
	  /* Type 0 is default, custom values */
      hov_col = g_settings.hover_color;
      tex_col = g_settings.text_color;
   } else if (g_settings.theme_preset == 1) {
	   /* Type 1 is green/white */
	   hov_col = 46060;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 2) {
	   /* Type 2 is mute red/white */
	   hov_col = 60647;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 3) {
       /* Type 3 is yellow/white */
	   hov_col = 64450;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 4) {
       /* Type 4 is pink/white */
	   hov_col = 64927;
	   tex_col = 32767;
   } else if (g_settings.theme_preset == 5) {
	   /* Type 5 is white/gray */
	   hov_col = 32767;
	   tex_col = 54965;
   } else if (g_settings.theme_preset == 6) {
	   /* Type 6 is cyan/darkblue */
	   hov_col = 46046;
	   tex_col = 35351;
   } else if (g_settings.theme_preset == 7) {
	   /* Type 7 is zsnes */
	   hov_col = 32767;
	   tex_col = 54965;
   } else if (g_settings.theme_preset == 8) {
	   /* Type 8 is red/gold */
	   hov_col = 64512;
	   tex_col = 60071;
   }
   //znes
   //issue: alpha makes dark image so I'm using two values to get closer to original.
   //BG = 17464 = 0x402c80 with 60% alpha bad
   //hov= 62396
   //tex= 44362

   while (*message)
   {
      for (j = 0; j < FONT_HEIGHT; j++)
      {
         for (i = 0; i < FONT_WIDTH; i++)
         {
            uint8_t rem = 1 << ((i + j * FONT_WIDTH) & 7);
            int offset = (i + j * FONT_WIDTH) >> 3;
            bool col = (driver.menu->font[FONT_OFFSET
                  ((unsigned char)*message) + offset] & rem);

            if (col)
            {
               driver.menu->frame_buf[(y + j) *
                  (driver.menu->frame_buf_pitch >> 1) + (x + i)] = green ?
#if defined(GEKKO)|| defined(PSP)
               hov_col : tex_col;
#else
               (15 << 0) | (7 << 4) | (15 << 8) | (7 << 12) : 0xFFFF;
#endif
            }
         }
      }

      x += FONT_WIDTH_STRIDE;
      message++;
   }
}

static bool init_font(menu_handle_t *menu, const uint8_t *font_bmp_buf)
{
   unsigned i;
   uint8_t *font = (uint8_t *) calloc(1, FONT_OFFSET(256));

   if (!font)
   {
      RARCH_ERR("Font memory allocation failed.\n");
      return false;
   }

   menu->alloc_font = true;
   for (i = 0; i < 256; i++)
   {
      unsigned y = i / 16;
      unsigned x = i % 16;
      rgui_copy_glyph(&font[FONT_OFFSET(i)],
            font_bmp_buf + 54 + 3 * (256 * (255 - 16 * y) + 16 * x));
   }

   menu->font = font;
   return true;
}

static bool rguidisp_init_font(void *data)
{
   menu_handle_t *menu = (menu_handle_t*)data;

   const uint8_t *font_bmp_buf = NULL;
   const uint8_t *font_bin_buf = bitmap_bin;

   if (font_bmp_buf)
      return init_font(menu, font_bmp_buf);
   else if (font_bin_buf)
      menu->font = font_bin_buf;
   else
      return false;

   return true;
}

/* Returns true if particle is on screen */
static bool INLINE rgui_draw_particle(
      uint16_t *data,
      unsigned fb_width, unsigned fb_height,
      int x, int y,
      unsigned width, unsigned height,
      uint16_t color)
{
   unsigned x_index, y_index;

   /* This great convoluted mess just saves us
    * having to perform comparisons on every
    * iteration of the for loops... */
   int x_start = x > 0 ? x : 0;
   int y_start = y > 0 ? y : 0;
   int x_end = x + width;
   int y_end = y + height;

   x_start = x_start <= fb_width  ? x_start : fb_width;
   y_start = y_start <= fb_height ? y_start : fb_height;

   x_end = x_end >  0        ? x_end : 0;
   x_end = x_end <= fb_width ? x_end : fb_width;

   y_end = y_end >  0         ? y_end : 0;
   y_end = y_end <= fb_height ? y_end : fb_height;

   for (x_index = (unsigned)x_start; x_index < (unsigned)x_end; x_index++)
      for (y_index = (unsigned)y_start; y_index < (unsigned)y_end; y_index++)
         data[(y_index * fb_width) + x_index] = color;

   return (x_end > x_start) && (y_end > y_start);
}

static void rgui_init_particle_effect(rgui_handle_t *rgui)
{
   //size_t fb_pitch = driver.menu->frame_buf_pitch;
   unsigned fb_width = driver.menu->width, fb_height = driver.menu->height;
   size_t i;

   /* Sanity check */
   if (!rgui)
      return;

   //menu_display_get_fb_size(&fb_width, &fb_height, &fb_pitch);

   switch (particle_effect)
   {
      case RGUI_PARTICLE_EFFECT_SNOW:
      case RGUI_PARTICLE_EFFECT_SNOW_ALT:
         {
            for (i = 0; i < NUM_PARTICLES; i++)
            {
               rgui_particle_t *particle = &particles[i];

               particle->a = (float)(rand() % fb_width);
               particle->b = (float)(rand() % fb_height);
               particle->c = (float)(rand() % 64 - 16) * 0.1f;
               particle->d = (float)(rand() % 64 - 48) * 0.1f;
            }
         }
         break;
      case RGUI_PARTICLE_EFFECT_RAIN:
         {
            uint8_t weights[] = { /* 60 entries */
               2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
               3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
               4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
               5, 5, 5, 5, 5, 5, 5, 5,
               6, 6, 6, 6, 6, 6,
               7, 7, 7, 7,
               8, 8, 8,
               9, 9,
               10};
            unsigned num_drops = (unsigned)(0.85f * ((float)fb_width / 426.0f) * (float)NUM_PARTICLES);

            num_drops = num_drops < NUM_PARTICLES ? num_drops : NUM_PARTICLES;

            for (i = 0; i < num_drops; i++)
            {
               rgui_particle_t *particle = &particles[i];

               /* x pos */
               particle->a = (float)(rand() % (fb_width / 3)) * 3.0f;
               /* y pos */
               particle->b = (float)(rand() % fb_height);
               /* drop length */
               particle->c = (float)weights[(unsigned)(rand() % 60)];
               /* drop speed (larger drops fall faster) */
               particle->d = (particle->c / 12.0f) * (0.5f + ((float)(rand() % 150) / 200.0f));
            }
         }
         break;
      case RGUI_PARTICLE_EFFECT_VORTEX:
         {
            float max_radius         = (float)sqrt((double)((fb_width * fb_width) + (fb_height * fb_height))) / 2.0f;
            float one_degree_radians = PI / 360.0f;

            for (i = 0; i < NUM_PARTICLES; i++)
            {
               rgui_particle_t *particle = &particles[i];

               /* radius */
               particle->a = 1.0f + (((float)rand() / (float)RAND_MAX) * max_radius);
               /* theta */
               particle->b = ((float)rand() / (float)RAND_MAX) * 2.0f * PI;
               /* radial speed */
               particle->c = (float)((rand() % 100) + 1) * 0.001f;
               /* rotational speed */
               particle->d = (((float)((rand() % 50) + 1) / 200.0f) + 0.1f) * one_degree_radians;
            }
         }
         break;
      case RGUI_PARTICLE_EFFECT_STARFIELD:
         {
            //float min_depth = (float)fb_width / 12.0f;

            for (i = 0; i < NUM_PARTICLES; i++)
            {
               rgui_particle_t *particle = &particles[i];

               /* x pos */
               particle->a = (float)(rand() % fb_width);
               /* y pos */
               particle->b = (float)(rand() % fb_height);
               /* depth */
               particle->c = (float)fb_width;
               /* speed */
               particle->d = 1.0f + ((float)(rand() % 20) * 0.01f);
            }
         }
         break;
      default:
         /* Do nothing... */
         break;
   }
}

static void rgui_render_particle_effect(rgui_handle_t *rgui)
{
   //size_t fb_pitch = driver.menu->frame_buf_pitch;
   unsigned fb_width = driver.menu->width, fb_height = driver.menu->height;
   size_t i;
   /* Give speed factor a long, awkward name to minimise
    * risk of clashing with specific particle effect
    * implementation variables... */
   float global_speed_factor = 1.0f;
   global_speed_factor = g_settings.particle_speed;
   particle_color = g_settings.particle_clr;
   //settings_t *settings = config_get_ptr();

   /* Sanity check */
   if (!rgui || !driver.menu->frame_buf)
      return;

   //menu_display_get_fb_size(&fb_width, &fb_height, &fb_pitch);

   /* Adjust global animation speed */
   /* > Apply user configured speed multiplier */
//   global_speed_factor = g_settings.particle_speed;
     //    (DEFAULT_RGUI_PARTICLE_EFFECT_SPEED > 0.0001f) ?
       //        DEFAULT_RGUI_PARTICLE_EFFECT_SPEED : 1.0f;

   /* > Account for non-standard frame times
    *   (high/low refresh rates, or frame drops) */
   //global_speed_factor *= menu_animation_get_delta_time() / particle_effect_period;

   /* Note: It would be more elegant to have 'update' and 'draw'
    * as separate functions, since 'update' is the part that
    * varies with particle effect whereas 'draw' is always
    * pretty much the same. However, this has the following
    * disadvantages:
    * - It means we have to loop through all particles twice,
    *   and given that we're already using a heap of CPU cycles
    *   to draw these effects any further performance overheads
    *   are to be avoided
    * - It locks us into a particular draw style. e.g. What if
    *   an effect calls for round particles, instead of square
    *   ones? This would make a mess of any 'standardised'
    *   drawing
    * So we go with the simple option of having the entire
    * update/draw sequence here. This results in some code
    * repetition, but it has better performance and allows for
    * complete flexibility */

   switch (particle_effect)
   {
      case RGUI_PARTICLE_EFFECT_SNOW:
      case RGUI_PARTICLE_EFFECT_SNOW_ALT:
         {
            unsigned particle_size;
            bool on_screen;

            for (i = 0; i < NUM_PARTICLES; i++)
            {
               rgui_particle_t *particle = &particles[i];

               /* Update particle 'speed' */
               particle->c = particle->c + (float)(rand() % 16 - 9) * 0.01f;
               particle->d = particle->d + (float)(rand() % 16 - 7) * 0.01f;

               particle->c = (particle->c < -0.4f) ? -0.4f : particle->c;
               particle->c = (particle->c >  0.1f) ?  0.1f : particle->c;

               particle->d = (particle->d < -0.1f) ? -0.1f : particle->d;
               particle->d = (particle->d >  0.4f) ?  0.4f : particle->d;

               /* Update particle location */
               particle->a = fmod(particle->a + (global_speed_factor * particle->c), fb_width);
               particle->b = fmod(particle->b + (global_speed_factor * particle->d), fb_height);

               /* Get particle size */
               particle_size = 1;
               if (particle_effect == RGUI_PARTICLE_EFFECT_SNOW_ALT)
               {
                  /* Gives the following distribution:
                   * 1x1: 32
                   * 2x2: 128
                   * 3x3: 32 */
                  if (!(i & 0x2))
                     particle_size = 2;
                  else if ((i & 0x7) == 0x7)
                     particle_size = 3;
               }

               /* Draw particle */
               on_screen = rgui_draw_particle(driver.menu->frame_buf, fb_width, fb_height,
                                 (int)particle->a, (int)particle->b,
                                 particle_size, particle_size, particle_color);

               /* Reset particle if it has fallen off screen */
               if (!on_screen)
               {
                  particle->a = (particle->a < 0.0f) ? (particle->a + (float)fb_width)  : particle->a;
                  particle->b = (particle->b < 0.0f) ? (particle->b + (float)fb_height) : particle->b;
               }
            }
         }
         break;
      case RGUI_PARTICLE_EFFECT_RAIN:
         {
            uint8_t weights[] = { /* 60 entries */
               2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
               3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
               4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
               5, 5, 5, 5, 5, 5, 5, 5,
               6, 6, 6, 6, 6, 6,
               7, 7, 7, 7,
               8, 8, 8,
               9, 9,
               10};
            unsigned num_drops = (unsigned)(0.85f * ((float)fb_width / 426.0f) * (float)NUM_PARTICLES);
            bool on_screen;

            num_drops = num_drops < NUM_PARTICLES ? num_drops : NUM_PARTICLES;

            for (i = 0; i < num_drops; i++)
            {
               rgui_particle_t *particle = &particles[i];

               /* Draw particle */
               on_screen = rgui_draw_particle(driver.menu->frame_buf, fb_width, fb_height,
                                 (int)particle->a, (int)particle->b,
                                 2, (unsigned)particle->c, particle_color);

               /* Update y pos */
               particle->b += particle->d * global_speed_factor;

               /* Reset particle if it has fallen off the bottom of the screen */
               if (!on_screen)
               {
                  /* x pos */
                  particle->a = (float)(rand() % (fb_width / 3)) * 3.0f;
                  /* y pos */
                  particle->b = 0.0f;
                  /* drop length */
                  particle->c = (float)weights[(unsigned)(rand() % 60)];
                  /* drop speed (larger drops fall faster) */
                  particle->d = (particle->c / 12.0f) * (0.5f + ((float)(rand() % 150) / 200.0f));
               }
            }
         }
         break;
      case RGUI_PARTICLE_EFFECT_VORTEX:
         {
            float max_radius         = (float)sqrt((double)((fb_width * fb_width) + (fb_height * fb_height))) / 2.0f;
            float one_degree_radians = PI / 360.0f;
            int x_centre             = (int)(fb_width >> 1);
            int y_centre             = (int)(fb_height >> 1);
            unsigned particle_size;
            float r_speed, theta_speed;
            int x, y;

            for (i = 0; i < NUM_PARTICLES; i++)
            {
               rgui_particle_t *particle = &particles[i];

               /* Get particle location */
               x = (int)(particle->a * cos(particle->b)) + x_centre;
               y = (int)(particle->a * sin(particle->b)) + y_centre;

               /* Get particle size */
               particle_size = 1 + (unsigned)(((1.0f - ((max_radius - particle->a) / max_radius)) * 3.5f) + 0.5f);

               /* Draw particle */
               rgui_draw_particle(driver.menu->frame_buf, fb_width, fb_height,
                     x, y, particle_size, particle_size, particle_color);

               /* Update particle speed */
               r_speed     = particle->c * global_speed_factor;
               theta_speed = particle->d * global_speed_factor;
               if ((particle->a > 0.0f) && (particle->a < (float)fb_height))
               {
                  float base_scale_factor = ((float)fb_height - particle->a) / (float)fb_height;
                  r_speed     *= 1.0f + (base_scale_factor * 8.0f);
                  theta_speed *= 1.0f + (base_scale_factor * base_scale_factor * 6.0f);
               }
               particle->a -= r_speed;
               particle->b += theta_speed;

               /* Reset particle if it has reached the centre of the screen */
               if (particle->a < 0.0f)
               {
                  /* radius
                   * Note: In theory, this should be:
                   * > particle->a = max_radius;
                   * ...but it turns out that spawning new particles at random
                   * locations produces a more visually appealing result... */
                  particle->a = 1.0f + (((float)rand() / (float)RAND_MAX) * max_radius);
                  /* theta */
                  particle->b = ((float)rand() / (float)RAND_MAX) * 2.0f * PI;
                  /* radial speed */
                  particle->c = (float)((rand() % 100) + 1) * 0.001f;
                  /* rotational speed */
                  particle->d = (((float)((rand() % 50) + 1) / 200.0f) + 0.1f) * one_degree_radians;
               }
            }
         }
         break;
      case RGUI_PARTICLE_EFFECT_STARFIELD:
         {
            float focal_length = (float)fb_width * 2.0f;
            int x_centre       = (int)(fb_width >> 1);
            int y_centre       = (int)(fb_height >> 1);
            unsigned particle_size;
            int x, y;
            bool on_screen;

            /* Based on an example found here:
             * https://codepen.io/nodws/pen/pejBNb */
            for (i = 0; i < NUM_PARTICLES; i++)
            {
               rgui_particle_t *particle = &particles[i];

               /* Get particle location */
               x = (int)((particle->a - (float)x_centre) * (focal_length / particle->c));
               x += x_centre;

               y = (int)((particle->b - (float)y_centre) * (focal_length / particle->c));
               y += y_centre;

               /* Get particle size */
               particle_size = (unsigned)(focal_length / (2.0f * particle->c));

               /* Draw particle */
               on_screen = rgui_draw_particle(driver.menu->frame_buf, fb_width, fb_height,
                                 x, y, particle_size, particle_size, particle_color);

               /* Update depth */
               particle->c -= particle->d * global_speed_factor;

               /* Reset particle if it has:
                * - Dropped off the edge of the screen
                * - Reached the screen depth
                * - Grown larger than 16 pixels across
                *   (this is an arbitrary limit, set to reduce overall
                *   performance impact - i.e. larger particles are slower
                *   to draw, and without setting a limit they can fill the screen...) */
               if (!on_screen || (particle->c <= 0.0f) || particle_size > 16)
               {
                  /* x pos */
                  particle->a = (float)(rand() % fb_width);
                  /* y pos */
                  particle->b = (float)(rand() % fb_height);
                  /* depth */
                  particle->c = (float)fb_width;
                  /* speed */
                  particle->d = 1.0f + ((float)(rand() % 20) * 0.01f);
               }
            }
         }
         break;
      default:
         /* Do nothing... */
         break;
   }

   /* If border is enabled, it must be drawn *above*
    * particle effect
    * (Wastes CPU cycles, but nothing we can do about it...) */
   //if (rgui->border_enable && !rgui->show_wallpaper)
     // rgui_render_border(rgui, driver.menu->frame_buf, fb_pitch, fb_width, fb_height);
}

static void rgui_render_background(void)
{
   if (!driver.menu)
      return;

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         0, 0, driver.menu->width, driver.menu->height, g_settings.menu_solid ? custom_filler : gray_filler);

  /* fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         5, 5, driver.menu->width - 10, 5, green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         5, driver.menu->height - 10, driver.menu->width - 10, 5,
         green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         5, 5, 5, driver.menu->height - 10, green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         driver.menu->width - 10, 5, 5, driver.menu->height - 10,
         green_filler);*/

#if 0
	if (snow_enable) {
      for (int i = 0; i < 1024; i += 4) {
         snowflakes[i    ] = rand()%320;
         snowflakes[i + 1] = rand()%240;
         snowflakes[i + 2] = (rand()%64 - 16)*.1;
         snowflakes[i + 3] = (rand()%64 - 48)*.1;
      }
   }
#endif
#if 0
   /* Snow */
   if (snow_enable) {
      for (int i = 0; i < 1024; i += 4) {
         snowflakes[i + 2] = fmin(fmax(snowflakes[i + 2] + (rand()%16 - 9)*.01, -0.4), 0.1);
         snowflakes[i + 3] = fmin(fmax(snowflakes[i + 3] + (rand()%16 - 7)*.01, -0.1), 0.4);
         snowflakes[i    ] = fmod(snowflakes[i    ] + snowflakes[i + 2], 320);
         snowflakes[i + 1] = fmod(snowflakes[i + 1] + snowflakes[i + 3], 240);
         if (snowflakes[i    ] < 0) snowflakes[i    ] += 320;
         if (snowflakes[i + 1] < 0) snowflakes[i + 1] += 240;
         driver.menu->frame_buf[ (int) snowflakes[i+1] * (driver.menu->frame_buf_pitch >> 1) + (int) snowflakes[i]] = 0xFFFF;
      }
   }
#endif
   //Enable particle effects
   rgui_handle_t *rgui = NULL;
  // menu_handle_t *menu = (menu_handle_t*)data;
   //if (!menu)
     // return;
   rgui = (rgui_handle_t*)driver.menu->userdata;
   
  // fb_size_changed = (last_width  != 320) || 
        //             (last_height != 240);

	particle_effect = g_settings.particle_type;

    /* Reinitialise particle effect, if required */
    //if (gx_mode.fbWidth == 608 && (particle_effect != RGUI_PARTICLE_EFFECT_NONE))
    if (fb_size_changed && (particle_effect != RGUI_PARTICLE_EFFECT_NONE)) {
        rgui_init_particle_effect(rgui);
		fb_size_changed = false;
	}
	
	/* Render particle effect, if required */
   if (particle_effect != RGUI_PARTICLE_EFFECT_NONE)
      rgui_render_particle_effect(rgui);
}

static void rgui_render_messagebox(const char *message)
{
   size_t i;

   if (!driver.menu || !message || !*message)
      return;

   struct string_list *list = string_split(message, "\n");
   if (!list)
      return;
   if (list->elems == 0)
   {
      string_list_free(list);
      return;
   }

   unsigned width = 0;
   unsigned glyphs_width = 0;
   for (i = 0; i < list->size; i++)
   {
      char *msg = list->elems[i].data;
      unsigned msglen = strlen(msg);
      if (msglen > RGUI_TERM_WIDTH)
      {
         msg[RGUI_TERM_WIDTH - 2] = '.';
         msg[RGUI_TERM_WIDTH - 1] = '.';
         msg[RGUI_TERM_WIDTH - 0] = '.';
         msg[RGUI_TERM_WIDTH + 1] = '\0';
         msglen = RGUI_TERM_WIDTH;
      }

      unsigned line_width = msglen * FONT_WIDTH_STRIDE - 1 + 6 + 10;
      width = max(width, line_width);
      glyphs_width = max(glyphs_width, msglen);
   }

   unsigned height = FONT_HEIGHT_STRIDE * list->size + 6 + 10;
   int x = (driver.menu->width - width) / 2;
   int y = (driver.menu->height - height) / 2;

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x + 5, y + 5, width - 10, height - 10, g_settings.menu_solid ? custom_filler : gray_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x, y, width - 5, 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x + width - 5, y, 5, height - 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x + 5, y + height - 5, width - 5, 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   fill_rect(driver.menu->frame_buf, driver.menu->frame_buf_pitch,
         x, y + 5, 5, height - 5, g_settings.menu_msg_clr ? custom_msg_filler : green_filler);

   for (i = 0; i < list->size; i++)
   {
      const char *msg = list->elems[i].data;
      int offset_x = FONT_WIDTH_STRIDE * (glyphs_width - strlen(msg)) / 2;
      int offset_y = FONT_HEIGHT_STRIDE * i;
      blit_line(x + 8 + offset_x, y + 8 + offset_y, msg, false);
   }

   string_list_free(list);
}

static void rgui_render(void)
{
   size_t begin = 0;
   size_t end;

   if (driver.menu->need_refresh 
         && g_extern.is_menu
         && !driver.menu->msg_force)
      return;

   if (driver.menu->selection_ptr >= RGUI_TERM_HEIGHT / 2)
      begin = driver.menu->selection_ptr - RGUI_TERM_HEIGHT / 2;
   end   = (driver.menu->selection_ptr + RGUI_TERM_HEIGHT <=
         menu_list_get_size(driver.menu->menu_list)) ?
      driver.menu->selection_ptr + RGUI_TERM_HEIGHT :
      menu_list_get_size(driver.menu->menu_list);

   /* Do not scroll if all items are visible. */
   if (menu_list_get_size(driver.menu->menu_list) <= RGUI_TERM_HEIGHT)
      begin = 0;

   if (end - begin > RGUI_TERM_HEIGHT)
      end = begin + RGUI_TERM_HEIGHT;

   rgui_render_background();

   char title[256];
   const char *dir = NULL;
   const char *label = NULL;
   unsigned menu_type = 0;
   menu_list_get_last_stack(driver.menu->menu_list,
         &dir, &label, &menu_type);

#if 0
   RARCH_LOG("Dir is: %s\n", label);
#endif

   get_title(label, dir, menu_type, title, sizeof(title));

   char title_buf[256];
   menu_ticker_line(title_buf, RGUI_TERM_WIDTH - 3,
         g_extern.frame_count / RGUI_TERM_START_X, title, true);
   // Current TITLE
   blit_line(g_settings.title_posx, g_settings.title_posy, title_buf, true);

   char title_msg[64];
   const char *core_name = g_extern.menu.info.library_name;
   if (!core_name)
      core_name = g_extern.system.info.library_name;
   if (!core_name)
      core_name = "No Core";

   const char *core_version = g_extern.menu.info.library_version;
   if (!core_version)
      core_version = g_extern.system.info.library_version;
   if (!core_version)
      core_version = "";

   if (!g_settings.single_mode) {
         //snprintf(title_msg, sizeof(title_msg), "%s - %s %s", PACKAGE_VERSION,
           //  core_name, core_version);
         snprintf(title_msg, sizeof(title_msg), "%s %s",
                  core_name, core_version);
         blit_line(
             RGUI_TERM_START_X + RGUI_TERM_START_X,
             (RGUI_TERM_HEIGHT * FONT_HEIGHT_STRIDE) +
             RGUI_TERM_START_Y + 2, title_msg, true);
	}

    if (g_settings.clock_show) {
        char timetxt[10];
        time_t currenttime = time(0);
        struct tm * timeinfo = localtime(&currenttime);

     // strftime(timetxt, sizeof(timetxt),
     //          "%H:%M:%S", timeinfo);
        strftime(timetxt, sizeof(timetxt),
                    "%I:%M %p", timeinfo);
        blit_line(
             g_settings.clock_posx,
             (RGUI_TERM_HEIGHT * FONT_HEIGHT_STRIDE) +
             RGUI_TERM_START_Y + 2, timetxt, true);
    }

   unsigned x, y;
   size_t i;

   x = RGUI_TERM_START_X;
   y = RGUI_TERM_START_Y;

   for (i = begin; i < end; i++, y += FONT_HEIGHT_STRIDE)
   {
      char message[PATH_MAX], type_str[PATH_MAX],
           entry_title_buf[PATH_MAX], type_str_buf[PATH_MAX],
           path_buf[PATH_MAX];
      const char *path = NULL, *entry_label = NULL;
      unsigned type = 0, w = 0;
      bool selected = false;

      menu_list_get_at_offset(driver.menu->menu_list->selection_buf, i, &path,
            &entry_label, &type);
      rarch_setting_t *setting = (rarch_setting_t*)setting_data_find_setting(
            driver.menu->list_settings,
            driver.menu->menu_list->selection_buf->list[i].label);
      (void)setting;

      disp_set_label(driver.menu->menu_list->selection_buf, &w, type, i, label,
            type_str, sizeof(type_str), 
            entry_label, path,
            path_buf, sizeof(path_buf));

      selected = (i == driver.menu->selection_ptr);

      menu_ticker_line(entry_title_buf, RGUI_TERM_WIDTH - (w + 1 + 2),
            g_extern.frame_count / RGUI_TERM_START_X, path_buf, selected);
      menu_ticker_line(type_str_buf, w, g_extern.frame_count / RGUI_TERM_START_X,
            type_str, selected);

      snprintf(message, sizeof(message), "%c %-*.*s %-*s", selected ? !g_settings.hide_cursor ? '>' : ' ' : ' ',
            RGUI_TERM_WIDTH - (w + 1 + 2),
            RGUI_TERM_WIDTH - (w + 1 + 2),
            entry_title_buf,
            w,
            type_str_buf);

      blit_line(x + g_settings.item_posx, y + g_settings.item_posy, message, selected);
   }

#ifdef GEKKO
   const char *message_queue;

   if (driver.menu->msg_force)
   {
      message_queue = msg_queue_pull(g_extern.msg_queue);
      driver.menu->msg_force = false;
   }
   else
      message_queue = driver.current_msg;

   rgui_render_messagebox(message_queue);
#endif
/*
   if (driver.menu->keyboard.display)
   {
      char msg[PATH_MAX];
      const char *str = *driver.menu->keyboard.buffer;
      if (!str)
         str = "";
      snprintf(msg, sizeof(msg), "%s\n%s", driver.menu->keyboard.label, str);
      rgui_render_messagebox(msg);
   }*/
}

static void *rgui_init(void)
{
   menu_handle_t *menu = (menu_handle_t*)calloc(1, sizeof(*menu));

   if (!menu)
      return NULL;

   menu->userdata = (rgui_handle_t*)calloc(1, sizeof(rgui_handle_t));
   
   if (!menu->userdata)
   {
      free(menu);
      return NULL;
   }

   menu->frame_buf = (uint16_t*)malloc(400 * 240 * sizeof(uint16_t)); 

   if (!menu->frame_buf)
   {
      free(menu->userdata);
      free(menu);
      return NULL;
   }

   menu->width = 320;
   menu->height = 240;
   menu->frame_buf_pitch = menu->width * sizeof(uint16_t);

   bool ret = rguidisp_init_font(menu);

   if (!ret)
   {
      RARCH_ERR("No font bitmap or binary, abort");

      rarch_main_command(RARCH_CMD_QUIT_RETROARCH);

      free(menu->userdata);
      free(menu);
      return NULL;
   }
   
   //This code crashes on real Wii but not Dolphin
    //last_width = menu->width;
    //last_height = menu->height;
   //	rgui_handle_t *rgui = NULL;

    //rgui = (rgui_handle_t*)driver.menu->userdata;
	//if(particle_effect != RGUI_PARTICLE_EFFECT_NONE)
		//rgui_init_particle_effect(rgui);

   return menu;
}

static void rgui_free(void *data)
{
   rgui_handle_t *rgui = NULL;
   menu_handle_t *menu = (menu_handle_t*)data;

   if (!menu)
      return;

   rgui = (rgui_handle_t*)menu->userdata;

   if (!rgui)
      return;

   if (menu->frame_buf)
      free(menu->frame_buf);

   if (menu->userdata)
      free(menu->userdata);
   driver.menu->userdata = NULL;

   if (menu->alloc_font)
      free((uint8_t*)menu->font);
}

static void rgui_set_texture(void *data)
{
   menu_handle_t *menu = (menu_handle_t*)data;

   if (driver.video_data && driver.video_poke &&
         driver.video_poke->set_texture_frame)
      driver.video_poke->set_texture_frame(driver.video_data,
            menu->frame_buf, false, menu->width, menu->height, 1.0f);
}

menu_ctx_driver_t menu_ctx_rgui = {
   rgui_set_texture,
   rgui_render_messagebox,
   rgui_render,
   NULL,
   rgui_init,
   NULL,
   rgui_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   &menu_ctx_backend_common,
   "rgui",
};
