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

#include <boolean.h>
#include "../../driver.h"
#include "../../general.h"
#include "../../libretro_private.h"
#include "../../gfx/gx/sdk_defines.h"

#include <file/file_path.h>

#if defined(HW_RVL) && !defined(IS_SALAMANDER)
#include "../../wii/mem2_manager.h"
#include <ogc/mutex.h>
#include <ogc/cond.h>

#ifdef USE_NAND
#include <ogc/machine/processor.h> // causes redefinition warns
#include "../../wii/isfs.h"
#include "../../wii/title.h"

//char dataPath[128] = {0};
#endif

#endif

#ifndef CONF_NAME
#define CONF_NAME "main.cfg"
#endif

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#ifndef _DIRENT_HAVE_D_TYPE
#include <sys/stat.h>
#endif
#include <unistd.h>
#include <dirent.h>

#ifdef HW_RVL
#include <ogc/ios.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>
extern void system_exec_wii(const char *path, bool should_load_game);
#endif
#include <sdcard/gcsd.h>
#include <fat.h>

static bool exit_spawn = false;
static bool exitspawn_start_game = false;

#ifndef IS_SALAMANDER

enum
{
   GX_DEVICE_SD = 0,
   GX_DEVICE_USB,
#ifdef USE_NAND
   GX_DEVICE_NAND,
#endif
#ifdef HW_DOL
   GX_DEVICE_GC,
   GX_DEVICE_SP2,
#endif
   GX_DEVICE_END
};

#if defined(HAVE_LOGGER) || defined(HAVE_FILE_LOGGER)
static devoptab_t dotab_stdout = {
   "stdout",   // device name
   0,          // size of file structure
   NULL,       // device open
   NULL,       // device close
   NULL,       // device write
   NULL,       // device read
   NULL,       // device seek
   NULL,       // device fstat
   NULL,       // device stat
   NULL,       // device link
   NULL,       // device unlink
   NULL,       // device chdir
   NULL,       // device rename
   NULL,       // device mkdir
   0,          // dirStateSize
   NULL,       // device diropen_r
   NULL,       // device dirreset_r
   NULL,       // device dirnext_r
   NULL,       // device dirclose_r
   NULL,       // device statvfs_r
   NULL,       // device ftrunctate_r
   NULL,       // device fsync_r
   NULL,       // deviceData;
};
#endif


static struct {
   bool mounted;
   const DISC_INTERFACE *interface;
   const char *name;
} gx_devices[GX_DEVICE_END];
#ifdef HW_RVL
static mutex_t gx_device_mutex;
#ifndef USE_TITLE
static mutex_t gx_device_cond_mutex;
static cond_t gx_device_cond;

static void *gx_devthread(void *a)
{
   struct timespec timeout = {0};

   timeout.tv_sec = 1;
   timeout.tv_nsec = 0;

   while (1)
   {
      LWP_MutexLock(gx_device_mutex);
      unsigned i;
#ifdef USE_NAND
      for (i = 0; i < (GX_DEVICE_END-1); i++)
#else
      for (i = 0; i < GX_DEVICE_END; i++)
#endif
      {
         if (gx_devices[i].mounted && 
               !gx_devices[i].interface->isInserted())
         {
            gx_devices[i].mounted = false;
            char n[8];
            snprintf(n, sizeof(n), "%s:", gx_devices[i].name);
            fatUnmount(n);
         }
      }
      LWP_MutexUnlock(gx_device_mutex);
      LWP_MutexLock(gx_device_cond_mutex);
      LWP_CondTimedWait(gx_device_cond, gx_device_cond_mutex, &timeout);
      LWP_MutexUnlock(gx_device_cond_mutex);
   }

   return NULL;
}
#endif
#endif

#if 0
static int gx_get_device_from_path(const char *path)
{
#ifdef HW_RVL
   if (strstr(path, "sd:") == path)
      return GX_DEVICE_SD;
   if (strstr(path, "usb:") == path)
      return GX_DEVICE_USB;
#ifdef USE_NAND
   if (strstr(path, "nand:") == path)
      return GX_DEVICE_NAND;
#endif
#endif
#ifdef HW_DOL
   if (strstr(path, "gcsd:") == path)
      return GX_DEVICE_GC;
#ifdef USE_SP2
	if (strstr(path, "sp2:") == path)
      return GX_DEVICE_SP2;
#endif
#endif
   return -1;
}
#endif

