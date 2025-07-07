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

#include <file/config_file.h>
#include <file/config_file_macros.h>
#include <compat/strl.h>
#include <compat/posix_string.h>
#include "config.def.h"
#include <file/file_path.h>
#include "input/input_common.h"

#ifdef USE_TITLE
#include "wii/utils/playlog.h"
//#include "wii/title.h"
extern uint8_t _binary_wii_save_bnr_banner_tpl_start[];
extern uint8_t _binary_wii_save_bnr_banner_tpl_end[];
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>

struct settings g_settings;
struct global g_extern;
struct defaults g_defaults;

const char *config_get_default_audio(void)
{
   switch (AUDIO_DEFAULT_DRIVER)
   {
      case AUDIO_WII:
         return "gx";
      case AUDIO_NULL:
         return "null";
   }

   return NULL;
}

const char *config_get_default_audio_resampler(void)
{
   switch (AUDIO_DEFAULT_RESAMPLER_DRIVER)
   {
      case AUDIO_RESAMPLER_CC:
         return "CC";
      case AUDIO_RESAMPLER_SINC:
         return "SINC";
      case AUDIO_RESAMPLER_NEAREST:
         return "Nearest";
   }

   return NULL;
}

const char *config_get_default_video(void)
{
   switch (VIDEO_DEFAULT_DRIVER)
   {
      case VIDEO_WII:
         return "gx";
   }

   return NULL;
}

const char *config_get_default_input(void)
{
   switch (INPUT_DEFAULT_DRIVER)
   {
      case INPUT_WII:
         return "gx";
      case INPUT_NULL:
         return "null";
   }

   return NULL;
}

const char *config_get_default_joypad(void)
{
   switch (JOYPAD_DEFAULT_DRIVER)
   {
      case JOYPAD_GX:
         return "gx";
   }

   return NULL;
}

#ifdef HAVE_MENU
const char *config_get_default_menu(void)
{
   switch (MENU_DEFAULT_DRIVER)
   {
      case MENU_RGUI:
         return "rgui";
   //   case MENU_RMENU:
     //    return "rmenu";
    //  case MENU_RMENU_XUI:
      //   return "rmenu_xui";
    //  case MENU_LAKKA:
      //   return "lakka";
     // case MENU_GLUI:
    //     return "glui";
    //  case MENU_XMB:
      //   return "xmb";
   }

   return "NULL";
}
#endif

const char *config_get_default_osk(void)
{
   switch (OSK_DEFAULT_DRIVER)
   {
      case OSK_NULL:
         return "null";
   }

   return NULL;
}

const char *config_get_default_camera(void)
{
   switch (CAMERA_DEFAULT_DRIVER)
   {
      case CAMERA_NULL:
         return "null";
   }

   return NULL;
}

const char *config_get_default_location(void)
{
   switch (LOCATION_DEFAULT_DRIVER)
   {
      case LOCATION_NULL:
         return "null";
   }

   return NULL;
}

