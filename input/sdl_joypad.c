/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
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

#include "input_common.h"
#include "SDL.h"
#include "../general.h"

typedef struct _sdl_joypad
{
   SDL_Joystick *joypad;
#ifdef HAVE_SDL2
   SDL_GameController *controller;
   SDL_Haptic *haptic;
   int rumble_effect; // -1 = not initialized, -2 = error/unsupported
#endif
   unsigned num_axes;
   unsigned num_buttons;
   unsigned num_hats;
#ifdef HAVE_SDL2
   unsigned num_balls;
#endif
} sdl_joypad_t;

#ifdef HAVE_SDL2
const int g_subsystem = SDL_INIT_GAMECONTROLLER;
#else
const int g_subsystem = SDL_INIT_JOYSTICK;
#endif

static sdl_joypad_t g_pads[MAX_PLAYERS];
#ifdef HAVE_SDL2
static bool g_has_haptic;
#endif

static const char* pad_name(unsigned id)
{
#ifdef HAVE_SDL2
   if (g_pads[id].controller)
      return SDL_GameControllerNameForIndex(id);
   return SDL_JoystickNameForIndex(id);
#else
   return SDL_JoystickName(id);
#endif
}

static uint8_t pad_get_button(sdl_joypad_t *pad, unsigned button)
{
#ifdef HAVE_SDL2
   /* TODO: see if a LUT like winxinput_joypad.c's button_index_to_bitmap_code is needed. */
   if (pad->controller)
      return SDL_GameControllerGetButton(pad->controller, (SDL_GameControllerButton)button);
#endif
   return SDL_JoystickGetButton(pad->joypad, button);
}

static uint8_t pad_get_hat(sdl_joypad_t *pad, unsigned hat)
{
#ifdef HAVE_SDL2
   if (pad->controller)
      return pad_get_button(pad, hat);
#endif
   return SDL_JoystickGetHat(pad->joypad, hat);
}

static int16_t pad_get_axis(sdl_joypad_t *pad, unsigned axis)
{
#ifdef HAVE_SDL2
   /* TODO: see if a rarch <-> sdl translation is needed. */
   if (pad->controller)
      return SDL_GameControllerGetAxis(pad->controller, (SDL_GameControllerAxis)axis);
#endif
   return SDL_JoystickGetAxis(pad->joypad, axis);
}

static void pad_connect(unsigned id)
{
   sdl_joypad_t *pad = &g_pads[id];
   bool success = false;
   int32_t product = 0;
   int32_t vendor = 0;

#ifdef HAVE_SDL2
   uint16_t *guid = NULL;

   if (SDL_IsGameController(id))
   {
      pad->controller = SDL_GameControllerOpen(id);
      pad->joypad     = SDL_GameControllerGetJoystick(pad->controller);

      success = pad->joypad != NULL && pad->controller != NULL;
   }
   else
#endif
   {
      pad->joypad = SDL_JoystickOpen(id);
      success = pad->joypad != NULL;
   }

   if (!success)
   {
      RARCH_ERR("[SDL]: Couldn't open joystick #%u: %s.\n", id, SDL_GetError());

      if (pad->joypad)
         SDL_JoystickClose(pad->joypad);

      pad->joypad = NULL;

      return;
   }

   strlcpy(g_settings.input.device_names[id], pad_name(id), sizeof(g_settings.input.device_names[id]));

#ifdef HAVE_SDL2
   guid = (uint16_t*)SDL_JoystickGetGUID(pad->joypad).data;
#ifdef __linux
   vendor = guid[2];
   product = guid[4];
#elif _WIN32
   vendor = guid[0];
   product = guid[1];
#endif
#endif
   input_config_autoconfigure_joypad(id, pad_name(id),
         vendor, product, sdl_joypad.ident);

   RARCH_LOG("[SDL]: Joypad #%u (%04x:%04x) connected: %s.\n", id, vendor,
             product, pad_name(id));

#ifdef HAVE_SDL2

   if (pad->controller)
      RARCH_LOG("[SDL]: Joypad #%u supports game controller api.\n", id);

   pad->haptic = g_has_haptic ? SDL_HapticOpenFromJoystick(pad->joypad) : NULL;

   if (g_has_haptic && !pad->haptic)
      RARCH_WARN("[SDL]: Couldn't open haptic device of the joypad #%u: %s\n",
                 id, SDL_GetError());

   pad->rumble_effect = -1;

   if (pad->haptic)
   {
      SDL_HapticEffect efx;
      efx.type = SDL_HAPTIC_LEFTRIGHT;
      efx.leftright.type = SDL_HAPTIC_LEFTRIGHT;
      efx.leftright.large_magnitude = efx.leftright.small_magnitude = 0x4000;
      efx.leftright.length = 5000;

      if (SDL_HapticEffectSupported(pad->haptic, &efx) == SDL_FALSE)
      {
         pad->rumble_effect = -2;
         RARCH_WARN("[SDL]: Joypad #%u does not support rumble.\n", id);
      }
   }
#endif

   pad->num_axes    = SDL_JoystickNumAxes(pad->joypad);
   pad->num_buttons = SDL_JoystickNumButtons(pad->joypad);
   pad->num_hats    = SDL_JoystickNumHats(pad->joypad);

#ifdef HAVE_SDL2
   pad->num_balls   = SDL_JoystickNumBalls(pad->joypad);

   RARCH_LOG("[SDL]: Joypad #%u has: %u axes, %u buttons, %u hats and %u trackballs.\n",
             id, pad->num_axes, pad->num_buttons, pad->num_hats, pad->num_balls);
#else
   RARCH_LOG("[SDL]: Joypad #%u has: %u axes, %u buttons, %u hats.\n",
             id, pad->num_axes, pad->num_buttons, pad->num_hats);
#endif
}