#ifdef HAVE_LOGGER
int gx_logger_net(struct _reent *r, int fd, const char *ptr, size_t len)
{
   static char temp[4000];
   size_t l = len >= 4000 ? 3999 : len;
   memcpy(temp, ptr, l);
   temp[l] = 0;
   logger_send("%s", temp);
   return len;
}
#elif defined(HAVE_FILE_LOGGER)
int gx_logger_file(struct _reent *r, int fd, const char *ptr, size_t len)
{
   fwrite(ptr, 1, len, g_extern.log_file);
   return len;
}
#endif

#endif

#ifdef IS_SALAMANDER
extern char gx_rom_path[PATH_MAX];
#endif

static void frontend_gx_get_environment_settings(int *argc, char *argv[],
      void *args, void *params_data)
{
#ifndef IS_SALAMANDER
#if defined(HAVE_LOGGER)
   logger_init();
#elif defined(HAVE_FILE_LOGGER)
   g_extern.log_file = fopen("/log.txt", "w");
#endif
#endif

   /* This situation can happen on some loaders so we really need some
      fake args or else retroarch will just crash on parsing NULL pointers */
#ifdef USE_TITLE
   const char *titlepath;
   titlepath = title_get_path(); //should always be 29 characters
   static char dataPath[PATH_MAX] = {0};
   static char contentPath[PATH_MAX] = {0};
   static char ROMpath[PATH_MAX];
   sprintf(dataPath, "nand:%s/", titlepath);
   sprintf(contentPath, "nand:%s", titlepath);
   contentPath[0x1E] = 0; //remove 'data'
   *ROMpath = '\0';
//   fill_pathname_join(dataPath, "nand:", titlepath, sizeof(dataPath));
   fill_pathname_join(ROMpath, contentPath, "content/00000002.app", sizeof(ROMpath));

   if(1)
   {
      struct rarch_main_wrap *args = (struct rarch_main_wrap*)params_data;
      if (args)
      {
       //  static char dataPath[PATH_MAX];
       //  *dataPath = '\0';
       //  fill_pathname_join(dataPath, "nand:/title/00010001/454e4545/", "data/", sizeof(dataPath));

         args->touched        = true;
         args->no_content     = false;
         args->verbose        = false;
         args->libretro_path  = NULL;

         sprintf(g_settings.core_options_path, dataPath);
		// printf("SHOW_DATAP: %s", dataPath);
		 
      //   args->config_path    = "nand:/title/00010001/454e4545/data/";
       //  args->sram_path      = "nand:/title/00010001/454e4545/data/";
       //  args->state_path     = "nand:/title/00010001/454e4545/data/";
		 
         args->config_path    = dataPath;
         args->sram_path      = dataPath;
         args->state_path     = dataPath;
         args->content_path   = ROMpath;
      //   args->content_path   = "nand:/title/00010001/454e4545/content/00000002.app";
      //   args->content_path   = NULL;

        // printf("SHOWNAN: %s,,", titlePath);
		// sleep(4);
      }
   }
#endif

#ifndef HW_DOL
   //chdir("carda:/private/other");
   if(1) {
   struct rarch_main_wrap *args = (struct rarch_main_wrap*)params_data;
      //if (args)
       //  static char dataPath[PATH_MAX];
       //  *dataPath = '\0';

         args->touched        = true;
         args->no_content     = false;
         args->verbose        = false;
         args->libretro_path  = NULL;

       //  sprintf(g_settings.core_options_path, dataPath);
  // args->config_path    = "gcsd:/apps/RA/";
  // args->sram_path      = "gcsd:/apps/Chao Transfer Tool/";
   args->state_path     = NULL;
   args->content_path   = "gcsd:/apps/GAME/Arcade/Cannonball/content.game";
   }
#endif

//#ifndef USE_TITLE
#ifdef HW_DOL
   getcwd(g_defaults.core_dir, MAXPATHLEN);
   char *last_slash = strrchr(g_defaults.core_dir, '/');
   if (last_slash)
      *last_slash = 0;
   char *device_end = strchr(g_defaults.core_dir, '/');
   // Make sure the private directory exists
   snprintf(g_defaults.port_dir_first, sizeof(g_defaults.port_dir_first),
            "%.*s/private", device_end - g_defaults.core_dir,
            g_defaults.core_dir);
   path_mkdir(g_defaults.port_dir_first);
   if (device_end)
      snprintf(g_defaults.port_dir, sizeof(g_defaults.port_dir),
            "%.*s/private/other", device_end - g_defaults.core_dir,
            g_defaults.core_dir);
   else
      fill_pathname_join(g_defaults.port_dir, g_defaults.port_dir,
            "private/other", sizeof(g_defaults.port_dir));
   fill_pathname_join(g_defaults.overlay_dir, g_defaults.port_dir,
         "overlays", sizeof(g_defaults.overlay_dir));
   fill_pathname_join(g_defaults.config_path, g_defaults.port_dir,
         //"main.cfg", sizeof(g_defaults.config_path));
         CONF_NAME, sizeof(g_defaults.config_path));
   fill_pathname_join(g_defaults.system_dir, g_defaults.port_dir,
         "system", sizeof(g_defaults.system_dir));
   fill_pathname_join(g_defaults.sram_dir, g_defaults.port_dir,
         "savefiles", sizeof(g_defaults.sram_dir));
   fill_pathname_join(g_defaults.savestate_dir, g_defaults.port_dir,
         "savestates", sizeof(g_defaults.savestate_dir));
   //fill_pathname_join(g_defaults.playlist_dir, g_defaults.port_dir,
       //  "playlists", sizeof(g_defaults.playlist_dir));
   fill_pathname_join(g_defaults.audio_filter_dir, g_defaults.port_dir,
         "audiofilters", sizeof(g_defaults.audio_filter_dir));
   fill_pathname_join(g_defaults.video_filter_dir, g_defaults.port_dir,
         "videofilters", sizeof(g_defaults.video_filter_dir));
   fill_pathname_join(g_defaults.screenshot_dir, g_defaults.port_dir,
         "screenshots", sizeof(g_defaults.screenshot_dir));
   fill_pathname_join(g_defaults.extract_dir, g_defaults.port_dir,
         "system/temp", sizeof(g_defaults.extract_dir));
#ifdef NEOGEO_FOLDER
   fill_pathname_join(g_defaults.neogeo_dir, g_defaults.port_dir,
         "system/neogeo", sizeof(g_defaults.neogeo_dir));
#endif

#endif

#ifndef IS_SALAMANDER
   /* Determine if we start in single game mode */
#ifdef USE_TITLE
   g_settings.single_mode = true;
  // g_settings.single_mode = false;
#else
   g_settings.single_mode = false;
#endif
#endif

#ifdef IS_SALAMANDER
   if (*argc > 2 && argv[1] != NULL && argv[2] != NULL)
      fill_pathname_join(gx_rom_path, argv[1], argv[2], sizeof(gx_rom_path));
   else
      gx_rom_path[0] = '\0';
#else
#ifdef HW_RVL
   /* One argument = cfg path */
   if(*argc > 1 && argv[1] != NULL && argv[2] == NULL)
   {
      struct rarch_main_wrap *args = (struct rarch_main_wrap*)params_data;
      if (args)
      {
         args->touched        = true;
         args->no_content     = false;
         args->verbose        = false;
         args->config_path    = NULL;
         args->sram_path      = NULL;
         args->state_path     = NULL;
         args->content_path   = NULL;
         args->libretro_path  = NULL;
      }

      if (argv[1] != NULL) {
         /* Name of .cfg when loading via args */
         fill_pathname_config(g_defaults.config_path, argv[1],
           sizeof(g_defaults.config_path));
      }
   }

   /* Needed on Wii; some loaders require two args,
    * filename and path are separate in the argument list */
#ifdef USE_TITLE
   if (0)
#else
   if (*argc > 2 && argv[1] != NULL && argv[2] != NULL)
#endif
   {
      static char path[PATH_MAX];
      *path = '\0';
      struct rarch_main_wrap *args = (struct rarch_main_wrap*)params_data;
	  g_settings.single_mode = true;

#ifdef USE_TITLE
      if (0)
#else
      if (args)
#endif
      {
         fill_pathname_join(path, argv[1], argv[2], sizeof(path));

         args->touched        = true;
         args->no_content     = false;
         args->verbose        = false;
         args->config_path    = NULL;
         args->sram_path      = NULL;
         args->state_path     = NULL;
         args->content_path   = path;
         args->libretro_path  = NULL;
      }

	  if (argv[3] != NULL) {
         /* Name of .cfg when loading via args */
         fill_pathname_config(g_defaults.config_path, argv[3],
           sizeof(g_defaults.config_path));
      }
   }

#endif
#endif

#ifdef HW_DOL
 // g_settings.single_mode = true;
#endif
}