static void config_set_defaults(void)
{
   unsigned i, j;
   const char *def_video = config_get_default_video();
   const char *def_audio = config_get_default_audio();
   const char *def_audio_resampler = config_get_default_audio_resampler();
   const char *def_input = config_get_default_input();
   const char *def_joypad = config_get_default_joypad();
#ifdef HAVE_MENU
   const char *def_menu  = config_get_default_menu();
#endif
   //const char *def_camera = config_get_default_camera();
   //const char *def_location = config_get_default_location();
   //const char *def_osk = config_get_default_osk();

  // if (def_camera)
   //   strlcpy(g_settings.camera.driver,
    //        def_camera, sizeof(g_settings.camera.driver));
  // if (def_location)
   //   strlcpy(g_settings.location.driver,
   //         def_location, sizeof(g_settings.location.driver));
   //if (def_osk)
    //  strlcpy(g_settings.osk.driver,
    //        def_osk, sizeof(g_settings.osk.driver));
   if (def_video)
      strlcpy(g_settings.video.driver,
            def_video, sizeof(g_settings.video.driver));
   if (def_audio)
      strlcpy(g_settings.audio.driver,
            def_audio, sizeof(g_settings.audio.driver));
   if (def_audio_resampler)
      strlcpy(g_settings.audio.resampler,
            def_audio_resampler, sizeof(g_settings.audio.resampler));
   if (def_input)
      strlcpy(g_settings.input.driver,
            def_input, sizeof(g_settings.input.driver));
   if (def_joypad)
      strlcpy(g_settings.input.joypad_driver,
            def_input, sizeof(g_settings.input.joypad_driver));
#ifdef HAVE_MENU
   if (def_menu)
      strlcpy(g_settings.menu.driver,
            def_menu,  sizeof(g_settings.menu.driver));
#endif

 //  g_settings.history_list_enable = def_history_list_enable;
 //  g_settings.load_dummy_on_core_shutdown = load_dummy_on_core_shutdown;

   //g_settings.video.scale = scale;
  // g_settings.video.fullscreen = g_extern.force_fullscreen ? true : fullscreen;
  // g_settings.video.windowed_fullscreen = windowed_fullscreen;
  // g_settings.video.monitor_index = monitor_index;
   //g_settings.video.fullscreen_x = fullscreen_x;
  // g_settings.video.fullscreen_y = fullscreen_y;
 //  g_settings.video.disable_composition = disable_composition;
   g_settings.video.vsync = vsync;
   g_settings.video.forcesync = video_forcesync;
  // g_settings.video.hard_sync = hard_sync;
  // g_settings.video.hard_sync_frames = hard_sync_frames;
   g_settings.video.frame_delay = frame_delay;
 //  g_settings.video.black_frame_insertion = black_frame_insertion;
  // g_settings.video.swap_interval = swap_interval;
  // g_settings.video.threaded = video_threaded;

   //if (g_defaults.settings.video_threaded_enable != video_threaded)
     // g_settings.video.threaded = g_defaults.settings.video_threaded_enable;

   //g_settings.video.shared_context = video_shared_context;
  // g_settings.video.force_srgb_disable = false;
#ifdef GEKKO
   g_settings.video.drawdone = video_drawdone;
   g_settings.video.viwidth = video_viwidth;
   g_settings.video.wide_viwidth = video_wide_viwidth;
#ifdef USE_TILED
   g_settings.video.tiled = video_tiled;
#endif
   g_settings.video.vfilter = video_vfilter;
   g_settings.video.dither = video_dither;
   g_settings.video.vbright = video_vbright;
   g_settings.video.vres = video_vres;
   g_settings.video.blendframe = video_blendframe;
   g_settings.video.prescale = video_prescale;
   g_settings.video.blend_smooth = video_blend_smooth;
   g_settings.video.autores = video_autores;
   g_settings.video.force_288p = video_force_288p;
   g_settings.video.wide_correction = video_wide_correction;
   g_settings.video.correct_amount = video_correct_amount;
#ifdef HAVE_RENDERSCALE
   g_settings.video.renderscale = video_renderscale;
   g_settings.video.top = video_top;
   g_settings.video.bottom = video_bottom;
   g_settings.video.left = video_left;
   g_settings.video.right = video_right;
#endif
#endif
   g_settings.video.smooth = video_smooth;
   g_settings.video.wide_smooth = video_wide_smooth;
   g_settings.video.menu_smooth = video_menu_smooth;
   g_settings.video.force_aspect = force_aspect;
   g_settings.video.scale_integer = scale_integer;
   g_settings.video.crop_overscan = crop_overscan;
   g_settings.video.aspect_ratio = aspect_ratio;
   g_settings.video.aspect_ratio_auto = aspect_ratio_auto; // Let implementation decide if automatic, or 1:1 PAR.
   g_settings.video.aspect_ratio_idx = aspect_ratio_idx;
   g_settings.video.shader_enable = shader_enable;
   g_settings.video.allow_rotate = allow_rotate;
   
   g_settings.state_slot = state_slot;
   g_settings.hover_color = hover_color;
   g_settings.text_color = text_color;
   g_settings.theme_preset = theme_preset;
   g_settings.menu_bg_clr = menu_bg_clr;
   g_settings.menu_solid = menu_solid;
   g_settings.menu_msg_clr = menu_msg_clr;
   g_settings.menu_fullscreen = menu_fullscreen;
   g_settings.hide_core = hide_core;
   g_settings.hide_states = hide_states;
   g_settings.hide_screenshot = hide_screenshot;
   g_settings.hide_resume = hide_resume;
   g_settings.hide_reset = hide_reset;
   g_settings.hide_exit = hide_exit;
   g_settings.hide_settings = hide_settings;
   g_settings.hide_cursor = hide_cursor;
   g_settings.hide_curr_state = hide_curr_state;
   g_settings.show_manuals = show_manuals;
   g_settings.title_posx = title_posx;
   g_settings.title_posy = title_posy;
   g_settings.item_posx = item_posx;
   g_settings.item_posy = item_posy;
   g_settings.clock_show = clock_show;
   g_settings.clock_posx = clock_posx;
  // g_settings.single_mode = false; // No point setting this here
  
   g_settings.particle_type = particle_type;
   g_settings.particle_speed = particle_speed;
   g_settings.particle_clr = particle_clr;

  // g_settings.video.font_enable = font_enable;
   //g_settings.video.font_size = font_size;
   //g_settings.video.msg_pos_x = message_pos_offset_x;
   //g_settings.video.msg_pos_y = message_pos_offset_y;
   
   g_settings.video.msg_color_r = ((message_color >> 16) & 0xff) / 255.0f;
   g_settings.video.msg_color_g = ((message_color >>  8) & 0xff) / 255.0f;
   g_settings.video.msg_color_b = ((message_color >>  0) & 0xff) / 255.0f;

   g_settings.video.refresh_rate = refresh_rate;

   if (g_defaults.settings.video_refresh_rate > 0.0 &&
         g_defaults.settings.video_refresh_rate != refresh_rate)
      g_settings.video.refresh_rate = g_defaults.settings.video_refresh_rate;

   g_settings.video.post_filter_record = post_filter_record;
   g_settings.video.gpu_record = gpu_record;
   //g_settings.video.gpu_screenshot = gpu_screenshot;
   g_settings.video.rotation = ORIENTATION_NORMAL;

   g_settings.audio.sinc_taps = sinc_taps;
   g_settings.audio.enable = audio_enable;
   //g_settings.audio.mute_frames = mute_frames;
   g_settings.audio.out_rate = out_rate;
  // g_settings.audio.block_frames = 0;
  // if (audio_device)
    //  strlcpy(g_settings.audio.device,
      //      audio_device, sizeof(g_settings.audio.device));

  // if (!g_defaults.settings.out_latency)
    //  g_defaults.settings.out_latency = out_latency;


 //  g_settings.audio.latency = g_defaults.settings.out_latency;
   g_settings.audio.sync = audio_sync;
   g_settings.audio.rate_control = rate_control;
   g_settings.audio.rate_control_delta = rate_control_delta;
   g_settings.audio.volume = audio_volume;
   g_extern.audio_data.volume_gain = db_to_gain(g_settings.audio.volume);

   g_settings.rewind_enable = rewind_enable;
   g_settings.rewind_buffer_size = rewind_buffer_size;
   g_settings.rewind_granularity = rewind_granularity;
   g_settings.slowmotion_ratio = slowmotion_ratio;
   g_settings.fastforward_ratio = fastforward_ratio;
   g_settings.fastforward_ratio_throttle_enable = fastforward_ratio_throttle_enable;
   //g_settings.pause_nonactive = pause_nonactive;
   g_settings.autosave_interval = autosave_interval;

   g_settings.block_sram_overwrite = block_sram_overwrite;
   g_settings.savestate_auto_index = savestate_auto_index;
   g_settings.regular_state_pause  = regular_state_pause;
   g_settings.stateload_pause      = stateload_pause;
   g_settings.savestate_auto_once  = savestate_auto_once;
   g_settings.savestate_auto_save  = savestate_auto_save;
   g_settings.savestate_auto_load  = savestate_auto_load;
   g_settings.network_cmd_enable   = network_cmd_enable;
   g_settings.network_cmd_port     = network_cmd_port;
   g_settings.stdin_cmd_enable     = stdin_cmd_enable;
  // g_settings.content_history_size    = default_content_history_size;
 //  g_settings.libretro_log_level   = libretro_log_level;

#ifdef HAVE_MENU
   g_settings.menu_show_start_screen = menu_show_start_screen;
   g_settings.menu.pause_libretro = true;
#endif

  // g_settings.location.allow = false;
  // g_settings.camera.allow = false;

   rarch_assert(sizeof(g_settings.input.binds[0]) >= sizeof(retro_keybinds_1));
   rarch_assert(sizeof(g_settings.input.binds[1]) >= sizeof(retro_keybinds_rest));
   memcpy(g_settings.input.binds[0], retro_keybinds_1, sizeof(retro_keybinds_1));
#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++) // 4 players
#endif
      memcpy(g_settings.input.binds[i], retro_keybinds_rest,
            sizeof(retro_keybinds_rest));

#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++) // 4 players
#endif
   {
      for (j = 0; j < RARCH_BIND_LIST_END; j++)
      {
         g_settings.input.autoconf_binds[i][j].joykey = NO_BTN;
         g_settings.input.autoconf_binds[i][j].joyaxis = AXIS_NONE;
      }
   }
   memset(g_settings.input.autoconfigured, 0,
         sizeof(g_settings.input.autoconfigured));

   /* Verify that binds are in proper order. */
#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++)
#endif
      for (j = 0; j < RARCH_BIND_LIST_END; j++)
         if (g_settings.input.binds[i][j].valid)
            rarch_assert(j == g_settings.input.binds[i][j].id);

   g_settings.input.menu_combos = menu_combos;
   g_settings.input.trigger_threshold = trigger_threshold;
   g_settings.input.key_profile = key_profile;
#ifdef HW_RVL
   g_settings.input.n3ds = n3ds;
   g_settings.input.n3ds_port = n3ds_port;
   g_settings.input.n3ds_a = n3ds_a;
   g_settings.input.n3ds_b = n3ds_b;
   g_settings.input.n3ds_x = n3ds_x;
   g_settings.input.n3ds_y = n3ds_y;
   g_settings.input.n3ds_l = n3ds_l;
   g_settings.input.n3ds_r = n3ds_r;
   g_settings.input.n3ds_start = n3ds_start;
   g_settings.input.n3ds_select = n3ds_select;
   g_settings.input.n3ds_up = n3ds_up;
   g_settings.input.n3ds_down = n3ds_down;
   g_settings.input.n3ds_left = n3ds_left;
   g_settings.input.n3ds_right = n3ds_right;
   g_settings.input.n3ds_stick_up = n3ds_stick_up;
   g_settings.input.n3ds_stick_down = n3ds_stick_down;
   g_settings.input.n3ds_stick_left = n3ds_stick_left;
   g_settings.input.n3ds_stick_right = n3ds_stick_right;
#endif
   g_settings.input.poll_rate = poll_rate;
   g_settings.input.rgui_reset = rgui_reset;
   g_settings.input.axis_threshold = axis_threshold;
   //g_settings.input.netplay_client_swap_input = netplay_client_swap_input;
   g_settings.input.turbo_period = turbo_period;
   g_settings.input.turbo_duty_cycle = turbo_duty_cycle;
   g_settings.input.home_should_exit = home_should_exit;
   
   g_settings.input.gc_once = false;

   g_settings.input.overlay_opacity = 0.7f;
   g_settings.input.overlay_scale = 1.0f;
   g_settings.input.autodetect_enable = input_autodetect_enable;
  // *g_settings.input.keyboard_layout = '\0';

#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++)
#endif
   {
      g_settings.input.joypad_map[i] = i;
      g_settings.input.analog_dpad_mode[i] = ANALOG_DPAD_LSTICK;
      if (!g_extern.has_set_libretro_device[i])
         g_settings.input.libretro_device[i] = RETRO_DEVICE_JOYPAD;
   }

   g_extern.console.screen.viewports.custom_vp.width = 0;
   g_extern.console.screen.viewports.custom_vp.height = 0;
   g_extern.console.screen.viewports.custom_vp.x = 0;
   g_extern.console.screen.viewports.custom_vp.y = 0;


   /* Make sure settings from other configs carry over into defaults 
    * for another config. */
   if (!g_extern.has_set_save_path)
      *g_extern.savefile_dir = '\0';
   if (!g_extern.has_set_state_path)
      *g_extern.savestate_dir = '\0';

  // *g_settings.libretro_info_path = '\0';
  // if (!g_extern.has_set_libretro_directory)
    //  *g_settings.libretro_directory = '\0';

   *g_settings.core_options_path = '\0';
   //*g_settings.content_history_path = '\0';
  // *g_settings.cheat_database = '\0';
  // *g_settings.cheat_settings_path = '\0';
  // *g_settings.resampler_directory = '\0';
   *g_settings.screenshot_directory = '\0';
   *g_settings.system_directory = '\0';
   *g_settings.extraction_directory = '\0';
   *g_settings.input.autoconfig_dir = '\0';
   *g_settings.input.overlay = '\0';
   *g_settings.content_directory = '\0';
  // *g_settings.assets_directory = '\0';
  // *g_settings.playlist_directory = '\0';
  // *g_settings.video.shader_path = '\0';
  // *g_settings.video.shader_dir = '\0';
   *g_settings.video.filter_dir = '\0';
   *g_settings.audio.filter_dir = '\0';
   *g_settings.video.softfilter_plugin = '\0';
   *g_settings.audio.dsp_plugin = '\0';
#ifdef HAVE_MENU
   *g_settings.menu_content_directory = '\0';
   *g_settings.menu_config_directory = '\0';
#endif
   g_settings.core_specific_config = default_core_specific_config;

   g_settings.user_language = 0;

   g_extern.console.sound.system_bgm_enable = false;
#ifdef RARCH_CONSOLE
   g_extern.console.screen.gamma_correction = DEFAULT_GAMMA;

   g_extern.console.screen.resolutions.current.id = 0;
   g_extern.console.sound.mode = SOUND_MODE_NORMAL;
#endif
   
   if (default_filter_dir)
      fill_pathname_expand_special(g_settings.video.filter_dir,
            default_filter_dir, sizeof(g_settings.video.filter_dir));

   if (default_dsp_filter_dir)
      fill_pathname_expand_special(g_settings.audio.filter_dir,
            default_dsp_filter_dir, sizeof(g_settings.audio.filter_dir));

   if (*g_defaults.video_filter_dir)
      strlcpy(g_settings.video.filter_dir,
            g_defaults.video_filter_dir, sizeof(g_settings.video.filter_dir));
   if (*g_defaults.audio_filter_dir)
      strlcpy(g_settings.audio.filter_dir,
            g_defaults.audio_filter_dir, sizeof(g_settings.audio.filter_dir));
   if (*g_defaults.assets_dir)
      strlcpy(g_settings.assets_directory,
            g_defaults.assets_dir, sizeof(g_settings.assets_directory));
  // if (*g_defaults.playlist_dir)
   //   strlcpy(g_settings.playlist_directory,
    //        g_defaults.playlist_dir, sizeof(g_settings.playlist_directory));
   if (*g_defaults.core_dir)
      fill_pathname_expand_special(g_settings.libretro_directory,
            g_defaults.core_dir, sizeof(g_settings.libretro_directory));
   if (*g_defaults.core_path)
      strlcpy(g_settings.libretro, g_defaults.core_path,
            sizeof(g_settings.libretro));
   //if (*g_defaults.core_info_dir)
    //  fill_pathname_expand_special(g_settings.libretro_info_path,
      //      g_defaults.core_info_dir, sizeof(g_settings.libretro_info_path));
#ifdef HAVE_OVERLAY
   if (*g_defaults.overlay_dir)
   {
      fill_pathname_expand_special(g_extern.overlay_dir,
            g_defaults.overlay_dir, sizeof(g_extern.overlay_dir));
      if (!*g_settings.input.overlay)
            fill_pathname_join(g_settings.input.overlay,
                  g_extern.overlay_dir,
                  "...", // this used to be nul.cfg, but there's no point in naming this
                  sizeof(g_settings.input.overlay));
   }
#endif
#ifdef HAVE_MENU
   if (*g_defaults.menu_config_dir)
      strlcpy(g_settings.menu_config_directory,
            g_defaults.menu_config_dir,
            sizeof(g_settings.menu_config_directory));
#endif
   if (*g_defaults.shader_dir)
      fill_pathname_expand_special(g_settings.video.shader_dir,
            g_defaults.shader_dir, sizeof(g_settings.video.shader_dir));
   if (*g_defaults.autoconfig_dir)
      strlcpy(g_settings.input.autoconfig_dir,
            g_defaults.autoconfig_dir,
            sizeof(g_settings.input.autoconfig_dir));

   if (!g_extern.has_set_state_path && *g_defaults.savestate_dir)
      strlcpy(g_extern.savestate_dir,
            g_defaults.savestate_dir, sizeof(g_extern.savestate_dir));
   if (!g_extern.has_set_save_path && *g_defaults.sram_dir)
      strlcpy(g_extern.savefile_dir,
            g_defaults.sram_dir, sizeof(g_extern.savefile_dir));
   if (*g_defaults.system_dir)
      strlcpy(g_settings.system_directory,
            g_defaults.system_dir, sizeof(g_settings.system_directory));
   if (*g_defaults.screenshot_dir)
      strlcpy(g_settings.screenshot_directory,
            g_defaults.screenshot_dir,
            sizeof(g_settings.screenshot_directory));
  // if (*g_defaults.resampler_dir)
    //  strlcpy(g_settings.resampler_directory,
      //      g_defaults.resampler_dir,
        //    sizeof(g_settings.resampler_directory));
	if (*g_defaults.extract_dir)
      strlcpy(g_settings.extraction_directory,
            g_defaults.extract_dir, sizeof(g_settings.extraction_directory));

   if (*g_defaults.config_path)
      fill_pathname_expand_special(g_extern.config_path,
            g_defaults.config_path, sizeof(g_extern.config_path));
   
   g_settings.config_save_on_exit = config_save_on_exit;

   /* Avoid reloading config on every content load */
   g_extern.block_config_read = default_block_config_read;
   
   
      // Redefine settings for title
#ifdef USE_TITLE
   g_settings.video.viwidth = 640;
   g_settings.video.wide_viwidth = 712;
//video_wide_correction = false;
//video_correct_amount = 480;

// prog detect this, but prescale should add some blur
   g_settings.video.vfilter = false;

//video_dither = true;
//g_settings.video.vbright = -22; //vfilter brightness
//video_vres = GX_RESOLUTIONS_640_480
//video_blendframe = true;
   g_settings.video.blend_smooth = false;
//   g_settings.video.smooth = false;
   g_settings.video.prescale = true;
//video_wide_smooth = false;
//video_autores = false; //for PCE, Genesis
//frame_delay = 1;

   // handle save banner
   //FILE* fd = NULL;
#if 1
   const char *titlepath;
   titlepath = title_get_path(); //should always be 29 characters
   char bannerPath[PATH_MAX] = {0};
   sprintf(bannerPath, "nand:%s/banner.bin", titlepath);

  // FILE* fd = fopen("nand:/title/00010001/454e4545/data/banner.bin", "rb");
   FILE* fd = fopen(bannerPath, "rb");
   if(!fd) {
   u8 header[0x72A0];
   memset(header, 0, sizeof header);
   memcpy(header, "WIBN", 4);
  // header[7] = 1; // no copy or move
   header[9] = 2; // no animation
   char sTitle[0x20] = {0};
   char sSub[0x20] = {0};
   snprintf(sTitle, 0x20, "NEMO");
   snprintf(sSub,   0x20, "Arcade");
   u8 i = 0;
   u8 pos = 0;
   for(i=1; i<0x40; i+=2) {
     header[0x20+i] = sTitle[pos];
     ++pos;
     if(pos > strlen(sTitle))
       break;
   }
   i = 0;
   pos = 0;
   for(i=1; i<0x40; i+=2) {
     header[0x60+i] = sSub[pos];
	 ++pos;
     if(pos > strlen(sSub))
       break;
   }
   
   //add banner and icon
   memcpy(&header[0xA0], _binary_wii_save_bnr_banner_tpl_start+0x80, 0x6000);
   memcpy(&header[0x60A0], _binary_wii_save_bnr_banner_tpl_start+0x6080, 0x1200);

#if 1
 //  const char *titlepath;
 //  titlepath = title_get_path(); //should always be 29 characters
   //sprintf(bannerPath, "%s/banner.bin", titlepath);
 //  sprintf(bannerPath, "%s/banner.bin", titlepath);

 //  sprintf(bannerPath, "nand:%s/banner.bin", titlepath);
   //fill_pathname_join(bannerPath, titlepath, "banner.bin", sizeof(bannerPath));

 //  s32 bnr_res = -1;
 //  bnr_res = ISFS_CreateFile(bannerPath, 0, 3, 3, 0);
   fd = fopen(bannerPath, "wb");
   fwrite(header, 1, sizeof header, fd);
   fclose(fd);
#endif
   } else
    fclose(fd);
#endif

#if 0
   // create log for nand
   char logPath[128] = {0};
   const char *titlepath;
   titlepath = title_get_path(); //should always be 29 characters
   sprintf(logPath, titlepath);
   s32 log_res = -1;
   log_res = ISFS_CreateFile("/shared2/lg.dat", 0, 3, 3, 0);
   if (log_res < 0) {
      ; //printf("FailedSAVE");
	  //NOTE: You cannot use IOS_Write to create a banner.bin, nor isfs.c impl
   } else {
      log_res = ISFS_Open("/shared2/lg.dat", 3);
	  ISFS_Write(log_res, logPath, sizeof(logPath));
	  ISFS_Close(log_res);
   }
#endif

   // Mimic the VC by reading the vc settings.sav, only if 480i is set
   u8 useDoubleStrike = 0;

   // Use one of the empty slots in vc settings.sav for the 3DS controller port
   g_settings.input.n3ds_port = 0;

   s32 pt_fd = -1;
   pt_fd = IOS_Open(_vcsettings_path, IPC_OPEN_READ);
   if(pt_fd < 0) {
     ; // oh well
   } else {
    vcSettings * vc_buf = memalign(32, ALIGN32(sizeof(vcSettings)));
    if(vc_buf) {
      if(IOS_Read(pt_fd, vc_buf, sizeof(vcSettings)) == sizeof(vcSettings)) {
        if(IOS_Seek(pt_fd, 0, 0) > -1) {
          // Using one of the empty slots for 3DS ctroller, only up to 2 players for now
          if(vc_buf->n3ds_port == 1)
              g_settings.input.n3ds_port = 1;
          
          // This should only be if 480p is OFF, but it's possible to have it ON without component
          if(!CONF_GetProgressiveScan() || !VIDEO_HaveComponentCable()) {
               useDoubleStrike = vc_buf->ds;
          }
        }
      }
	  free(vc_buf);
    }
     IOS_Close(pt_fd);
   }
   
   // Trap filter
   if(!VIDEO_HaveComponentCable())
       g_extern.console.softfilter_enable = true;

#if 0
   unsigned ogScale = 0;
   // check core.cfg for original scale setting, this is JUST for Sailor Moon
   //char cfgPath[128] = {0};
   //sprintf(cfgPath, "%score.cfg", dataPath);

  // const char _core_path[] __attribute__((aligned(32))) =
    //          "/title/00010001/45533245/data/core.cfg";

   fd = fopen("nand:/title/00010001/45533245/data/core.cfg", "rb");
  // sprintf(bannerPath, "nand:%s/core.cfg", titlepath);
   //fd = fopen(bannerPath, "rb");
   //fd = fopen("nand:/title/00010001/45533245/data/core.cfg", "rb");
   //pt_fd = IOS_Open("nand:/title/00010001/4a4d5045/data/core.cfg", IPC_OPEN_READ);
   if(fd) {
     fseek(fd, 0x14, SEEK_CUR);
	 fread(&ogScale, 1, 4, fd);
     ogScale &= 0xFF00;
	 if(ogScale != 0x5000)
       ogScale = 0;
     fclose(fd);
   }
#endif

#ifdef USE_NEMO
   unsigned viScale = 0;
   char cfgPath[PATH_MAX] = {0};
   sprintf(cfgPath, "nand:%s/core.cfg", titlepath);
   fd = fopen(cfgPath, "rb");
   if(fd) {
     fseek(fd, 0x18, SEEK_CUR);
	 fread(&viScale, 1, 4, fd);
     viScale &= 0xFF0000;
	 if(viScale != 0x4E0000)
       viScale = 0;
     fclose(fd);
   }
#endif

   if(useDoubleStrike) {
     g_settings.video.vres = GX_RESOLUTIONS_640_240;
     g_extern.console.screen.viewports.custom_vp.width = 640;
     g_extern.console.screen.viewports.custom_vp.height = 224;
     g_extern.console.screen.viewports.custom_vp.x = 0;
     g_extern.console.screen.viewports.custom_vp.y = 8;
#if 0
     if(ogScale) {
       g_extern.console.screen.viewports.custom_vp.height = 240;
       g_extern.console.screen.viewports.custom_vp.y = 0;
       g_settings.video.prescale = false;
       g_settings.video.smooth = false;
     }
#endif
#ifdef USE_NEMO
     if(viScale) {
       g_settings.video.vres = GX_RESOLUTIONS_384_240;
       g_extern.console.screen.viewports.custom_vp.width = 384;
       g_settings.video.prescale = false;
       g_settings.video.smooth = false;
     }
#endif
   }
   else {
    // g_settings.video.vres = GX_RESOLUTIONS_640_456;
#ifdef HW_RVL
     g_extern.console.screen.viewports.custom_vp.width = CONF_GetAspectRatio() == CONF_ASPECT_16_9 ? ((640*3)/4) : 640;
     g_extern.console.screen.viewports.custom_vp.x = CONF_GetAspectRatio() == CONF_ASPECT_16_9 ? ((640-480)/2) : 0;
#else
     g_extern.console.screen.viewports.custom_vp.width = 640;
     g_extern.console.screen.viewports.custom_vp.x = 0;
#endif
     g_extern.console.screen.viewports.custom_vp.height = 448;
     g_extern.console.screen.viewports.custom_vp.y = 16;

#if 0
     if(ogScale) {
       g_extern.console.screen.viewports.custom_vp.height = 480;
       g_extern.console.screen.viewports.custom_vp.y = 0;
       if(CONF_GetAspectRatio() != CONF_ASPECT_16_9) {
         g_settings.video.prescale = false;
         g_settings.video.smooth = false;
       }
     }
#endif
#ifdef USE_NEMO
     if(viScale && (CONF_GetAspectRatio() != CONF_ASPECT_16_9)) {
       g_settings.video.vres = GX_RESOLUTIONS_384_480;
       g_extern.console.screen.viewports.custom_vp.width = 384;
       g_settings.video.prescale = false;
       g_settings.video.smooth = false;
     }
#endif
   }

   g_settings.input.rgui_reset = false;
   g_settings.hide_states = true;
   //g_settings.hide_core = true;
   g_settings.hide_screenshot = true;
//hide_resume = true;
//hide_reset = true;
//hide_exit = true;
   g_settings.hide_settings = true;
//g_settings.hide_cursor = true;
//g_settings.show_manuals = true;
//g_settings.title_posx = 30;
//g_settings.title_posy = 12;
//g_settings.item_posx = 0;
   g_settings.item_posy = 4;
   g_settings.clock_posx = 238;
#ifdef USE_NEMO
   if(viScale && (CONF_GetAspectRatio() != CONF_ASPECT_16_9))
     g_settings.clock_posx = 300;
#endif
//g_settings.home_should_exit = true;
//g_settings.video_menu_smooth = true;
//hover_color = ;
//text_color = ;
//theme_preset = 1;
//menu_bg_clr = ;
//clock_show = false;
   g_settings.menu_solid = true; //if using prescale, enable this
   g_settings.menu_fullscreen = true;

   g_settings.particle_type = 5; // 1=snow, 2=close snow, 3=rain, 4=vortex, 5=stars
   g_settings.particle_speed = 2.0;
//g_settings.particle_clr = 55000;

   g_settings.fadein = true;
   g_settings.reset_fade = 0; // 1=fadeout, 2=fadeout/fadein
   g_settings.exit_fade = true;

//rewind_enable = true;
//rewind_buffer_size = 20<<20; //20MB, I guess GB, NES games
//rewind_granularity = 2;

//autosave_interval = 60; //seconds
   g_settings.savestate_auto_save = true;
   g_settings.savestate_auto_load = true;
//regular_state_pause = true; //shows menu after loading a state
//stateload_pause = true; //shows menu after auto-loading a state
   g_settings.savestate_auto_once = true; //alt method for loading auto-state, needed for PSSM

//scale_integer = true;
//aspect_ratio = ;

//audio_volume = 2.0;
   g_settings.audio.sinc_taps = 8; //bigger uses more memory
   g_settings.audio.out_rate = 32000;
//audio_enable = false;
//audio_sync = false;
//rate_control_delta = 0.010;

//menu_combos = 1;
//key_profile = 1;
//poll_rate = 0;
//trigger_threshold = 120; //GC triggers
//axis_threshold = 0.30;


   g_settings.input.autodetect_enable = false;

   g_settings.input.n3ds = true;
   //g_settings.input.n3ds_port = 0; //1=p2 -- switching to P2 requires L+R+Y in Menu
   g_settings.input.n3ds_a = 46;
   g_settings.input.n3ds_b = 45;
   g_settings.input.n3ds_x = 44;
   g_settings.input.n3ds_y = 43;
   g_settings.input.n3ds_l = 6;
   g_settings.input.n3ds_r = 7;

   g_settings.menu_show_start_screen = false;
   g_settings.config_save_on_exit = true;
#endif
}

