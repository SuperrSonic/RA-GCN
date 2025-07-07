/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2013-2014 - Jason Fetters
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <boolean.h>
#include "joypad_connection.h"

struct hidpad_ps4_data
{
   struct pad_connection* connection;
   send_control_t send_control;
   uint8_t data[512];
   uint32_t slot;
   bool have_led;
   uint16_t motors[2];
};

static void hidpad_ps4_send_control(struct hidpad_ps4_data* device)
{
   /* TODO: Can this be modified to turn off motion tracking? */
   static uint8_t report_buffer[79] = {
      0x52, 0x11, 0xB0, 0x00, 0x0F
   };
   
#if 0
   uint8_t rgb[4][3] = { { 0xFF, 0, 0 }, { 0, 0xFF, 0 }, { 0, 0, 0xFF }, { 0xFF, 0xFF, 0xFF } };
   report_buffer[ 9] = rgb[(device->slot % 4)][0];
   report_buffer[10] = rgb[(device->slot % 4)][1];
   report_buffer[11] = rgb[(device->slot % 4)][2];
#endif
    
   device->send_control(device->connection, report_buffer, sizeof(report_buffer));
}

static void* hidpad_ps4_init(void *data, uint32_t slot, send_control_t ptr)
{
   uint8_t magic_data[0x25];
   struct pad_connection* connection = (struct pad_connection*)data;
   struct hidpad_ps4_data* device = (struct hidpad_ps4_data*)
    calloc(1, sizeof(struct hidpad_ps4_data));

   if (!device)
      return NULL;

   if (!connection)
   {
      free(device);
      return NULL;
   }

   device->connection = connection;  
   device->slot = slot;
   device->send_control = ptr;
   
   /* TODO - unsure of this */
   /* This is needed to get full input packet over bluetooth. */
   device->send_control(device->connection, magic_data, 0x2);

   /* Without this, the digital buttons won't be reported. */
   hidpad_ps4_send_control(device);

   return device;
}

static void hidpad_ps4_deinit(void *data)
{
   struct hidpad_ps4_data *device = (struct hidpad_ps4_data*)data;
    
   if (device)
      free(device);
}

#if 0
static uint32_t hidpad_ps4_get_buttons(void *data)
{
   uint32_t result = 0;
   struct hidpad_ps4_data *device = (struct hidpad_ps4_data*)data;

   struct Report
   {
      uint8_t leftX;
      uint8_t leftY;
      uint8_t rightX;
      uint8_t rightY;
      uint8_t buttons[3];
      uint8_t leftTrigger;
      uint8_t rightTrigger;
   };

   struct Report* rpt = (struct Report*)&device->data[4];
   const uint8_t dpad_state = rpt->buttons[0] & 0xF;

   result |= ((rpt->buttons[0] & 0x20) ? (1 << RETRO_DEVICE_ID_JOYPAD_B) : 0);
   result |= ((rpt->buttons[0] & 0x40) ? (1 << RETRO_DEVICE_ID_JOYPAD_A) : 0);
   result |= ((rpt->buttons[0] & 0x10) ? (1 << RETRO_DEVICE_ID_JOYPAD_Y) : 0);
   result |= ((rpt->buttons[0] & 0x80) ? (1 << RETRO_DEVICE_ID_JOYPAD_X) : 0);
   result |= ((rpt->buttons[1] & 0x01) ? (1 << RETRO_DEVICE_ID_JOYPAD_L) : 0);
   result |= ((rpt->buttons[1] & 0x02) ? (1 << RETRO_DEVICE_ID_JOYPAD_R) : 0);
   result |= ((rpt->buttons[1] & 0x04) ? (1 << RETRO_DEVICE_ID_JOYPAD_L2) : 0);
   result |= ((rpt->buttons[1] & 0x08) ? (1 << RETRO_DEVICE_ID_JOYPAD_R2) : 0);
   result |= ((rpt->buttons[1] & 0x10) ? (1 << RETRO_DEVICE_ID_JOYPAD_SELECT) : 0);
   result |= ((rpt->buttons[1] & 0x20) ? (1 << RETRO_DEVICE_ID_JOYPAD_START) : 0);
   result |= ((rpt->buttons[1] & 0x40) ? (1 << RETRO_DEVICE_ID_JOYPAD_L3) : 0);
   result |= ((rpt->buttons[1] & 0x80) ? (1 << RETRO_DEVICE_ID_JOYPAD_R3) : 0);
   result |= ((dpad_state & 0x00) ? (1 << RETRO_DEVICE_ID_JOYPAD_UP) : 0);
   result |= ((dpad_state & 0x02) ? (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT) : 0);
   result |= ((dpad_state & 0x04) ? (1 << RETRO_DEVICE_ID_JOYPAD_DOWN) : 0);
   result |= ((dpad_state & 0x06) ? (1 << RETRO_DEVICE_ID_JOYPAD_LEFT) : 0);
   result |= ((rpt->buttons[2] & 0x01) ? (1 << 16) : 0);

   return result;
}

static int16_t hidpad_ps4_get_axis(void *data, unsigned axis)
{
   struct hidpad_ps4_data *device = (struct hidpad_ps4_data*)data;
    
   if (device && (axis < 4))
   {
      int val = device->data[7 + axis];
      val = (val << 8) - 0x8000;
      return (abs(val) > 0x1000) ? val : 0;
   }

   return 0;
}
#endif

static void hidpad_ps4_packet_handler(void *data, uint8_t *packet, uint16_t size)
{
   int i;
   struct hidpad_ps4_data *device = (struct hidpad_ps4_data*)data;
    
   if (!device)
      return;
    
   (void)i;
    
#if 0
   if (!device->have_led)
   {
      hidpad_ps4_send_control(device);
      device->have_led = true;
   }
#endif

   memcpy(device->data, packet, size);
}

static void hidpad_ps4_set_rumble(void *data,
   enum retro_rumble_effect effect, uint16_t strength)
{
   /* TODO */
#if 0
   struct hidpad_ps4_data *device = (struct hidpad_ps4_data*)data;
   unsigned idx = (effect == RETRO_RUMBLE_STRONG) ? 0 : 1;

   if (device && (device->motors[idx] != strength))
   {
      device->motors[idx] = strength;
      hidpad_ps4_send_control(device);
   }
#endif
}

pad_connection_interface_t pad_connection_ps4 = {
   hidpad_ps4_init,
   hidpad_ps4_deinit,
   hidpad_ps4_packet_handler,
   hidpad_ps4_set_rumble
};