#ifdef USE_NAND
#if 1
#define HAVE_AHBPROT ((*(vu32*)0xcd800064 == 0xFFFFFFFF) ? 1 : 0)
#define MEM2_PROT 0x0D8B420A

static void disable_memory_protection() {
    write32(MEM2_PROT, read32(MEM2_PROT) & 0x0000FFFF);
	
	// Do this manually because there's a conflict with processor.h
	//u32* MEM_PRT = (u32*)0xD8B420A;
	//*MEM_PRT &= 0xFFFF;
}

static u32 apply_patch(char *name, const u8 *old, u32 old_size, const u8 *patch, u32 patch_size, u32 patch_offset) {
    u8 *ptr = (u8 *)0x93400000;
    u32 found = 0;
    u8 *location = NULL;
    while ((u32)ptr < (0x94000000 - patch_size)) {
        if (!memcmp(ptr, old, old_size)) {
            found++;
            location = ptr + patch_offset;
            u8 *start = location;
            u32 i;
            for (i = 0; i < patch_size; i++) {
                *location++ = patch[i];
            }
            DCFlushRange((u8 *)(((u32)start) >> 5 << 5), (patch_size >> 5 << 5) + 64);
        }
        ptr++;
    }
    return found;
}

const u8 isfs_permissions_old[] = { 0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66 };
const u8 isfs_permissions_patch[] = { 0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66 };