static void parse_config_file(void);

static config_file_t *open_default_config_file(void)
{
   config_file_t *conf = NULL;

#if defined(_WIN32) && !defined(_XBOX)
   char conf_path[PATH_MAX];

   char app_path[PATH_MAX];
   fill_pathname_application_path(app_path, sizeof(app_path));
   fill_pathname_resolve_relative(conf_path, app_path,
         "retroarch.cfg", sizeof(conf_path));

   conf = config_file_new(conf_path);
   if (!conf)
   {
      const char *appdata = getenv("APPDATA");
      if (appdata)
      {
         fill_pathname_join(conf_path, appdata,
               "retroarch.cfg", sizeof(conf_path));
         conf = config_file_new(conf_path);
      }
   }

   /* Try to create a new config file. */
   if (!conf)
   {
      conf = config_file_new(NULL);
      bool saved = false;
      if (conf)
      {
         /* Since this is a clean config file, we can 
          * safely use config_save_on_exit. */
         fill_pathname_resolve_relative(conf_path, app_path,
               "retroarch.cfg", sizeof(conf_path));
         config_set_bool(conf, "config_save_on_exit", true);
         saved = config_file_write(conf, conf_path);
      }

      if (saved)
         RARCH_WARN("Created new config file in: \"%s\".\n", conf_path);
      else
      {
         /* WARN here to make sure user has a good chance of seeing it. */
         RARCH_ERR("Failed to create new config file in: \"%s\".\n",
               conf_path);
         config_file_free(conf);
         conf = NULL;
      }
   }

   if (conf)
      strlcpy(g_extern.config_path, conf_path,
            sizeof(g_extern.config_path));
#elif defined(OSX)
   char conf_path[PATH_MAX];
   const char *home = getenv("HOME");

   if (!home)
      return NULL;

   fill_pathname_join(conf_path, home,
         "Library/Application Support/RetroArch", sizeof(conf_path));
   path_mkdir(conf_path);
      
   fill_pathname_join(conf_path, conf_path,
         "retroarch.cfg", sizeof(conf_path));
   conf = config_file_new(conf_path);

   if (!conf)
   {
      conf = config_file_new(NULL);
      bool saved = false;
      if (conf)
      {
         config_set_bool(conf, "config_save_on_exit", true);
         saved = config_file_write(conf, conf_path);
      }
      
      if (saved)
         RARCH_WARN("Created new config file in: \"%s\".\n", conf_path);
      else
      {
         /* WARN here to make sure user has a good chance of seeing it. */
         RARCH_ERR("Failed to create new config file in: \"%s\".\n",
               conf_path);
         config_file_free(conf);
         conf = NULL;
      }
   }

   if (conf)
      strlcpy(g_extern.config_path, conf_path, sizeof(g_extern.config_path));

#elif !defined(__CELLOS_LV2__) && !defined(_XBOX)
   char conf_path[PATH_MAX];
   const char *xdg  = getenv("XDG_CONFIG_HOME");
   const char *home = getenv("HOME");

   /* XDG_CONFIG_HOME falls back to $HOME/.config. */
   if (xdg)
      fill_pathname_join(conf_path, xdg,
            "retroarch/retroarch.cfg", sizeof(conf_path));
   else if (home)
#ifdef __HAIKU__
      fill_pathname_join(conf_path, home,
            "config/settings/retroarch/retroarch.cfg", sizeof(conf_path));
#else
      fill_pathname_join(conf_path, home,
            ".config/retroarch/retroarch.cfg", sizeof(conf_path));
#endif

   if (xdg || home)
   {
      RARCH_LOG("Looking for config in: \"%s\".\n", conf_path);
      conf = config_file_new(conf_path);
   }

   /* Fallback to $HOME/.retroarch.cfg. */
   if (!conf && home)
   {
      fill_pathname_join(conf_path, home,
            ".retroarch.cfg", sizeof(conf_path));
      RARCH_LOG("Looking for config in: \"%s\".\n", conf_path);
      conf = config_file_new(conf_path);
   }

   /* Try to create a new config file. */
   if (!conf && (home || xdg))
   {
      /* XDG_CONFIG_HOME falls back to $HOME/.config. */
      if (xdg)
         fill_pathname_join(conf_path, xdg,
               "retroarch/retroarch.cfg", sizeof(conf_path));
      else if (home)
#ifdef __HAIKU__
         fill_pathname_join(conf_path, home,
               "config/settings/retroarch/retroarch.cfg", sizeof(conf_path));
#else
         fill_pathname_join(conf_path, home,
               ".config/retroarch/retroarch.cfg", sizeof(conf_path));
#endif

      char basedir[PATH_MAX];
      fill_pathname_basedir(basedir, conf_path, sizeof(basedir));

      if (path_mkdir(basedir))
      {
#ifndef GLOBAL_CONFIG_DIR
#if defined(__HAIKU__)
#define GLOBAL_CONFIG_DIR "/system/settings"
#else
#define GLOBAL_CONFIG_DIR "/etc"
#endif
#endif
         char skeleton_conf[PATH_MAX];
         fill_pathname_join(skeleton_conf, GLOBAL_CONFIG_DIR,
               "retroarch.cfg", sizeof(skeleton_conf));
         conf = config_file_new(skeleton_conf);
         if (conf)
            RARCH_WARN("Using skeleton config \"%s\" as base for a new config file.\n", skeleton_conf);
         else
            conf = config_file_new(NULL);

         bool saved = false;
         if (conf)
         {
            /* Since this is a clean config file, we can safely use config_save_on_exit. */
            config_set_bool(conf, "config_save_on_exit", true);
            saved = config_file_write(conf, conf_path);
         }

         if (saved)
            RARCH_WARN("Created new config file in: \"%s\".\n", conf_path);
         else
         {
            /* WARN here to make sure user has a good chance of seeing it. */
            RARCH_ERR("Failed to create new config file in: \"%s\".\n", conf_path);
            config_file_free(conf);
            conf = NULL;
         }
      }
   }

   if (conf)
      strlcpy(g_extern.config_path, conf_path, sizeof(g_extern.config_path));
#endif
   
   return conf;
}

