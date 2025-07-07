#include <gccore.h>

#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "title.h"

u32 ng_id = 0;
u32 ms_id = 0;

static u64 title_id = 0;
static char title_path[ISFS_MAXPATH] __attribute__((aligned(0x20)));

static u8 buf[0x1000] __attribute__((aligned(0x20)));

static void title_get_ngid(void) {
	s32 res;

	res = ES_GetDeviceID(&ng_id);
	if (res < 0) {
		printf("ES_GetDeviceID failed: %d\n", res);
	}
}

static void title_get_msid(void) {
	s32 ret;

	// check for dpki
	ret = ES_GetDeviceCert(buf);
	if (ret < 0) {
		printf("ES_GetDeviceCert failed: %d\n", ret);
		return;
	}

	ms_id = buf[0x99] - '0';

	if (ms_id == 3) {
		printf("On dpki\n");
	} else if (ms_id == 2) {
		printf("On retail\n");
	} else {
		printf("Unknown ms-id %d?\n", ms_id);
	}
}

static void title_get_title_path(void) {
	s32 res;

	res = ES_GetTitleID(&title_id);
	if (res < 0) {
		printf("ES_GetTitleID failed: %d\n", res);
		return;
	}

	res = ES_GetDataDir(title_id, title_path);
	if (res < 0) {
		printf("ES_GetDataDir failed: %d\n", res);
		return;
	}

	printf("data path is '%s'\n", title_path);
}

const char *title_get_path(void) {
	return title_path;
}

static bool title_is_installed(u64 title_id) {
	s32 ret;
	u32 x;

	ret = ES_GetTitleContentsCount(title_id, &x);

	if (ret < 0)
		return false; // title was never installed

	if (x <= 0)
		return false; // title was installed but deleted via Channel Management

	return true;
}

#define TITLEID_200           0x0000000100000200ll

bool is_vwii(void) {
	return title_is_installed(TITLEID_200);
}

static fstats __st ATTRIBUTE_ALIGN(32);
u32 ownerID = 0;

void get_uid(void) {
	s32 fd = -1;
	fd = ISFS_Open("/sys/uid.sys", ISFS_OPEN_READ);
	if(fd < 0) {
		printf("Failed to get uid!");
	} else {
		u64 curTID = 0;
		u8 *uid_bin = NULL;
		
		ISFS_GetFileStats(fd, &__st);
		DCInvalidateRange(&__st, sizeof(__st));
		
		uid_bin = memalign(32, __st.file_length);
		ISFS_Read(fd, uid_bin, __st.file_length);
		ISFS_Close(fd);
		int id = 0;
		for(id = 0; id < __st.file_length; id += 0xC) {
			memcpy(&curTID, &uid_bin[id], 8);
			if(curTID == title_id) {
				memcpy(&ownerID, &uid_bin[id+8], 4);
				printf("Show UID: 0x%X\n", ownerID);
				break;
			}
		}
		free(uid_bin);
	}
}

void title_init(void) {
	memset(title_path, 0, sizeof(title_path));
	title_get_msid();
	title_get_ngid();
	title_get_title_path();
	
	// This allows isfs.c to set the correct permissions
	// so saves can be copied by Data Management.
	// This doesn't solve the issue of needing NAND access patched
	// in order to read the ROM from the content folder.
	get_uid();
}