//const u8 es_identify_old[] = { 0x28, 0x03, 0xD1, 0x23 };
//const u8 es_identify_patch[] = { 0x00, 0x00 };

u32 IOSPATCH_Apply() {
    u32 count = 0;
    if (HAVE_AHBPROT) {
        disable_memory_protection();
      //  count += apply_patch("di_readlimit", di_readlimit_old, sizeof(di_readlimit_old), di_readlimit_patch, sizeof(di_readlimit_patch), 12);
        count += apply_patch("isfs_permissions", isfs_permissions_old, sizeof(isfs_permissions_old), isfs_permissions_patch, sizeof(isfs_permissions_patch), 0);
       // count += apply_patch("es_setuid", setuid_old, sizeof(setuid_old), setuid_patch, sizeof(setuid_patch), 0);
     //   count += apply_patch("es_identify", es_identify_old, sizeof(es_identify_old), es_identify_patch, sizeof(es_identify_patch), 2);
       // count += apply_patch("hash_check", hash_old, sizeof(hash_old), hash_patch, sizeof(hash_patch), 1);
       // count += apply_patch("new_hash_check", new_hash_old, sizeof(new_hash_old), hash_patch, sizeof(hash_patch), 1);
    }
    return count;
}
#endif
#endif

extern void __exception_setreload(int t);