static void config_read_keybinds_conf(config_file_t *conf);

/* Also dumps inherited values, useful for logging. */

static void config_file_dump_all(config_file_t *conf)
{
   struct config_include_list *includes = conf->includes;
   while (includes)
   {
      RARCH_LOG("#include \"%s\"\n", includes->path);
      includes = includes->next;
   }

   struct config_entry_list *list = conf->entries;
   while (list)
   {
      RARCH_LOG("%s = \"%s\" %s\n", list->key,
            list->value, list->readonly ? "(included)" : "");
      list = list->next;
   }
}

static bool config_load_file(const char *path, bool set_defaults)
{
   unsigned i;
   char *save, tmp_str[PATH_MAX];
   char tmp_append_path[PATH_MAX]; /* Don't destroy append_config_path. */
   const char *extra_path;
   unsigned msg_color = 0;
   config_file_t *conf = NULL;

   if (path)
   {
      conf = config_file_new(path);
      if (!conf)
         return false;
   }
   else
      conf = open_default_config_file();

   if (!conf)
      return true;

   if (set_defaults)
      config_set_defaults();

   strlcpy(tmp_append_path, g_extern.append_config_path,
         sizeof(tmp_append_path));
   extra_path = strtok_r(tmp_append_path, ",", &save);

   while (extra_path)
   {
      bool ret = false;
      RARCH_LOG("Appending config \"%s\"\n", extra_path);
      ret = config_append_file(conf, extra_path);
      if (!ret)
         RARCH_ERR("Failed to append config \"%s\"\n", extra_path);
      extra_path = strtok_r(NULL, ";", &save);
   }

   if (g_extern.verbosity)
   {
      RARCH_LOG_OUTPUT("=== Config ===\n");
      config_file_dump_all(conf);
      RARCH_LOG_OUTPUT("=== Config end ===\n");
   }


  // CONFIG_GET_FLOAT(video.scale, "video_scale");
   //CONFIG_GET_INT(video.fullscreen_x, "video_fullscreen_x");
 //  CONFIG_GET_INT(video.fullscreen_y, "video_fullscreen_y");

  // if (!g_extern.force_fullscreen)
    //  CONFIG_GET_BOOL(video.fullscreen, "video_fullscreen");

   //CONFIG_GET_BOOL(video.windowed_fullscreen, "video_windowed_fullscreen");
  // CONFIG_GET_INT(video.monitor_index, "video_monitor_index");
  // CONFIG_GET_BOOL(video.disable_composition, "video_disable_composition");
   CONFIG_GET_BOOL(video.vsync, "video_vsync");
   CONFIG_GET_BOOL(video.forcesync, "video_forcesync");
  // CONFIG_GET_BOOL(video.hard_sync, "video_hard_sync");

#ifdef HAVE_MENU
   CONFIG_GET_BOOL(menu.pause_libretro, "menu_pause_libretro");
#endif

 //  CONFIG_GET_INT(video.hard_sync_frames, "video_hard_sync_frames");
  // if (g_settings.video.hard_sync_frames > 3)
    //  g_settings.video.hard_sync_frames = 3;

   CONFIG_GET_INT(video.frame_delay, "video_frame_delay");
   if (g_settings.video.frame_delay > 15)
      g_settings.video.frame_delay = 15;

   //CONFIG_GET_BOOL(video.black_frame_insertion, "video_black_frame_insertion");
   //CONFIG_GET_INT(video.swap_interval, "video_swap_interval");
 //  g_settings.video.swap_interval = max(g_settings.video.swap_interval, 1);
   //g_settings.video.swap_interval = min(g_settings.video.swap_interval, 4);
   //CONFIG_GET_BOOL(video.threaded, "video_threaded");
  // CONFIG_GET_BOOL(video.shared_context, "video_shared_context");
#ifdef GEKKO
   CONFIG_GET_BOOL(video.drawdone, "video_drawdone");
   CONFIG_GET_INT(video.viwidth, "video_viwidth");
   CONFIG_GET_INT(video.wide_viwidth, "video_wide_viwidth");
#ifdef USE_TILED
   CONFIG_GET_BOOL(video.tiled, "video_tiled");
#endif
   CONFIG_GET_BOOL(video.vfilter, "video_vfilter");
   CONFIG_GET_BOOL(video.dither, "video_dither");
   CONFIG_GET_FLOAT(video.vbright, "video_vbright");
   CONFIG_GET_INT(video.vres, "video_vres");
   CONFIG_GET_BOOL(video.blendframe, "video_blendframe");
   CONFIG_GET_BOOL(video.prescale, "video_prescale");
   CONFIG_GET_BOOL(video.blend_smooth, "video_blend_smooth");
   CONFIG_GET_BOOL(video.autores, "video_autores");
   CONFIG_GET_BOOL(video.force_288p, "video_force_288p");
   CONFIG_GET_BOOL(video.wide_correction, "video_wide_correction");
   CONFIG_GET_INT(video.correct_amount, "video_correct_amount");
#ifdef HAVE_RENDERSCALE
   CONFIG_GET_INT(video.renderscale, "video_renderscale");
   CONFIG_GET_FLOAT(video.top, "video_top");
   CONFIG_GET_FLOAT(video.bottom, "video_bottom");
   CONFIG_GET_FLOAT(video.left, "video_left");
   CONFIG_GET_FLOAT(video.right, "video_right");
#endif
#endif
   CONFIG_GET_BOOL(video.smooth, "video_smooth");
   CONFIG_GET_BOOL(video.wide_smooth, "video_wide_smooth");
   CONFIG_GET_BOOL(video.menu_smooth, "video_menu_smooth");
   CONFIG_GET_BOOL(video.force_aspect, "video_force_aspect");
   CONFIG_GET_BOOL(video.scale_integer, "video_scale_integer");
   CONFIG_GET_BOOL(video.crop_overscan, "video_crop_overscan");
   CONFIG_GET_FLOAT(video.aspect_ratio, "video_aspect_ratio");
   CONFIG_GET_BOOL(video.aspect_ratio_auto, "video_aspect_ratio_auto");
#ifdef HW_RVL
   if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
      CONFIG_GET_INT(video.aspect_ratio_idx, "aspect_ratio_index_wide");
   else
#endif
      CONFIG_GET_INT(video.aspect_ratio_idx, "aspect_ratio_index");
   CONFIG_GET_FLOAT(video.refresh_rate, "video_refresh_rate");

   //CONFIG_GET_PATH(video.shader_path, "video_shader");
  // CONFIG_GET_BOOL(video.shader_enable, "video_shader_enable");

   CONFIG_GET_BOOL(video.allow_rotate, "video_allow_rotate");

  // CONFIG_GET_PATH(video.font_path, "video_font_path");
  // CONFIG_GET_FLOAT(video.font_size, "video_font_size");
  // CONFIG_GET_BOOL(video.font_enable, "video_font_enable");
  // CONFIG_GET_FLOAT(video.msg_pos_x, "video_message_pos_x");
  // CONFIG_GET_FLOAT(video.msg_pos_y, "video_message_pos_y");
   CONFIG_GET_INT(video.rotation, "video_rotation");

   CONFIG_GET_BOOL(video.force_srgb_disable, "video_force_srgb_disable");

#ifdef RARCH_CONSOLE
   /* TODO - will be refactored later to make it more clean - it's more 
    * important that it works for consoles right now */

   CONFIG_GET_BOOL_EXTERN(console.screen.gamma_correction, "gamma_correction");

  // config_get_bool(conf, "custom_bgm_enable",
    //     &g_extern.console.sound.system_bgm_enable);
 //  config_get_bool(conf, "flicker_filter_enable",
   //      &g_extern.console.flickerfilter_enable);
   config_get_bool(conf, "soft_filter_enable",
         &g_extern.console.softfilter_enable);

   config_get_bool(conf, "agb_yscale",
         &g_extern.console.agb_yscale);

   //CONFIG_GET_INT_EXTERN(console.screen.flicker_filter_index,
     //    "flicker_filter_index");
  // CONFIG_GET_INT_EXTERN(console.screen.soft_filter_index,
    //     "soft_filter_index");
  // CONFIG_GET_INT_EXTERN(console.screen.resolutions.current.id,
    //     "current_resolution_id");
  // CONFIG_GET_INT_EXTERN(console.sound.mode, "sound_mode");
#endif
   CONFIG_GET_INT(state_slot, "state_slot");
   CONFIG_GET_INT(hover_color, "hover_color");
   CONFIG_GET_INT(text_color, "text_color");
   CONFIG_GET_INT(theme_preset, "theme_preset");
   CONFIG_GET_INT(menu_bg_clr, "menu_bg_clr");
   CONFIG_GET_BOOL(menu_solid, "menu_solid");
   CONFIG_GET_INT(menu_msg_clr, "menu_msg_clr");
   CONFIG_GET_BOOL(menu_fullscreen, "menu_fullscreen");
   CONFIG_GET_BOOL(hide_core, "hide_core");
   CONFIG_GET_BOOL(hide_states, "hide_states");
   CONFIG_GET_BOOL(hide_screenshot, "hide_screenshot");
   CONFIG_GET_BOOL(hide_resume, "hide_resume");
   CONFIG_GET_BOOL(hide_reset, "hide_reset");
   CONFIG_GET_BOOL(hide_exit, "hide_exit");
   CONFIG_GET_BOOL(hide_settings, "hide_settings");
   CONFIG_GET_BOOL(hide_cursor, "hide_cursor");
   CONFIG_GET_BOOL(hide_curr_state, "hide_curr_state");
   CONFIG_GET_BOOL(show_manuals, "show_manuals");
   CONFIG_GET_INT(title_posx, "title_posx");
   CONFIG_GET_INT(title_posy, "title_posy");
   CONFIG_GET_INT(item_posx, "item_posx");
   CONFIG_GET_INT(item_posy, "item_posy");
   CONFIG_GET_INT(clock_posx, "clock_posx");
#ifdef HW_RVL
   if(CONF_GetAspectRatio() == CONF_ASPECT_16_9) {
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.x,
         "custom_viewport_wide_x");
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.y,
         "custom_viewport_wide_y");
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.width,
         "custom_viewport_wide_width");
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.height,
         "custom_viewport_wide_height");
   } else
