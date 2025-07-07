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

#ifndef INPUT_COMMON_H__
#define INPUT_COMMON_H__

#include "input_autodetect.h"
#include "../driver.h"
#include <file/config_file.h>
#include "../general.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t retro_input_t ;

static inline void input_conv_analog_id_to_bind_id(unsigned idx, unsigned ident,
      unsigned *ident_minus, unsigned *ident_plus)
{
   switch ((idx << 1) | ident)
   {
      case (RETRO_DEVICE_INDEX_ANALOG_LEFT << 1) | RETRO_DEVICE_ID_ANALOG_X:
         *ident_minus = RARCH_ANALOG_LEFT_X_MINUS;
         *ident_plus  = RARCH_ANALOG_LEFT_X_PLUS;
         break;

      case (RETRO_DEVICE_INDEX_ANALOG_LEFT << 1) | RETRO_DEVICE_ID_ANALOG_Y:
         *ident_minus = RARCH_ANALOG_LEFT_Y_MINUS;
         *ident_plus  = RARCH_ANALOG_LEFT_Y_PLUS;
         break;

      case (RETRO_DEVICE_INDEX_ANALOG_RIGHT << 1) | RETRO_DEVICE_ID_ANALOG_X:
         *ident_minus = RARCH_ANALOG_RIGHT_X_MINUS;
         *ident_plus  = RARCH_ANALOG_RIGHT_X_PLUS;
         break;

      case (RETRO_DEVICE_INDEX_ANALOG_RIGHT << 1) | RETRO_DEVICE_ID_ANALOG_Y:
         *ident_minus = RARCH_ANALOG_RIGHT_Y_MINUS;
         *ident_plus  = RARCH_ANALOG_RIGHT_Y_PLUS;
         break;
   }
}

#if 0
static inline bool get_bit(const uint8_t *buf, unsigned bit)
{
   return buf[bit >> 3] & (1 << (bit & 7));
}

static inline void clear_bit(uint8_t *buf, unsigned bit)
{
   buf[bit >> 3] &= ~(1 << (bit & 7));
}

static inline void set_bit(uint8_t *buf, unsigned bit)
{
   buf[bit >> 3] |= 1 << (bit & 7);
}
#endif

bool input_translate_coord_viewport(int mouse_x, int mouse_y,
      int16_t *res_x, int16_t *res_y, int16_t *res_screen_x,
      int16_t *res_screen_y);

#ifdef ANDROID
enum back_button_enums
{
   BACK_BUTTON_QUIT = 0,
   BACK_BUTTON_GUI_TOGGLE,
   BACK_BUTTON_MENU_TOGGLE,
};
#endif

struct rarch_joypad_driver
{
   bool (*init)(void);
   bool (*query_pad)(unsigned);
   void (*destroy)(void);
   bool (*button)(unsigned, uint16_t);
   int16_t (*axis)(unsigned, uint32_t);
   void (*poll)(void);
   bool (*set_rumble)(unsigned, enum retro_rumble_effect, uint16_t);
   const char *(*name)(unsigned);

   const char *ident;
};

/* If ident points to NULL or a zero-length string,
 * equivalent to calling input_joypad_init_first(). */
const rarch_joypad_driver_t *input_joypad_init_driver(const char *ident);

const rarch_joypad_driver_t *input_joypad_init_first(void);

bool input_joypad_pressed(const rarch_joypad_driver_t *driver,
      unsigned port, const struct retro_keybind *binds, unsigned key);

int16_t input_joypad_analog(const rarch_joypad_driver_t *driver,
      unsigned port, unsigned idx, unsigned ident,
      const struct retro_keybind *binds);

bool input_joypad_set_rumble(const rarch_joypad_driver_t *driver,
      unsigned port, enum retro_rumble_effect effect, uint16_t strength);

int16_t input_joypad_axis_raw(const rarch_joypad_driver_t *driver,
      unsigned joypad, unsigned axis);

bool input_joypad_button_raw(const rarch_joypad_driver_t *driver,
      unsigned joypad, unsigned button);

bool input_joypad_hat_raw(const rarch_joypad_driver_t *driver,
      unsigned joypad, unsigned hat_dir, unsigned hat);

const char *input_joypad_name(const rarch_joypad_driver_t *driver,
      unsigned joypad);

extern rarch_joypad_driver_t dinput_joypad;
extern rarch_joypad_driver_t linuxraw_joypad;
extern rarch_joypad_driver_t parport_joypad;
extern rarch_joypad_driver_t udev_joypad;
extern rarch_joypad_driver_t winxinput_joypad;
extern rarch_joypad_driver_t sdl_joypad;
extern rarch_joypad_driver_t ps3_joypad;
extern rarch_joypad_driver_t psp_joypad;
extern rarch_joypad_driver_t xdk_joypad;
extern rarch_joypad_driver_t gx_joypad;
extern rarch_joypad_driver_t apple_hid_joypad;
extern rarch_joypad_driver_t apple_ios_joypad;
extern rarch_joypad_driver_t android_joypad;
extern rarch_joypad_driver_t qnx_joypad;

struct rarch_key_map
{
   unsigned sym;
   enum retro_key rk;
};

extern const struct rarch_key_map rarch_key_map_x11[];
extern const struct rarch_key_map rarch_key_map_sdl[];
extern const struct rarch_key_map rarch_key_map_sdl2[];
extern const struct rarch_key_map rarch_key_map_dinput[];
extern const struct rarch_key_map rarch_key_map_rwebinput[];
extern const struct rarch_key_map rarch_key_map_linux[];
extern const struct rarch_key_map rarch_key_map_apple_hid[];

void input_init_keyboard_lut(const struct rarch_key_map *map);
enum retro_key input_translate_keysym_to_rk(unsigned sym);
unsigned input_translate_rk_to_keysym(enum retro_key key);

/* Input config. */
struct input_bind_map
{
   bool valid;

   /* Meta binds get input as prefix, not input_playerN".
    * 0 = libretro related.
    * 1 = Common hotkey.
    * 2 = Uncommon/obscure hotkey.
    */
   unsigned meta;

   const char *base;
   const char *desc;
   unsigned retro_key;
};

extern const struct input_bind_map input_config_bind_map[];

/* auto_bind can be NULL. */
void input_get_bind_string(char *buf, const struct retro_keybind *bind,
      const struct retro_keybind *auto_bind, size_t size);

struct input_key_map
{
   const char *str;
   enum retro_key key;
};
extern const struct input_key_map input_config_key_map[];
void input_translate_rk_to_str(enum retro_key key, char *buf, size_t size);
enum retro_key input_translate_str_to_rk(const char *str);

const char *input_config_get_prefix(unsigned player, bool meta);

/* Returns RARCH_BIND_LIST_END on not found. */
unsigned input_translate_str_to_bind_id(const char *str);

void input_config_parse_key(config_file_t *conf,
      const char *prefix, const char *btn,
      struct retro_keybind *bind);

void input_config_parse_joy_button(config_file_t *conf, const char *prefix,
      const char *btn, struct retro_keybind *bind);

void input_config_parse_joy_axis(config_file_t *conf, const char *prefix,
      const char *axis, struct retro_keybind *bind);

void input_push_analog_dpad(struct retro_keybind *binds, unsigned mode);

void input_pop_analog_dpad(struct retro_keybind *binds);

#ifdef __cplusplus
}
#endif

#endif