static void frontend_gx_init(void *data)
{
   (void)data;
#ifdef HW_RVL
   //IOS_ReloadIOS(249);
#ifndef USE_TITLE
   IOS_ReloadIOS(IOS_GetVersion()); // breaks ahb access
#endif
   L2Enhance();
#ifndef IS_SALAMANDER
   gx_init_mem2();
#endif
#endif

#if defined(DEBUG) && defined(IS_SALAMANDER)
   VIDEO_Init();
   GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
   void *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
   console_init(xfb, 20, 20, rmode->fbWidth,
         rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
   VIDEO_Configure(rmode);
   VIDEO_SetNextFramebuffer(xfb);
   VIDEO_SetBlack(FALSE);
   VIDEO_Flush();
   VIDEO_WaitVSync();
   VIDEO_WaitVSync();
#endif

#ifndef DEBUG
   __exception_setreload(8);
#endif

#ifdef USE_NAND
   ISFS_Initialize();

   IOSPATCH_Apply(); // enable for full nand writing/reading

   title_init();

   if (is_vwii()) {
     // poke DMCU to turn off pillarboxing
     write32(0xd8006a0, 0x30000004);
     mask32(0xd8006a8, 0, 2);
   }
#endif

#ifndef USE_TITLE
   //fatInitDefault();
#endif

#ifdef HAVE_LOGGER
   devoptab_list[STD_OUT] = &dotab_stdout;
   devoptab_list[STD_ERR] = &dotab_stdout;
   dotab_stdout.write_r = gx_logger_net;
#elif defined(HAVE_FILE_LOGGER) && !defined(IS_SALAMANDER)
   devoptab_list[STD_OUT] = &dotab_stdout;
   devoptab_list[STD_ERR] = &dotab_stdout;
   dotab_stdout.write_r = gx_logger_file;
#endif

#if defined(HW_RVL) && !defined(IS_SALAMANDER)
#ifndef USE_TITLE
   OSThread gx_device_thread;
   gx_devices[GX_DEVICE_SD].interface = &__io_wiisd;
   gx_devices[GX_DEVICE_SD].name = "sd";
   gx_devices[GX_DEVICE_SD].mounted = fatMountSimple(
         gx_devices[GX_DEVICE_SD].name,
         gx_devices[GX_DEVICE_SD].interface);
   gx_devices[GX_DEVICE_USB].interface = &__io_usbstorage;
   gx_devices[GX_DEVICE_USB].name = "usb";
   gx_devices[GX_DEVICE_USB].mounted = fatMountSimple(
            gx_devices[GX_DEVICE_USB].name,
            gx_devices[GX_DEVICE_USB].interface);
#endif
#ifdef USE_NAND
   gx_devices[GX_DEVICE_NAND].interface = NULL;
   gx_devices[GX_DEVICE_NAND].name = "nand";
   gx_devices[GX_DEVICE_NAND].mounted = ISFS_Mount(0); // 1=readonly
#endif

#ifndef USE_TITLE
   OSInitMutex(&gx_device_cond_mutex);
   OSInitCond(&gx_device_cond);
   OSInitMutex(&gx_device_mutex);
   OSCreateThread(&gx_device_thread, gx_devthread, 0, NULL, NULL, 0, 66, 0);
#endif
#endif

#ifdef HW_DOL
   gx_devices[GX_DEVICE_GC].interface = &__io_gcode;
   gx_devices[GX_DEVICE_GC].name = "gcsd";
   gx_devices[GX_DEVICE_GC].mounted = fatMountSimple(
            gx_devices[GX_DEVICE_GC].name,
            gx_devices[GX_DEVICE_GC].interface);
#ifdef USE_SP2
   gx_devices[GX_DEVICE_SP2].interface = &__io_gcsd2;
   gx_devices[GX_DEVICE_SP2].name = "sp2";
   gx_devices[GX_DEVICE_SP2].mounted = fatMountSimple(
            gx_devices[GX_DEVICE_SP2].name,
            gx_devices[GX_DEVICE_SP2].interface);
#endif
   AR_Init(NULL, 0);
   ARQ_Init();
#endif
}

static void frontend_gx_exec(const char *path, bool should_load_game);

static void frontend_gx_exitspawn(char *core_path, size_t sizeof_core_path)
{
   bool should_load_game = false;
#if defined(IS_SALAMANDER)
   if (gx_rom_path[0] != '\0')
      should_load_game = true;
#elif defined(HW_RVL)
   should_load_game = exitspawn_start_game;

   if (!exit_spawn)
      return;

   frontend_gx_exec(core_path, should_load_game);

   /* FIXME/TODO - hack
    * direct loading failed (out of memory), try to jump to Salamander,
    * then load the correct core */
   fill_pathname_join(core_path, g_defaults.core_dir,
         "boot.dol", sizeof_core_path);
#endif
   frontend_gx_exec(core_path, should_load_game);
}

static void frontend_gx_process_args(int *argc, char *argv[])
{
#ifndef IS_SALAMANDER
   /* A big hack: sometimes Salamander doesn't save the new core 
    * it loads on first boot, so we make sure
    * g_settings.libretro is set here. */
   if (!g_settings.libretro[0] && *argc >= 1 && strrchr(argv[0], '/'))
   {
      char path[PATH_MAX];
      strlcpy(path, strrchr(argv[0], '/') + 1, sizeof(path));
      rarch_environment_cb(RETRO_ENVIRONMENT_SET_LIBRETRO_PATH, path);
   }
#endif
}

static void frontend_gx_exec(const char *path, bool should_load_game)
{
#ifdef HW_RVL
   system_exec_wii(path, should_load_game);
#endif
}

static void frontend_gx_set_fork(bool exitspawn, bool start_game)
{
   exit_spawn = exitspawn;
   exitspawn_start_game = start_game;
}

static int frontend_gx_get_rating(void)
{
#ifdef HW_RVL
   return 8;
#else
   return 6;
#endif
}

const frontend_ctx_driver_t frontend_ctx_gx = {
   frontend_gx_get_environment_settings, /* get_environment_settings */
   frontend_gx_init,                /* init */
   NULL,                            /* deinit */
   frontend_gx_exitspawn,           /* exitspawn */
   frontend_gx_process_args,        /* process_args */
   NULL,                            /* process_events */
   frontend_gx_exec,                /* exec */
   frontend_gx_set_fork,            /* set_fork */
   NULL,                            /* shutdown */
   NULL,                            /* get_name */
   frontend_gx_get_rating,          /* get_rating */
   NULL,                            /* load_content */
   "gx",
};