#endif
 {
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.x,
         "custom_viewport_x");
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.y,
         "custom_viewport_y");
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.width,
         "custom_viewport_width");
   CONFIG_GET_INT_EXTERN(console.screen.viewports.custom_vp.height,
         "custom_viewport_height");
   }

   if (config_get_hex(conf, "video_message_color", &msg_color))
   {
      g_settings.video.msg_color_r = ((msg_color >> 16) & 0xff) / 255.0f;
      g_settings.video.msg_color_g = ((msg_color >>  8) & 0xff) / 255.0f;
      g_settings.video.msg_color_b = ((msg_color >>  0) & 0xff) / 255.0f;
   }

   CONFIG_GET_BOOL(video.post_filter_record, "video_post_filter_record");
   CONFIG_GET_BOOL(video.gpu_record, "video_gpu_record");
  // CONFIG_GET_BOOL(video.gpu_screenshot, "video_gpu_screenshot");

   //CONFIG_GET_PATH(video.shader_dir, "video_shader_dir");
   //if (!strcmp(g_settings.video.shader_dir, "default"))
     // *g_settings.video.shader_dir = '\0';

   CONFIG_GET_PATH(video.filter_dir, "video_filter_dir");
   if (!strcmp(g_settings.video.filter_dir, "default"))
      *g_settings.video.filter_dir = '\0';

   CONFIG_GET_PATH(audio.filter_dir, "audio_filter_dir");
   if (!strcmp(g_settings.audio.filter_dir, "default"))
      *g_settings.audio.filter_dir = '\0';

   CONFIG_GET_INT(input.menu_combos, "input_menu_combos");
   CONFIG_GET_INT(input.trigger_threshold, "input_trigger_threshold");
   CONFIG_GET_INT(input.key_profile, "input_key_profile");
#ifdef HW_RVL
   CONFIG_GET_BOOL(input.n3ds, "input_n3ds");
   CONFIG_GET_INT(input.n3ds_port, "input_n3ds_port");
   CONFIG_GET_INT(input.n3ds_a, "input_n3ds_a");
   CONFIG_GET_INT(input.n3ds_b, "input_n3ds_b");
   CONFIG_GET_INT(input.n3ds_x, "input_n3ds_x");
   CONFIG_GET_INT(input.n3ds_y, "input_n3ds_y");
   CONFIG_GET_INT(input.n3ds_l, "input_n3ds_l");
   CONFIG_GET_INT(input.n3ds_r, "input_n3ds_r");
   CONFIG_GET_INT(input.n3ds_start, "input_n3ds_start");
   CONFIG_GET_INT(input.n3ds_select, "input_n3ds_select");
   CONFIG_GET_INT(input.n3ds_up, "input_n3ds_up");
   CONFIG_GET_INT(input.n3ds_down, "input_n3ds_down");
   CONFIG_GET_INT(input.n3ds_left, "input_n3ds_left");
   CONFIG_GET_INT(input.n3ds_right, "input_n3ds_right");
   CONFIG_GET_INT(input.n3ds_stick_up, "input_n3ds_stick_up");
   CONFIG_GET_INT(input.n3ds_stick_down, "input_n3ds_stick_down");
   CONFIG_GET_INT(input.n3ds_stick_left, "input_n3ds_stick_left");
   CONFIG_GET_INT(input.n3ds_stick_right, "input_n3ds_stick_right");
#endif
   CONFIG_GET_INT(input.poll_rate, "input_poll_rate");
   CONFIG_GET_BOOL(input.rgui_reset, "input_rgui_reset");
   CONFIG_GET_FLOAT(input.axis_threshold, "input_axis_threshold");
   CONFIG_GET_BOOL(input.home_should_exit, "input_home_should_exit");
  // CONFIG_GET_BOOL(input.netplay_client_swap_input,
    //     "netplay_client_swap_input");

#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++)
#endif
   {
      char buf[64];
      snprintf(buf, sizeof(buf), "input_player%u_joypad_index", i + 1);
      CONFIG_GET_INT(input.joypad_map[i], buf);

      snprintf(buf, sizeof(buf), "input_player%u_analog_dpad_mode", i + 1);
      CONFIG_GET_INT(input.analog_dpad_mode[i], buf);

      if (!g_extern.has_set_libretro_device[i])
      {
         snprintf(buf, sizeof(buf), "input_libretro_device_p%u", i + 1);
         CONFIG_GET_INT(input.libretro_device[i], buf);
      }
   }

   /* Audio settings. */
   CONFIG_GET_BOOL(audio.enable, "audio_enable");
   CONFIG_GET_INT(audio.out_rate, "audio_out_rate");
   //CONFIG_GET_INT(audio.block_frames, "audio_block_frames");
  // CONFIG_GET_STRING(audio.device, "audio_device");
 //  CONFIG_GET_INT(audio.latency, "audio_latency");
   CONFIG_GET_BOOL(audio.sync, "audio_sync");
   CONFIG_GET_BOOL(audio.rate_control, "audio_rate_control");
   CONFIG_GET_FLOAT(audio.rate_control_delta, "audio_rate_control_delta");
   CONFIG_GET_FLOAT(audio.volume, "audio_volume");
   CONFIG_GET_STRING(audio.resampler, "audio_resampler");
   CONFIG_GET_INT(audio.sinc_taps, "audio_sinc_taps");
  // CONFIG_GET_INT(audio.mute_frames, "audio_mute_frames");
   g_extern.audio_data.volume_gain = db_to_gain(g_settings.audio.volume);

   //CONFIG_GET_STRING(camera.device, "camera_device");
   //CONFIG_GET_BOOL(camera.allow, "camera_allow");

 //  CONFIG_GET_BOOL(location.allow, "location_allow");
   CONFIG_GET_STRING(video.driver, "video_driver");
#ifdef HAVE_MENU
   CONFIG_GET_STRING(menu.driver, "menu_driver");
#endif
  // CONFIG_GET_STRING(video.context_driver, "video_context_driver");
   CONFIG_GET_STRING(audio.driver, "audio_driver");
   CONFIG_GET_PATH(video.softfilter_plugin, "video_filter");
   CONFIG_GET_PATH(audio.dsp_plugin, "audio_dsp_plugin");
   CONFIG_GET_STRING(input.driver, "input_driver");
   CONFIG_GET_STRING(input.joypad_driver, "input_joypad_driver");
  // CONFIG_GET_STRING(input.keyboard_layout, "input_keyboard_layout");

   if (!g_extern.has_set_libretro)
      CONFIG_GET_PATH(libretro, "libretro_path");
   if (!g_extern.has_set_libretro_directory)
      CONFIG_GET_PATH(libretro_directory, "libretro_directory");

   /* Safe-guard against older behavior. */
   if (path_is_directory(g_settings.libretro))
   {
      RARCH_WARN("\"libretro_path\" is a directory, using this for \"libretro_directory\" instead.\n");
      strlcpy(g_settings.libretro_directory, g_settings.libretro,
            sizeof(g_settings.libretro_directory));
      *g_settings.libretro = '\0';
   }

   g_extern.config_type = CONFIG_PER_CORE;

   CONFIG_GET_BOOL(fps_show, "fps_show");
  // CONFIG_GET_BOOL(load_dummy_on_core_shutdown, "load_dummy_on_core_shutdown");
   CONFIG_GET_BOOL(clock_show, "clock_show");
   CONFIG_GET_BOOL(fadein, "fadein");
   CONFIG_GET_INT(reset_fade, "reset_fade");
   CONFIG_GET_BOOL(exit_fade, "exit_fade");
   CONFIG_GET_INT(particle_type, "particle_type");
   CONFIG_GET_FLOAT(particle_speed, "particle_speed");
   CONFIG_GET_INT(particle_clr, "particle_clr");

  // CONFIG_GET_PATH(libretro_info_path, "libretro_info_path");

   CONFIG_GET_PATH(core_options_path, "core_options_path");
   CONFIG_GET_PATH(screenshot_directory, "screenshot_directory");
   if (*g_settings.screenshot_directory)
   {
      if (!strcmp(g_settings.screenshot_directory, "default"))
         *g_settings.screenshot_directory = '\0';
      else if (!path_is_directory(g_settings.screenshot_directory))
      {
         RARCH_WARN("screenshot_directory is not an existing directory, ignoring ...\n");
         *g_settings.screenshot_directory = '\0';
      }
   }

  // CONFIG_GET_PATH(resampler_directory, "resampler_directory");
   CONFIG_GET_PATH(extraction_directory, "extraction_directory");
   CONFIG_GET_PATH(content_directory, "content_directory");
  // CONFIG_GET_PATH(assets_directory, "assets_directory");
  // CONFIG_GET_PATH(playlist_directory, "playlist_directory");
  // if (!strcmp(g_settings.content_directory, "default"))
    //  *g_settings.content_directory = '\0';
  // if (!strcmp(g_settings.assets_directory, "default"))
    //  *g_settings.assets_directory = '\0';
  // if (!strcmp(g_settings.playlist_directory, "default"))
    //  *g_settings.playlist_directory = '\0';
#ifdef HAVE_MENU
   CONFIG_GET_PATH(menu_content_directory, "rgui_browser_directory");
   if (!strcmp(g_settings.menu_content_directory, "default"))
      *g_settings.menu_content_directory = '\0';
   CONFIG_GET_PATH(menu_config_directory, "rgui_config_directory");
   if (!strcmp(g_settings.menu_config_directory, "default"))
      *g_settings.menu_config_directory = '\0';
   CONFIG_GET_BOOL(menu_show_start_screen, "rgui_show_start_screen");
#endif
 //  CONFIG_GET_INT(libretro_log_level, "libretro_log_level");

  // if (!g_extern.has_set_verbosity)
  //    CONFIG_GET_BOOL_EXTERN(verbosity, "log_verbosity");

  // CONFIG_GET_BOOL_EXTERN(perfcnt_enable, "perfcnt_enable");

#ifdef HAVE_OVERLAY
   CONFIG_GET_PATH_EXTERN(overlay_dir, "overlay_directory");
   if (!strcmp(g_extern.overlay_dir, "default"))
      *g_extern.overlay_dir = '\0';

   CONFIG_GET_PATH(input.overlay, "input_overlay");
   CONFIG_GET_FLOAT(input.overlay_opacity, "input_overlay_opacity");
   CONFIG_GET_FLOAT(input.overlay_scale, "input_overlay_scale");
