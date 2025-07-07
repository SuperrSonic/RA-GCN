#ifndef PLAYLOG_H_
#define PLAYLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>

#define ALIGN32(x) (((x) + 31) & ~31)

int Playlog_Exit(void);

const char _vcsettings_path[] __attribute__((aligned(32))) =
              "/shared2/menu/vc/settings.sav";

typedef struct _vcSettings
{
	u32 id_1;
	u8 unk_1;
	u8 unk_2;
	u8 ds;
	u8 unk_3; // seen it enabled
	u32 id_2; // copy of id_1
	u8 valid; // seems to require being 1 or the file is invalid
	u8 unk_4;
	u8 unk_5;
	u8 unk_6;
	u32 unused_1;
	u8 unused_2;
	u8 unused_3;
	u8 n3ds_port;
	u8 unused_4;
	u8 unused_rest[8];
} vcSettings;

#ifdef __cplusplus
}
#endif

#endif