static void pad_disconnect(unsigned id)
{
#ifdef HAVE_SDL2
   if (g_pads[id].haptic)
      SDL_HapticClose(g_pads[id].haptic);

   if (g_pads[id].controller)
   {
      SDL_GameControllerClose(g_pads[id].controller);
      RARCH_LOG("[SDL]: Joypad #%u disconnected.\n", id);
   }
   else
#endif
   if (g_pads[id].joypad)
   {
      SDL_JoystickClose(g_pads[id].joypad);
      RARCH_LOG("[SDL]: Joypad #%u disconnected.\n", id);
   }

   g_settings.input.device_names[id][0] = '\0';

   memset(&g_pads[id], 0, sizeof(g_pads[id]));
}

static void sdl_joypad_destroy(void)
{
   unsigned i;
   for (i = 0; i < MAX_PLAYERS; i++)
      pad_disconnect(i);

   SDL_QuitSubSystem(g_subsystem);
   memset(g_pads, 0, sizeof(g_pads));
}

static bool sdl_joypad_init(void)
{
   unsigned i;

   if (SDL_WasInit(0) == 0)
   {
      if (SDL_Init(g_subsystem) < 0)
         return false;
   }
   else if (SDL_InitSubSystem(g_subsystem) < 0)
      return false;

#if HAVE_SDL2
   g_has_haptic = false;
   if (SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0)
      RARCH_WARN("[SDL]: Failed to initialize haptic device support: %s\n",
            SDL_GetError());
   else
      g_has_haptic = true;
#endif

   memset(g_pads, 0, sizeof(g_pads));

   unsigned num_sticks = SDL_NumJoysticks();
   if (num_sticks > MAX_PLAYERS)
      num_sticks = MAX_PLAYERS;

   for (i = 0; i < num_sticks; i++)
      pad_connect(i);

#ifndef HAVE_SDL2
   /* quit if no joypad is detected. */
   num_sticks = 0;
   for (i = 0; i < MAX_PLAYERS; i++)
      if (g_pads[i].joypad)
         num_sticks++;

   if (num_sticks == 0)
      goto error;
#endif

   return true;

#ifndef HAVE_SDL2
error:
   sdl_joypad_destroy();
   return false;
#endif
}

static bool sdl_joypad_button(unsigned port, uint16_t joykey)
{
   if (joykey == NO_BTN)
      return false;

   sdl_joypad_t *pad = &g_pads[port];
   if (!pad->joypad)
      return false;

   // Check hat.
   if (GET_HAT_DIR(joykey))
   {
      uint16_t hat = GET_HAT(joykey);
      if (hat >= pad->num_hats)
         return false;

      Uint8 dir = pad_get_hat(pad, hat);
      switch (GET_HAT_DIR(joykey))
      {
         case HAT_UP_MASK:
            return dir & SDL_HAT_UP;
         case HAT_DOWN_MASK:
            return dir & SDL_HAT_DOWN;
         case HAT_LEFT_MASK:
            return dir & SDL_HAT_LEFT;
         case HAT_RIGHT_MASK:
            return dir & SDL_HAT_RIGHT;
         default:
            break;
      }
      return false;
   }

   // Check the button
   if (joykey < pad->num_buttons && pad_get_button(pad, joykey))
      return true;

   return false;
}