#endif

   CONFIG_GET_BOOL(rewind_enable, "rewind_enable");

   int buffer_size = 0;
   if (config_get_int(conf, "rewind_buffer_size", &buffer_size))
      g_settings.rewind_buffer_size = buffer_size * UINT64_C(1000000);

   CONFIG_GET_INT(rewind_granularity, "rewind_granularity");
   CONFIG_GET_FLOAT(slowmotion_ratio, "slowmotion_ratio");
   if (g_settings.slowmotion_ratio < 1.0f)
      g_settings.slowmotion_ratio = 1.0f;

   CONFIG_GET_FLOAT(fastforward_ratio, "fastforward_ratio");

   /* Sanitize fastforward_ratio value - previously range was -1
    * and up (with 0 being skipped) */
   if (g_settings.fastforward_ratio <= 0.0f)
      g_settings.fastforward_ratio = 1.0f;

   CONFIG_GET_BOOL(fastforward_ratio_throttle_enable, "fastforward_ratio_throttle_enable");

   //CONFIG_GET_BOOL(pause_nonactive, "pause_nonactive");
   CONFIG_GET_INT(autosave_interval, "autosave_interval");

  /* CONFIG_GET_PATH(cheat_database, "cheat_database_path");
   CONFIG_GET_PATH(cheat_settings_path, "cheat_settings_path");*/

   CONFIG_GET_BOOL(block_sram_overwrite, "block_sram_overwrite");
   CONFIG_GET_BOOL(savestate_auto_index, "savestate_auto_index");
   CONFIG_GET_BOOL(regular_state_pause,  "regular_state_pause");
   CONFIG_GET_BOOL(stateload_pause,      "stateload_pause");
   CONFIG_GET_BOOL(savestate_auto_once,  "savestate_auto_once");
   CONFIG_GET_BOOL(savestate_auto_save,  "savestate_auto_save");
   CONFIG_GET_BOOL(savestate_auto_load,  "savestate_auto_load");

   CONFIG_GET_BOOL(network_cmd_enable, "network_cmd_enable");
   CONFIG_GET_INT(network_cmd_port, "network_cmd_port");
   CONFIG_GET_BOOL(stdin_cmd_enable, "stdin_cmd_enable");

 /*  if (g_settings.playlist_directory[0] != '\0')
      fill_pathname_join(g_settings.content_history_path,
            g_settings.playlist_directory,
            "retroarch-content-history.txt",
            sizeof(g_settings.content_history_path));
   else
      fill_pathname_resolve_relative(g_settings.content_history_path,
            g_extern.config_path, "retroarch-content-history.txt",
            sizeof(g_settings.content_history_path));*/

  // CONFIG_GET_BOOL(history_list_enable, "history_list_enable");

 //  CONFIG_GET_PATH(content_history_path, "game_history_path");
  // CONFIG_GET_INT(content_history_size, "game_history_size");

   CONFIG_GET_INT(input.turbo_period, "input_turbo_period");
   CONFIG_GET_INT(input.turbo_duty_cycle, "input_duty_cycle");

   CONFIG_GET_BOOL(input.autodetect_enable, "input_autodetect_enable");
  // CONFIG_GET_PATH(input.autoconfig_dir, "joypad_autoconfig_dir");

  // if (!g_extern.has_set_username)
    //  CONFIG_GET_PATH(username, "netplay_nickname");
   CONFIG_GET_INT(user_language, "user_language");
#ifdef HAVE_NETPLAY
   if (!g_extern.has_set_netplay_mode)
      CONFIG_GET_BOOL_EXTERN(netplay_is_spectate,
            "netplay_spectator_mode_enable");
   if (!g_extern.has_set_netplay_mode)
      CONFIG_GET_BOOL_EXTERN(netplay_is_client, "netplay_mode");
   if (!g_extern.has_set_netplay_ip_address)
      CONFIG_GET_PATH_EXTERN(netplay_server, "netplay_ip_address");
   if (!g_extern.has_set_netplay_delay_frames)
      CONFIG_GET_INT_EXTERN(netplay_sync_frames, "netplay_delay_frames");
   if (!g_extern.has_set_netplay_ip_port)
      CONFIG_GET_INT_EXTERN(netplay_port, "netplay_ip_port");
#endif

   CONFIG_GET_BOOL(config_save_on_exit, "config_save_on_exit");
   
   CONFIG_GET_INT(state_slot, "state_slot");
   //CONFIG_GET_BOOL(hide_core, "hide_core");
   //CONFIG_GET_BOOL(hide_states, "hide_states");
   //CONFIG_GET_BOOL(hide_screenshot, "hide_screenshot");

   if (!g_extern.has_set_save_path &&
         config_get_path(conf, "savefile_directory", tmp_str, sizeof(tmp_str)))
   {
      if (!strcmp(tmp_str, "default"))
         strlcpy(g_extern.savefile_dir, g_defaults.sram_dir,
               sizeof(g_extern.savefile_dir));
      else if (path_is_directory(tmp_str))
      {
         strlcpy(g_extern.savefile_dir, tmp_str,
               sizeof(g_extern.savefile_dir));
         strlcpy(g_extern.savefile_name, tmp_str,
               sizeof(g_extern.savefile_name));
         fill_pathname_dir(g_extern.savefile_name, g_extern.basename,
               ".srm", sizeof(g_extern.savefile_name));
      }
      else
         RARCH_WARN("savefile_directory is not a directory, ignoring ...\n");
   }

   if (!g_extern.has_set_state_path &&
         config_get_path(conf, "savestate_directory", tmp_str, sizeof(tmp_str)))
   {
      if (!strcmp(tmp_str, "default"))
         strlcpy(g_extern.savestate_dir, g_defaults.savestate_dir,
               sizeof(g_extern.savestate_dir));
      else if (path_is_directory(tmp_str))
      {
         strlcpy(g_extern.savestate_dir, tmp_str,
               sizeof(g_extern.savestate_dir));
         strlcpy(g_extern.savestate_name, tmp_str,
               sizeof(g_extern.savestate_name));
         fill_pathname_dir(g_extern.savestate_name, g_extern.basename,
               ".state", sizeof(g_extern.savestate_name));
      }
      else
         RARCH_WARN("savestate_directory is not a directory, ignoring ...\n");
   }

   if (!config_get_path(conf, "system_directory",
            g_settings.system_directory, sizeof(g_settings.system_directory)))
   {
      RARCH_WARN("system_directory is not set in config. Assuming system directory is same folder as game: \"%s\".\n",
            g_settings.system_directory);
   }

   if (!strcmp(g_settings.system_directory, "default"))
      *g_settings.system_directory = '\0';

   config_read_keybinds_conf(conf);

   CONFIG_GET_BOOL(core_specific_config, "core_specific_config");

   config_file_free(conf);
   return true;
}

static void config_load_core_specific(void)
{
   *g_extern.core_specific_config_path = '\0';

   if (!*g_settings.libretro
#ifdef HAVE_DYNAMIC
      || g_extern.libretro_dummy
#endif
      )
      return;

#ifdef HAVE_MENU
   if (*g_settings.menu_config_directory)
   {
      path_resolve_realpath(g_settings.menu_config_directory,
            sizeof(g_settings.menu_config_directory));
      strlcpy(g_extern.core_specific_config_path,
            g_settings.menu_config_directory,
            sizeof(g_extern.core_specific_config_path));
   }
   else
#endif
   {
      /* Use original config file's directory as a fallback. */
      fill_pathname_basedir(g_extern.core_specific_config_path,
            g_extern.config_path, sizeof(g_extern.core_specific_config_path));
   }

   fill_pathname_dir(g_extern.core_specific_config_path, g_settings.libretro,
         ".cfg", sizeof(g_extern.core_specific_config_path));

   if (g_settings.core_specific_config)
   {
      char tmp[PATH_MAX];
      strlcpy(tmp, g_settings.libretro, sizeof(tmp));
      RARCH_LOG("Loading core-specific config from: %s.\n",
            g_extern.core_specific_config_path);

      if (!config_load_file(g_extern.core_specific_config_path, true))
         RARCH_WARN("Core-specific config not found, reusing last config.\n");

      /* Force some parameters which are implied when using core specific configs.
       * Don't have the core config file overwrite the libretro path. */
      strlcpy(g_settings.libretro, tmp, sizeof(g_settings.libretro));

      /* This must be true for core specific configs. */
      g_settings.core_specific_config = true;
   }
}

static void parse_config_file(void)
{
   bool ret;
   if (*g_extern.config_path)
   {
      RARCH_LOG("Loading config from: %s.\n", g_extern.config_path);
      ret = config_load_file(g_extern.config_path, false);
   }
   else
   {
      RARCH_LOG("Loading default config.\n");
      ret = config_load_file(NULL, false);
      if (*g_extern.config_path)
         RARCH_LOG("Found default config: %s.\n", g_extern.config_path);
   }

   if (!ret)
   {
      RARCH_ERR("Couldn't find config at path: \"%s\"\n",
            g_extern.config_path);
   }
}


static void read_keybinds_keyboard(config_file_t *conf, unsigned player,
      unsigned idx, struct retro_keybind *bind)
{
   if (input_config_bind_map[idx].valid && input_config_bind_map[idx].base)
   {
      const char *prefix = input_config_get_prefix(player,
            input_config_bind_map[idx].meta);
      if (prefix)
         input_config_parse_key(conf, prefix,
               input_config_bind_map[idx].base, bind);
   }
}

static void read_keybinds_button(config_file_t *conf, unsigned player,
      unsigned idx, struct retro_keybind *bind)
{
   if (input_config_bind_map[idx].valid && input_config_bind_map[idx].base)
   {
      const char *prefix = input_config_get_prefix(player,
            input_config_bind_map[idx].meta);
      if (prefix)
         input_config_parse_joy_button(conf, prefix,
               input_config_bind_map[idx].base, bind);
   }
}

static void read_keybinds_axis(config_file_t *conf, unsigned player,
      unsigned idx, struct retro_keybind *bind)
{
   if (input_config_bind_map[idx].valid && input_config_bind_map[idx].base)
   {
      const char *prefix = input_config_get_prefix(player,
            input_config_bind_map[idx].meta);
      if (prefix)
         input_config_parse_joy_axis(conf, prefix,
               input_config_bind_map[idx].base, bind);
   }
}

static void read_keybinds_player(config_file_t *conf, unsigned player)
{
   unsigned i;
   for (i = 0; input_config_bind_map[i].valid; i++)
   {
      struct retro_keybind *bind = (struct retro_keybind*)
         &g_settings.input.binds[player][i];

      if (!bind->valid)
         continue;

      read_keybinds_keyboard(conf, player, i, bind);
      read_keybinds_button(conf, player, i, bind);
      read_keybinds_axis(conf, player, i, bind);
   }
}

static void config_read_keybinds_conf(config_file_t *conf)
{
   unsigned i;
#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++)
#endif
      read_keybinds_player(conf, i);
}

#if 0
static bool config_read_keybinds(const char *path)
{
   config_file_t *conf = (config_file_t*)config_file_new(path);

   if (!conf)
      return false;

   config_read_keybinds_conf(conf);
   config_file_free(conf);

   return true;
}
#endif

static void save_keybind_key(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind)
{
   char key[64], btn[64];

   snprintf(key, sizeof(key), "%s_%s", prefix, base);
   input_translate_rk_to_str(bind->key, btn, sizeof(btn));
   config_set_string(conf, key, btn);
}

static void save_keybind_hat(config_file_t *conf, const char *key,
      const struct retro_keybind *bind)
{
   char config[16];
   unsigned hat = GET_HAT(bind->joykey);
   const char *dir = NULL;

   switch (GET_HAT_DIR(bind->joykey))
   {
      case HAT_UP_MASK:
         dir = "up";
         break;

      case HAT_DOWN_MASK:
         dir = "down";
         break;

      case HAT_LEFT_MASK:
         dir = "left";
         break;

      case HAT_RIGHT_MASK:
         dir = "right";
         break;

      default:
         rarch_assert(0);
   }

   snprintf(config, sizeof(config), "h%u%s", hat, dir);
   config_set_string(conf, key, config);
}

static void save_keybind_joykey(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind)
{
   char key[64];
   snprintf(key, sizeof(key), "%s_%s_btn", prefix, base);

   if (bind->joykey == NO_BTN)
      config_set_string(conf, key, "nul");
   else if (GET_HAT_DIR(bind->joykey))
      save_keybind_hat(conf, key, bind);
   else
      config_set_uint64(conf, key, bind->joykey);
}