static int16_t sdl_joypad_axis(unsigned port, uint32_t joyaxis)
{
   if (joyaxis == AXIS_NONE)
      return 0;

   sdl_joypad_t *pad = &g_pads[port];
   if (!pad->joypad)
      return false;

   int16_t val = 0;
   if (AXIS_NEG_GET(joyaxis) < pad->num_axes)
   {
      val = pad_get_axis(pad, AXIS_NEG_GET(joyaxis));

      if (val > 0)
         val = 0;
      else if (val < -0x7fff) // -0x8000 can cause trouble if we later abs() it.
         val = -0x7fff;
   }
   else if (AXIS_POS_GET(joyaxis) < pad->num_axes)
   {
      val = pad_get_axis(pad, AXIS_POS_GET(joyaxis));

      if (val < 0)
         val = 0;
   }

   return val;
}

static void sdl_joypad_poll(void)
{
#ifdef HAVE_SDL2
   SDL_Event event;

   SDL_PumpEvents();
   while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_JOYDEVICEADDED, SDL_JOYDEVICEREMOVED) > 0)
   {
      if (event.type == SDL_JOYDEVICEADDED)
      {
         pad_connect(event.jdevice.which);
      }
      else if (event.type == SDL_JOYDEVICEREMOVED)
      {
         pad_disconnect(event.jdevice.which);
      }
   }
#else
   SDL_JoystickUpdate();
#endif
}

#ifdef HAVE_SDL2
static bool sdl_joypad_set_rumble(unsigned pad, enum retro_rumble_effect effect, uint16_t strength)
{
   SDL_HapticEffect efx;
   memset(&efx, 0, sizeof(efx));

   sdl_joypad_t *joypad = &g_pads[pad];

   if (!joypad->joypad || !joypad->haptic)
      return false;

   efx.type = SDL_HAPTIC_LEFTRIGHT;
   efx.leftright.type  = SDL_HAPTIC_LEFTRIGHT;
   efx.leftright.length = 5000;

   switch (effect)
   {
      case RETRO_RUMBLE_STRONG: efx.leftright.large_magnitude = strength; break;
      case RETRO_RUMBLE_WEAK: efx.leftright.small_magnitude = strength; break;
      default: return false;
   }

   if (joypad->rumble_effect == -1)
   {
      joypad->rumble_effect = SDL_HapticNewEffect(g_pads[pad].haptic, &efx);
      if (joypad->rumble_effect < 0)
      {
         RARCH_WARN("[SDL]: Failed to create rumble effect for joypad %u: %s\n",
                    pad, SDL_GetError());
         joypad->rumble_effect = -2;
         return false;
      }
   }
   else if (joypad->rumble_effect >= 0)
      SDL_HapticUpdateEffect(joypad->haptic, joypad->rumble_effect, &efx);

   if (joypad->rumble_effect < 0)
      return false;

   if (SDL_HapticRunEffect(joypad->haptic, joypad->rumble_effect, 1) < 0)
   {
      RARCH_WARN("[SDL]: Failed to set rumble effect on joypad %u: %s\n",
                          pad, SDL_GetError());
      return false;
   }

   return true;
}
#endif

static bool sdl_joypad_query_pad(unsigned pad)
{
   return pad < MAX_PLAYERS && g_pads[pad].joypad;
}

static const char *sdl_joypad_name(unsigned pad)
{
   if (pad >= MAX_PLAYERS)
      return NULL;

   return pad_name(pad);
}

rarch_joypad_driver_t sdl_joypad = {
   sdl_joypad_init,
   sdl_joypad_query_pad,
   sdl_joypad_destroy,
   sdl_joypad_button,
   sdl_joypad_axis,
   sdl_joypad_poll,
#ifdef HAVE_SDL2
   sdl_joypad_set_rumble,
#else
   NULL,
#endif
   sdl_joypad_name,
#ifdef HAVE_SDL2
   "sdl2",
#else
   "sdl"
#endif
};