static void save_keybind_axis(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind)
{
   char key[64];
   unsigned axis = 0;
   char dir = '\0';

   snprintf(key, sizeof(key), "%s_%s_axis", prefix, base);

   if (bind->joyaxis == AXIS_NONE)
      config_set_string(conf, key, "nul");
   else if (AXIS_NEG_GET(bind->joyaxis) != AXIS_DIR_NONE)
   {
      dir = '-';
      axis = AXIS_NEG_GET(bind->joyaxis);
   }
   else if (AXIS_POS_GET(bind->joyaxis) != AXIS_DIR_NONE)
   {
      dir = '+';
      axis = AXIS_POS_GET(bind->joyaxis);
   }

   if (dir)
   {
      char config[16];
      snprintf(config, sizeof(config), "%c%u", dir, axis);
      config_set_string(conf, key, config);
   }
}

static void save_keybind(config_file_t *conf, const char *prefix,
      const char *base, const struct retro_keybind *bind)
{
   if (!bind->valid)
      return;

   save_keybind_key(conf, prefix, base, bind);
   save_keybind_joykey(conf, prefix, base, bind);
   save_keybind_axis(conf, prefix, base, bind);
}

static void save_keybinds_player(config_file_t *conf, unsigned player)
{
   unsigned i = 0;
   for (i = 0; input_config_bind_map[i].valid; i++)
   {
      const char *prefix = input_config_get_prefix(player,
            input_config_bind_map[i].meta);
      if (prefix)
         save_keybind(conf, prefix, input_config_bind_map[i].base,
               &g_settings.input.binds[player][i]);
   }
}

static void calculate_specific_config_path(const char *in_basename)
{
   if (*g_settings.menu_config_directory)
      strlcpy(g_extern.specific_config_path, g_settings.menu_config_directory, sizeof(g_extern.specific_config_path));
   else
   {
      // Use original config file's directory as a fallback.
      fill_pathname_basedir(g_extern.specific_config_path, g_extern.config_path, sizeof(g_extern.specific_config_path));
   }
   
   fill_pathname_dir(g_extern.specific_config_path, in_basename, ".cfg", sizeof(g_extern.specific_config_path));
}

void config_load(void)
{
   /* Flush out per-core configs before loading a new config. */
   if (*g_extern.core_specific_config_path &&
         g_settings.config_save_on_exit && g_settings.core_specific_config && g_extern.config_type == CONFIG_PER_CORE)
      config_save_file(g_extern.core_specific_config_path);

   if (g_settings.config_save_on_exit)
   {
      /* Flush out configs before loading a new one. */
      if (*g_extern.specific_config_path && g_extern.config_type != CONFIG_PER_CORE)
         config_save_file(g_extern.specific_config_path);
   }

   if (!g_extern.block_config_read)
   {
      config_set_defaults();
      parse_config_file();
   }

   /* Per-core config handling. */
   config_load_core_specific();


   /* Reset to default values */
   *g_extern.specific_config_path = '\0';
    g_extern.config_type = CONFIG_PER_CORE;
   
   /* First try per-game config (requires game already loaded) */
   if (*g_extern.basename)
   {
      calculate_specific_config_path(g_extern.basename);
     // RARCH_LOG("Loading game-specific cfg from: %s.\n", g_extern.specific_config_path);
      if (config_load_file(g_extern.specific_config_path, true))
         g_extern.config_type = CONFIG_PER_GAME;
   }
   
   /* Try the per-core config if the per-game was unsuccessful */
  /* if (*g_settings.libretro && g_extern.config_type != CONFIG_PER_GAME)
   {
      calculate_specific_config_path(g_settings.libretro);
      RARCH_LOG("Loading core-specific config from: %s.\n", g_extern.specific_config_path);
      if (!config_load_file(g_extern.specific_config_path, true))
         RARCH_WARN("Core-specific config not found, using defaults config.\n");
   } */
   
   /* if reached this point and no config was loaded, defaults are used. 
    * also the per-core setup is left so next time it gets saved as is. */
}

bool config_save_file(const char *path)
{
   unsigned i = 0;
   bool ret = false;
   config_file_t *conf = config_file_new(path);

   if (!conf)
      conf = config_file_new(NULL);

   if (!conf)
      return false;

   RARCH_LOG("Saving config at path: \"%s\"\n", path);

   config_set_int(conf, "input_menu_combos",
         g_settings.input.menu_combos);
   config_set_int(conf, "input_trigger_threshold",
         g_settings.input.trigger_threshold);
   config_set_int(conf, "input_key_profile",
         g_settings.input.key_profile);
#ifdef HW_RVL
   config_set_bool(conf, "input_n3ds",
         g_settings.input.n3ds);
   config_set_int(conf, "input_n3ds_port",
         g_settings.input.n3ds_port); //a,b,x,y,l,r,st,sl,up
   config_set_int(conf, "input_n3ds_a",
         g_settings.input.n3ds_a);
   config_set_int(conf, "input_n3ds_b",
         g_settings.input.n3ds_b);
   config_set_int(conf, "input_n3ds_x",
         g_settings.input.n3ds_x);
   config_set_int(conf, "input_n3ds_y",
         g_settings.input.n3ds_y);
   config_set_int(conf, "input_n3ds_l",
         g_settings.input.n3ds_l);
   config_set_int(conf, "input_n3ds_r",
         g_settings.input.n3ds_r);
   config_set_int(conf, "input_n3ds_start",
         g_settings.input.n3ds_start);
   config_set_int(conf, "input_n3ds_select",
         g_settings.input.n3ds_select);
   config_set_int(conf, "input_n3ds_up",
         g_settings.input.n3ds_up);
   config_set_int(conf, "input_n3ds_down",
         g_settings.input.n3ds_down);
   config_set_int(conf, "input_n3ds_left",
         g_settings.input.n3ds_left);
   config_set_int(conf, "input_n3ds_right",
         g_settings.input.n3ds_right);
   config_set_int(conf, "input_n3ds_stick_up",
         g_settings.input.n3ds_stick_up);
   config_set_int(conf, "input_n3ds_stick_down",
         g_settings.input.n3ds_stick_down);
   config_set_int(conf, "input_n3ds_stick_left",
         g_settings.input.n3ds_stick_left);
   config_set_int(conf, "input_n3ds_stick_right",
         g_settings.input.n3ds_stick_right);
   //finally...
#endif
   config_set_int(conf, "input_poll_rate",
         g_settings.input.poll_rate);
   config_set_bool(conf,  "input_rgui_reset",
         g_settings.input.rgui_reset);
   config_set_float(conf, "input_axis_threshold",
         g_settings.input.axis_threshold);
   config_set_bool(conf,  "input_home_should_exit",
         g_settings.input.home_should_exit);

  // config_set_bool(conf,  "load_dummy_on_core_shutdown",
    //     g_settings.load_dummy_on_core_shutdown);
   config_set_bool(conf,  "fps_show", g_settings.fps_show);
   config_set_bool(conf,  "clock_show", g_settings.clock_show);
   config_set_bool(conf,  "fadein", g_settings.fadein);
   config_set_int(conf,  "reset_fade", g_settings.reset_fade);
   config_set_bool(conf,  "exit_fade", g_settings.exit_fade);
   
   config_set_int(conf,  "particle_type", g_settings.particle_type);
   config_set_float(conf,"particle_speed", g_settings.particle_speed);
   config_set_int(conf,  "particle_clr", g_settings.particle_clr);

   config_set_path(conf,  "libretro_path", g_settings.libretro);
   config_set_path(conf,  "libretro_directory", g_settings.libretro_directory);
  // config_set_path(conf,  "libretro_info_path", g_settings.libretro_info_path);
  // config_set_path(conf,  "cheat_database_path", g_settings.cheat_database);
   config_set_bool(conf,  "rewind_enable", g_settings.rewind_enable);
  // config_set_int(conf,   "audio_latency", g_settings.audio.latency);
   config_set_bool(conf,  "audio_sync",    g_settings.audio.sync);
  // config_set_int(conf,   "audio_block_frames", g_settings.audio.block_frames);
   config_set_int(conf,   "rewind_granularity", g_settings.rewind_granularity);
  // config_set_path(conf,  "video_shader", g_settings.video.shader_path);
   //config_set_bool(conf,  "video_shader_enable",
     //    g_settings.video.shader_enable);
   config_set_float(conf, "video_aspect_ratio", g_settings.video.aspect_ratio);
 //  config_set_bool(conf,  "video_windowed_fullscreen",
   //      g_settings.video.windowed_fullscreen);
  // config_set_float(conf, "video_scale", g_settings.video.scale);
   config_set_int(conf,   "autosave_interval", g_settings.autosave_interval);
   config_set_bool(conf,  "video_crop_overscan", g_settings.video.crop_overscan);
   config_set_bool(conf,  "video_scale_integer", g_settings.video.scale_integer);
#ifdef GEKKO
   config_set_bool(conf,  "video_drawdone", g_settings.video.drawdone);
   config_set_int(conf,   "video_viwidth", g_settings.video.viwidth);
   config_set_int(conf,   "video_wide_viwidth", g_settings.video.wide_viwidth);
#ifdef USE_TILED
   config_set_bool(conf,   "video_tiled", g_settings.video.tiled);
#endif
   config_set_bool(conf,  "video_vfilter", g_settings.video.vfilter);
   config_set_bool(conf,  "video_dither", g_settings.video.dither);
   config_set_float(conf, "video_vbright", g_settings.video.vbright);
   config_set_int(conf,   "video_vres", g_settings.video.vres);
   config_set_bool(conf,  "video_blendframe", g_settings.video.blendframe);
   config_set_bool(conf,  "video_prescale", g_settings.video.prescale);
   config_set_bool(conf,  "video_blend_smooth", g_settings.video.blend_smooth);
   config_set_bool(conf,  "video_autores", g_settings.video.autores);
   config_set_bool(conf,  "video_force_288p", g_settings.video.force_288p);
   config_set_bool(conf,  "video_wide_correction", g_settings.video.wide_correction);
   config_set_int(conf,  "video_correct_amount", g_settings.video.correct_amount);
#ifdef HAVE_RENDERSCALE
   config_set_int(conf,   "video_renderscale", g_settings.video.renderscale);
   config_set_float(conf,  "video_top", g_settings.video.top);
   config_set_float(conf,  "video_bottom", g_settings.video.bottom);
   config_set_float(conf,  "video_left", g_settings.video.left);
   config_set_float(conf,  "video_right", g_settings.video.right);
#endif
#endif
   config_set_bool(conf,  "video_smooth", g_settings.video.smooth);
   config_set_bool(conf,  "video_wide_smooth", g_settings.video.wide_smooth);
   config_set_bool(conf,  "video_menu_smooth", g_settings.video.menu_smooth);
#ifdef HW_RVL
   if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
     config_set_int(conf, "aspect_ratio_index_wide", g_settings.video.aspect_ratio_idx);
   else
#endif
     config_set_int(conf, "aspect_ratio_index", g_settings.video.aspect_ratio_idx);
  // config_set_bool(conf,  "video_threaded", g_settings.video.threaded);
   //config_set_bool(conf,  "video_shared_context",
     //    g_settings.video.shared_context);
  // config_set_bool(conf,  "video_force_srgb_disable",
    //     g_settings.video.force_srgb_disable);
 //  config_set_bool(conf,  "video_fullscreen", g_settings.video.fullscreen);
   config_set_float(conf, "video_refresh_rate", g_settings.video.refresh_rate);
  // config_set_int(conf,   "video_monitor_index",
    //     g_settings.video.monitor_index);
  // config_set_int(conf,   "video_fullscreen_x", g_settings.video.fullscreen_x);
  // config_set_int(conf,   "video_fullscreen_y", g_settings.video.fullscreen_y);
   config_set_string(conf,"video_driver", g_settings.video.driver);
#ifdef HAVE_MENU
   config_set_string(conf,"menu_driver", g_settings.menu.driver);
   config_set_bool(conf,"menu_pause_libretro", g_settings.menu.pause_libretro);
#endif
   config_set_bool(conf,  "video_vsync", g_settings.video.vsync);
   config_set_bool(conf,  "video_forcesync", g_settings.video.forcesync);
  // config_set_bool(conf,  "video_hard_sync", g_settings.video.hard_sync);
  // config_set_int(conf,   "video_hard_sync_frames",
    //     g_settings.video.hard_sync_frames);
   config_set_int(conf,   "video_frame_delay", g_settings.video.frame_delay);
   //config_set_bool(conf,  "video_black_frame_insertion",
     //    g_settings.video.black_frame_insertion);
  // config_set_bool(conf,  "video_disable_composition",
    //     g_settings.video.disable_composition);
  // config_set_bool(conf,  "pause_nonactive", g_settings.pause_nonactive);
  // config_set_int(conf, "video_swap_interval", g_settings.video.swap_interval);
  // config_set_bool(conf, "video_gpu_screenshot", g_settings.video.gpu_screenshot);
   config_set_int(conf, "video_rotation", g_settings.video.rotation);
   config_set_path(conf, "screenshot_directory",
         *g_settings.screenshot_directory ?
         g_settings.screenshot_directory : "default");
  // config_set_string(conf, "audio_device", g_settings.audio.device);
   config_set_string(conf, "video_filter", g_settings.video.softfilter_plugin);
   config_set_string(conf, "audio_dsp_plugin", g_settings.audio.dsp_plugin);
  // config_set_string(conf, "camera_device", g_settings.camera.device);
  // config_set_bool(conf, "camera_allow", g_settings.camera.allow);
   config_set_bool(conf, "audio_rate_control", g_settings.audio.rate_control);
   config_set_float(conf, "audio_rate_control_delta",
         g_settings.audio.rate_control_delta);
   config_set_float(conf, "audio_volume", g_settings.audio.volume);
  // config_set_string(conf, "video_context_driver", g_settings.video.context_driver);
   config_set_string(conf, "audio_driver", g_settings.audio.driver);
   config_set_bool(conf, "audio_enable", g_settings.audio.enable);
   config_set_int(conf, "audio_out_rate", g_settings.audio.out_rate);

  // config_set_bool(conf, "location_allow", g_settings.location.allow);

  // config_set_float(conf, "video_font_size", g_settings.video.font_size);
  // config_set_bool(conf,  "video_font_enable", g_settings.video.font_enable);

   config_set_path(conf, "system_directory",
         *g_settings.system_directory ?
         g_settings.system_directory : "default");
   config_set_path(conf, "extraction_directory",
         g_settings.extraction_directory);
  // config_set_path(conf, "resampler_directory",
    //     g_settings.resampler_directory);
   config_set_string(conf, "audio_resampler", g_settings.audio.resampler);
   config_set_int(conf, "audio_sinc_taps", g_settings.audio.sinc_taps);
   //config_set_int(conf, "audio_mute_frames", g_settings.audio.mute_frames);
   config_set_path(conf, "savefile_directory",
         *g_extern.savefile_dir ? g_extern.savefile_dir : "default");
   config_set_path(conf, "savestate_directory",
         *g_extern.savestate_dir ? g_extern.savestate_dir : "default");
  // config_set_path(conf, "video_shader_dir",
    //     *g_settings.video.shader_dir ?
      //   g_settings.video.shader_dir : "default");
   config_set_path(conf, "video_filter_dir",
         *g_settings.video.filter_dir ?
         g_settings.video.filter_dir : "default");
   config_set_path(conf, "audio_filter_dir",
         *g_settings.audio.filter_dir ?
         g_settings.audio.filter_dir : "default");

  // config_set_path(conf, "content_directory",
    //     *g_settings.content_directory ?
      //   g_settings.content_directory : "default");
  // config_set_path(conf, "assets_directory",
    //     *g_settings.assets_directory ?
      //   g_settings.assets_directory : "default");
  // config_set_path(conf, "playlist_directory",
    //     *g_settings.playlist_directory ?
      //   g_settings.playlist_directory : "default");
#ifdef HAVE_MENU
   config_set_path(conf, "rgui_browser_directory",
         *g_settings.menu_content_directory ?
         g_settings.menu_content_directory : "default");
   config_set_path(conf, "rgui_config_directory",
         *g_settings.menu_config_directory ?
         g_settings.menu_config_directory : "default");
   config_set_bool(conf, "rgui_show_start_screen",
         g_settings.menu_show_start_screen);
#endif

 //  config_set_path(conf, "game_history_path", g_settings.content_history_path);
  // config_set_int(conf, "game_history_size", g_settings.content_history_size);
   //config_set_path(conf, "joypad_autoconfig_dir",
       //  g_settings.input.autoconfig_dir);
   config_set_bool(conf, "input_autodetect_enable",
         g_settings.input.autodetect_enable);

#ifdef HAVE_OVERLAY
   config_set_path(conf, "overlay_directory",
         *g_extern.overlay_dir ? g_extern.overlay_dir : "default");
   config_set_path(conf, "input_overlay", g_settings.input.overlay);
   config_set_float(conf, "input_overlay_opacity",
         g_settings.input.overlay_opacity);
   config_set_float(conf, "input_overlay_scale",
         g_settings.input.overlay_scale);
#endif

  // config_set_path(conf, "video_font_path", g_settings.video.font_path);
  // config_set_float(conf, "video_message_pos_x", g_settings.video.msg_pos_x);
  // config_set_float(conf, "video_message_pos_y", g_settings.video.msg_pos_y);

   config_set_bool(conf, "gamma_correction",
         g_extern.console.screen.gamma_correction);
   config_set_bool(conf, "soft_filter_enable",
         g_extern.console.softfilter_enable);

   config_set_bool(conf, "agb_yscale",
         g_extern.console.agb_yscale);

  // config_set_bool(conf, "flicker_filter_enable",
    //     g_extern.console.flickerfilter_enable);

 //  config_set_int(conf, "flicker_filter_index",
   //      g_extern.console.screen.flicker_filter_index);
 //  config_set_int(conf, "soft_filter_index",
   //      g_extern.console.screen.soft_filter_index);
  // config_set_int(conf, "current_resolution_id",
    //     g_extern.console.screen.resolutions.current.id);
#ifdef HW_RVL
   if(CONF_GetAspectRatio() == CONF_ASPECT_16_9) {

   config_set_int(conf, "custom_viewport_wide_width",
         g_extern.console.screen.viewports.custom_vp.width);
   config_set_int(conf, "custom_viewport_wide_height",
         g_extern.console.screen.viewports.custom_vp.height);
   config_set_int(conf, "custom_viewport_wide_x",
         g_extern.console.screen.viewports.custom_vp.x);
   config_set_int(conf, "custom_viewport_wide_y",
         g_extern.console.screen.viewports.custom_vp.y);
   } else
#endif
 {
   config_set_int(conf, "custom_viewport_width",
         g_extern.console.screen.viewports.custom_vp.width);
   config_set_int(conf, "custom_viewport_height",
         g_extern.console.screen.viewports.custom_vp.height);
   config_set_int(conf, "custom_viewport_x",
         g_extern.console.screen.viewports.custom_vp.x);
   config_set_int(conf, "custom_viewport_y",
         g_extern.console.screen.viewports.custom_vp.y);
   }

  // config_set_float(conf, "video_font_size", g_settings.video.font_size);

   config_set_bool(conf, "block_sram_overwrite",
         g_settings.block_sram_overwrite);
   config_set_bool(conf, "savestate_auto_index",
         g_settings.savestate_auto_index);
   config_set_bool(conf, "regular_state_pause",
         g_settings.regular_state_pause);
   config_set_bool(conf, "stateload_pause",
         g_settings.stateload_pause);
   config_set_bool(conf, "savestate_auto_once",
         g_settings.savestate_auto_once);
   config_set_bool(conf, "savestate_auto_save",
         g_settings.savestate_auto_save);
   config_set_bool(conf, "savestate_auto_load",
         g_settings.savestate_auto_load);
  // config_set_bool(conf, "history_list_enable",
    //     g_settings.history_list_enable);

   config_set_float(conf, "fastforward_ratio", g_settings.fastforward_ratio);
   config_set_bool(conf, "fastforward_ratio_throttle_enable", g_settings.fastforward_ratio_throttle_enable);
   config_set_float(conf, "slowmotion_ratio", g_settings.slowmotion_ratio);

   config_set_bool(conf, "config_save_on_exit",
         g_settings.config_save_on_exit);
  // config_set_int(conf,  "sound_mode",   g_extern.console.sound.mode);
   config_set_int(conf,  "state_slot",      g_settings.state_slot);
   config_set_int(conf,  "hover_color",     g_settings.hover_color);
   config_set_int(conf,  "text_color",      g_settings.text_color);
   config_set_int(conf,  "theme_preset",    g_settings.theme_preset);
   config_set_int(conf,  "menu_bg_clr",     g_settings.menu_bg_clr);
   config_set_bool(conf, "menu_solid",      g_settings.menu_solid);
   config_set_int(conf,  "menu_msg_clr",    g_settings.menu_msg_clr);
   config_set_bool(conf, "menu_fullscreen", g_settings.menu_fullscreen);
   config_set_bool(conf, "hide_core",       g_settings.hide_core);
   config_set_bool(conf, "hide_states",     g_settings.hide_states);
   config_set_bool(conf, "hide_screenshot", g_settings.hide_screenshot);
   config_set_bool(conf, "hide_resume",     g_settings.hide_resume);
   config_set_bool(conf, "hide_reset",      g_settings.hide_reset);
   config_set_bool(conf, "hide_settings",   g_settings.hide_settings);
   config_set_bool(conf, "hide_exit",       g_settings.hide_exit);
   config_set_bool(conf, "hide_cursor",     g_settings.hide_cursor);
   config_set_bool(conf, "hide_curr_state", g_settings.hide_curr_state);
   config_set_bool(conf, "show_manuals",    g_settings.show_manuals);
   config_set_int(conf,  "title_posx",      g_settings.title_posx);
   config_set_int(conf,  "title_posy",      g_settings.title_posy);
   config_set_int(conf,  "item_posx",       g_settings.item_posx);
   config_set_int(conf,  "item_posy",       g_settings.item_posy);
   config_set_int(conf,  "clock_posx",      g_settings.clock_posx);

#ifdef HAVE_NETPLAY
   config_set_bool(conf, "netplay_spectator_mode_enable",
         g_extern.netplay_is_spectate);
   config_set_bool(conf, "netplay_mode", g_extern.netplay_is_client);
   config_set_string(conf, "netplay_ip_address", g_extern.netplay_server);
   config_set_int(conf, "netplay_ip_port", g_extern.netplay_port);
   config_set_int(conf, "netplay_delay_frames", g_extern.netplay_sync_frames);
#endif
  // config_set_string(conf, "netplay_nickname", g_settings.username);
  // config_set_int(conf, "user_language", g_settings.user_language);

  // config_set_bool(conf, "custom_bgm_enable",
      //   g_extern.console.sound.system_bgm_enable);

   config_set_string(conf, "input_driver", g_settings.input.driver);
   config_set_string(conf, "input_joypad_driver",
         g_settings.input.joypad_driver);
   //config_set_string(conf, "input_keyboard_layout",
     //    g_settings.input.keyboard_layout);
#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++)
#endif
   {
      char cfg[64];
      snprintf(cfg, sizeof(cfg), "input_device_p%u", i + 1);
      config_set_int(conf, cfg, g_settings.input.device[i]);
      snprintf(cfg, sizeof(cfg), "input_player%u_joypad_index", i + 1);
      config_set_int(conf, cfg, g_settings.input.joypad_map[i]);
      snprintf(cfg, sizeof(cfg), "input_libretro_device_p%u", i + 1);
      config_set_int(conf, cfg, g_settings.input.libretro_device[i]);
      snprintf(cfg, sizeof(cfg), "input_player%u_analog_dpad_mode", i + 1);
      config_set_int(conf, cfg, g_settings.input.analog_dpad_mode[i]);
   }

#ifdef HAVE_5PLAY
   for (i = 0; i < MAX_PLAYERS - 11; i++) // 5 players
#elif HAVE_6PLAY
   for (i = 0; i < MAX_PLAYERS - 10; i++) // 6 players
#elif HAVE_8PLAY
   for (i = 0; i < MAX_PLAYERS - 8; i++) // 8 players
#else
   for (i = 0; i < MAX_PLAYERS - 12; i++)
#endif
      save_keybinds_player(conf, i);

   config_set_bool(conf, "core_specific_config",
         g_settings.core_specific_config);
  // config_set_int(conf, "libretro_log_level", g_settings.libretro_log_level);
  // config_set_bool(conf, "log_verbosity", g_extern.verbosity);
  // config_set_bool(conf, "perfcnt_enable", g_extern.perfcnt_enable);

   ret = config_file_write(conf, path);
   config_file_free(conf);
   return ret;
}
