/****************************************************************************
 * Copyright (C) 2010 Joseph Jordan <joe.ftpii@psychlaw.com.au>
 * Copyright (C) 2011 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <errno.h>
#include <ogc/isfs.h>
#include <ogc/lwp_watchdog.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/iosupport.h>
#include <malloc.h>
#include <fcntl.h>
#include <time.h>

#include "isfs.h"

#define DEVICE_NAME "nand"

#define UNUSED  __attribute__((unused))
#define FLAG_DIR 1
#define DIR_SEPARATOR '/'
#define DEV_ID		(0x4957)
#define SECTOR_SIZE	(0x4000)
#define FLASH_SIZE	(512 * 1024 * 1024) // 512 MB

typedef struct DIR_ENTRY_STRUCT {
	char *name;
	char *abspath;
	u32 size;
	u8 flags;
	u32 childCount;
	struct DIR_ENTRY_STRUCT *children;
} DIR_ENTRY;

typedef struct {
	s32 fd;
	u32 size;
	int flags;
	int mode;
} FILE_STRUCT;

typedef struct _DIR_STATE_STRUCT {
	DIR_ENTRY *entry;
	u32 index;
} DIR_STATE_STRUCT;


static s32 READ_ONLY = 1;
static DIR_STATE_STRUCT *current = NULL;
static fstats filest __attribute__((aligned(32)));
static s32 dotab_device = -1;

static inline bool is_dir(DIR_ENTRY *entry) {
	return entry->flags & FLAG_DIR;
}

static inline void RemoveDoubleSlash(char *str)
{
	if(!str) return;
	int count = 0;
	const char *ptr = str;
	while(*ptr != 0)
	{
		if(*ptr == '/' && ptr[1] == '/') {
			ptr++;
			continue;
		}
		str[count] = *ptr;
		ptr++;
		count++;
	}

	while(count > 2 && str[count-1] == '/')
		count--;

	str[count] = 0;
}

static DIR_ENTRY *add_child_entry(DIR_ENTRY *dir, const char *name) {
	if(!dir->children)
		dir->children = malloc(sizeof(DIR_ENTRY));

	DIR_ENTRY *newChildren = realloc(dir->children, (dir->childCount + 1) * sizeof(DIR_ENTRY));
	if (!newChildren) return NULL;
	bzero(newChildren + dir->childCount, sizeof(DIR_ENTRY));
	dir->children = newChildren;
	DIR_ENTRY *child = &dir->children[dir->childCount++];
	child->name = strdup(name);
	if (!child->name) return NULL;
	child->abspath = malloc(strlen(dir->abspath) + strlen(name) + 2);
	if (!child->abspath) return NULL;
	sprintf(child->abspath, "%s/%s", dir->abspath, name);
	RemoveDoubleSlash(child->abspath);
	return child;
}

static void cleanup_recursive(DIR_ENTRY *entry) {
	if(!entry)
		return;

	u32 i;
	for (i = 0; i < entry->childCount; i++)
		cleanup_recursive(&entry->children[i]);
	if (entry->children)
		free(entry->children);
	if (entry->name)
		free(entry->name);
	if (entry->abspath)
		free(entry->abspath);
	bzero(entry, sizeof(DIR_ENTRY));
}

static bool read_directory(DIR_ENTRY *parent) {
	if(!parent || !parent->abspath)
		return false;

	u32 fileCount;
	if(parent->size != 0 && is_dir(parent))
	{
		fileCount = parent->size;
	}
	else
	{
		s32 ret = ISFS_ReadDir(parent->abspath, NULL, &fileCount);
		if (ret != ISFS_OK) {
			return false;
		}
	}
	parent->flags = FLAG_DIR;
	parent->size = fileCount;
	parent->childCount = 0;

	if(strcmp(parent->abspath, "/") != 0)
	{
		DIR_ENTRY *child = add_child_entry(parent, ".");
		if (!child) return false;
		child->flags = FLAG_DIR;
		child->size = 0;
		child = add_child_entry(parent, "..");
		if (!child) return false;
		child->flags = FLAG_DIR;
		child->size = 0;
	}

	if (fileCount > 0)
	{
		char *buffer = (char *) memalign(32, ISFS_MAXPATHLEN * fileCount);
		if(!buffer) return false;

		s32 ret = ISFS_ReadDir(parent->abspath, buffer, &fileCount);
		if (ret != ISFS_OK)
		{
			free(buffer);
			return false;
		}

		u32 fileNum;
		char *name = buffer;
		for (fileNum = 0; fileNum < fileCount; fileNum++)
		{
			DIR_ENTRY *child = add_child_entry(parent, name);
			if (!child)
			{
				free(buffer);
				return false;
			}
			name += strlen(name) + 1;

			u32 childFileCount;
			ret = ISFS_ReadDir(child->abspath, NULL, &childFileCount);
			if (ret == ISFS_OK)
			{
				child->flags = FLAG_DIR;
				child->size = childFileCount;
			}
			else
			{
				s32 fd = ISFS_Open(child->abspath, ISFS_OPEN_READ);
				if (fd >= 0) {
					if (ISFS_GetFileStats(fd, &filest) == ISFS_OK)
						child->size = filest.file_length;
					ISFS_Close(fd);
				}
			}
		}
		free(buffer);
	}
	return true;
}

static void setCurrent(DIR_ENTRY *entry)
{
	if(!entry)
		return;

	cleanup_recursive(current->entry);

	current->index = 0;
	current->entry->name = strdup(entry->name);
	current->entry->abspath = strdup(entry->abspath);
	current->entry->size = entry->size;
	current->entry->flags = entry->flags;

	u32 i;
	for(i = 0; i < entry->childCount; ++i)
	{
		DIR_ENTRY *child = add_child_entry(current->entry, entry->children[i].name);
		if(!child) break;
		child->size = entry->children[i].size;
		child->flags = entry->children[i].flags;
		child->childCount = 0;
		child->children = 0;
	}
	current->entry->childCount = i;
}

static void stat_entry(DIR_ENTRY *entry, struct stat *st)
{
	if(!entry || !st)
		return;

	time_t t = time(0);
	st->st_dev = DEV_ID;
	st->st_ino = 0;
	st->st_mode = (is_dir(entry) ? S_IFDIR : S_IFREG) | (S_IRUSR | S_IRGRP | S_IROTH);
	st->st_nlink = 1;
	st->st_uid = 1;
	st->st_gid = 2;
	st->st_rdev = st->st_dev;
	st->st_size = entry->size;
	st->st_atime = t;
	st->st_spare1 = 0;
	st->st_mtime = t;
	st->st_spare2 = 0;
	st->st_ctime = t;
	st->st_spare3 = 0;
	st->st_blksize = SECTOR_SIZE;
	st->st_blocks = (entry->size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	st->st_spare4[0] = 0;
	st->st_spare4[1] = 0;
}

extern u32 ownerID;

static int _ISFS_open_r(struct _reent *r, void *fileStruct, const char *path, int flags, int mode) {
	FILE_STRUCT *file = (FILE_STRUCT *)fileStruct;

	char *abspath = malloc(ISFS_MAXPATHLEN);
	if(!abspath) {
		r->_errno = ENOMEM;
		return -1;
	}

	char *ptr = strchr(path, ':');
	if (ptr != NULL)
		snprintf(abspath, ISFS_MAXPATHLEN, "%s", ptr + 1);
	else
		snprintf(abspath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, path);

	RemoveDoubleSlash(abspath);

	if (!READ_ONLY && (flags & O_CREAT)) {
		int iOwnerPerm  = 0;
		int iGroupPerm = 0;
		int iOtherPerm = 0;

		if (flags & S_IRUSR)
			iOwnerPerm |= ISFS_OPEN_READ;
		if (flags & S_IWUSR)
			iOwnerPerm |= ISFS_OPEN_WRITE;
		if (flags & S_IRGRP)
			iGroupPerm |= ISFS_OPEN_READ;
		if (flags & S_IWGRP)
			iGroupPerm |= ISFS_OPEN_WRITE;
		if (iGroupPerm & S_IROTH)
			iOtherPerm |= ISFS_OPEN_READ;
		if (flags & S_IWOTH)
			iOtherPerm |= ISFS_OPEN_WRITE;

		ISFS_CreateFile(abspath, 0, iOwnerPerm, iGroupPerm, iOtherPerm);
		
		// Change attributes for channel save
		ISFS_SetAttr(abspath, ownerID, 0x3850, 0, 3, 3, 0); // 8P
	}

	int iOpenMode = 0;

	if ((flags & 0x03) == O_RDONLY)
		iOpenMode |= ISFS_OPEN_READ;
	if (!READ_ONLY && ((flags & 0x03) == O_WRONLY))
		iOpenMode |= ISFS_OPEN_WRITE;
	if ((flags & 0x03) == O_RDWR)
		iOpenMode |= READ_ONLY ? ISFS_OPEN_READ : ISFS_OPEN_RW;

	file->fd = ISFS_Open(abspath, iOpenMode);

	free(abspath);

	if (file->fd < 0) {
		if (file->fd == ISFS_EINVAL)
			r->_errno = EACCES;
		else
			r->_errno = -file->fd;
		return -1;
	}

	file->flags = flags;
	file->mode = mode;

	if (ISFS_GetFileStats(file->fd, &filest) == ISFS_OK)
		file->size = filest.file_length;

	if (!READ_ONLY && (flags & O_APPEND))
		ISFS_Seek(file->fd, 0, SEEK_END);

	return (int)file;
}

static int _ISFS_close_r(struct _reent *r, int fd) {
	FILE_STRUCT *file = (FILE_STRUCT *)fd;
	if (file->fd < 0) {
		r->_errno = EBADF;
		return -1;
	}

	s32 ret = ISFS_Close(file->fd);
	if (ret < 0) {
		r->_errno = -ret;
		return -1;
	}

	return 0;
}

static int _ISFS_read_r(struct _reent *r, int fd, char *buf, size_t len) {
	FILE_STRUCT *file = (FILE_STRUCT *)fd;
	if (file->fd < 0) {
		r->_errno = EBADF;
		return -1;
	}

	char *buf32 = (char *) memalign(32, SECTOR_SIZE);
	if(!buf32) {
		r->_errno = ENOMEM;
		return -1;
	}

	s32 ret = 0;
	s32 read = 0;

	while(len > 0)
	{
		ret = ISFS_Read(file->fd, buf32, len > SECTOR_SIZE ? SECTOR_SIZE : len);
		if (ret < 0) {
			r->_errno = -ret;
			read = ret;
			break;
		}
		if(ret == 0)
			break;

		memcpy(buf + read, buf32, ret);
		read += ret;
		len -= ret;
	}

	free(buf32);

	return read;
}

static int _ISFS_write_r(struct _reent *r UNUSED, int fd, const char *buf, size_t len) {

	if(READ_ONLY) {
		r->_errno = EACCES;
		return -1;
	}

	FILE_STRUCT *file = (FILE_STRUCT *)fd;
	if (file->fd < 0) {
		r->_errno = EBADF;
		return -1;
	}

	char *buf32 = (char *) memalign(32, SECTOR_SIZE);
	if(!buf32) {
		r->_errno = ENOMEM;
		return -1;
	}

	s32 ret = 0;
	s32 wrote = 0;

	while(len > 0)
	{
		s32 block = len > SECTOR_SIZE ? SECTOR_SIZE : len;
		memcpy(buf32, buf + wrote, block);

		ret = ISFS_Write(file->fd, buf32, block);
		if (ret <= 0) {
			r->_errno = -ret;
			wrote = ret;
			break;
		}

		wrote += ret;
		len -= ret;
	}

	free(buf32);

	return wrote;
}

static off_t _ISFS_seek_r(struct _reent *r, int fd, off_t pos, int dir) {
	FILE_STRUCT *file = (FILE_STRUCT *)fd;
	if (file->fd < 0) {
		r->_errno = EBADF;
		return -1;
	}

	// seems to fail on SEEK_END -> add exception for the common case of getting file size
	if(dir == SEEK_END && pos == 0) {
		dir = SEEK_SET;
		pos = file->size;
	}

	s32 ret = ISFS_Seek(file->fd, pos, dir);
	if (ret < 0) {
		r->_errno = -ret;
		return -1;
	}
	return ret;
}

static int _ISFS_fstat_r(struct _reent *r, int fd, struct stat *st) {
	FILE_STRUCT *file = (FILE_STRUCT *)fd;
	if (file->fd < 0) {
		r->_errno = EBADF;
		return -1;
	}
	time_t t = time(0);
	st->st_dev = DEV_ID;
	st->st_ino = 0;
	st->st_mode = S_IFREG | (S_IRUSR | S_IRGRP | S_IROTH);
	st->st_nlink = 1;
	st->st_uid = 1;
	st->st_gid = 2;
	st->st_rdev = st->st_dev;
	st->st_size = file->size;
	st->st_atime = t;
	st->st_spare1 = 0;
	st->st_mtime = t;
	st->st_spare2 = 0;
	st->st_ctime = t;
	st->st_spare3 = 0;
	st->st_blksize = SECTOR_SIZE;
	st->st_blocks = (file->size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	st->st_spare4[0] = 0;
	st->st_spare4[1] = 0;
	return 0;
}

static int _ISFS_stat_r(struct _reent *r, const char *path, struct stat *st) {
	char *abspath = malloc(ISFS_MAXPATHLEN);
	if(!abspath) return -1;

	char *ptr = strchr(path, ':');
	if (ptr != NULL)
		snprintf(abspath, ISFS_MAXPATHLEN, "%s", ptr + 1);
	else
		snprintf(abspath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, path);

	RemoveDoubleSlash(abspath);

	DIR_ENTRY *entry = 0;

	if(strcmp(abspath, current->entry->abspath) == 0)
		entry = current->entry;

	u32 i;
	for(i = 0; i < current->entry->childCount; ++i)
		if(strcmp(abspath, current->entry->children[i].abspath) == 0)
			entry = &current->entry->children[i];

	bool isDir = false;
	u32 size = 0;

	if(entry)
	{
		isDir = entry->flags & FLAG_DIR;
		size = entry->size;
	}
	else
	{
		u32 fileCount;
		s32 ret = ISFS_ReadDir(abspath, NULL, &fileCount);
		if (ret == ISFS_OK)
		{
			isDir = true;
			size = fileCount;
		}
		else
		{
			s32 fd = ISFS_Open(abspath, ISFS_OPEN_READ);
			if (fd < 0) {
				free(abspath);
				r->_errno = ENOENT;
				return -1;
			}
			if (ISFS_GetFileStats(fd, &filest) == ISFS_OK)
				size = filest.file_length;
			ISFS_Close(fd);
		}
	}

	free(abspath);

	time_t t = time(0);
	st->st_dev = DEV_ID;
	st->st_ino = 0;
	st->st_mode = (isDir ? S_IFDIR : S_IFREG) | (S_IRUSR | S_IRGRP | S_IROTH);
	st->st_nlink = 1;
	st->st_uid = 1;
	st->st_gid = 2;
	st->st_rdev = st->st_dev;
	st->st_size = size;
	st->st_atime = t;
	st->st_spare1 = 0;
	st->st_mtime = t;
	st->st_spare2 = 0;
	st->st_ctime = t;
	st->st_spare3 = 0;
	st->st_blksize = SECTOR_SIZE;
	st->st_blocks = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	st->st_spare4[0] = 0;
	st->st_spare4[1] = 0;

	return 0;
}

static int _ISFS_chdir_r(struct _reent *r, const char *path) {

	char *abspath = malloc(ISFS_MAXPATHLEN);
	if(!abspath) return -1;

	strncpy(abspath, path, ISFS_MAXPATHLEN-1);
	RemoveDoubleSlash(abspath);

	u32 i = 0;
	DIR_ENTRY *dir = current->entry;
	DIR_ENTRY *entry = NULL;
	while (i < dir->childCount) {
		entry = &dir->children[i];
		if (entry->name && !strcasecmp(abspath, entry->name))
			break;
		if (entry->abspath && !strcasecmp(abspath, entry->abspath))
			break;
		i++;
	}

	free(abspath);

	if(i == dir->childCount){
		r->_errno = ENOENT;
		return -1;
	}

	setCurrent(entry);

	if(!read_directory(current->entry)) {
		r->_errno = ENOENT;
		return -1;
	}

	current->index = 0;
	return 0;
}

static DIR_ITER *_ISFS_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path) {

	DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);
	char *abspath = malloc(ISFS_MAXPATHLEN);
	if(!abspath) return NULL;

	char *ptr = strchr(path, ':');
	if (ptr != NULL)
		strncpy(abspath, ptr + 1, ISFS_MAXPATHLEN-1);
	else
		snprintf(abspath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, path);

	RemoveDoubleSlash(abspath);

	u32 fileCount;
	s32 ret = ISFS_ReadDir(abspath, NULL, &fileCount);
	if (ret != ISFS_OK) {
		free(abspath);
		r->_errno = ENOENT;
		return NULL;
	}
	state->entry = malloc(sizeof(DIR_ENTRY));
	if(!state->entry) {
		free(abspath);
		r->_errno = ENOMEM;
		return NULL;
	}

	const char *name = strrchr(abspath, '/');

	bzero(state->entry, sizeof(DIR_ENTRY));
	state->entry->name = name && (strlen(name) > 1) ? strdup(name+1) : strdup("/");
	state->entry->abspath = strdup(abspath);
	state->entry->size = fileCount;
	state->entry->flags = FLAG_DIR;

	free(abspath);

	if(!state->entry->name || !state->entry->abspath || !read_directory(state->entry)) {
		if(state->entry->name)
			free(state->entry->name);
		if(state->entry->abspath)
			free(state->entry->abspath);
		free(state->entry);
		r->_errno = ENOENT;
		return NULL;
	}

	state->index = 0;

	setCurrent(state->entry);

	return dirState;
}

static int _ISFS_dirreset_r(struct _reent *r UNUSED, DIR_ITER *dirState) {
	DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);
	state->index = 0;
	return 0;
}

static int _ISFS_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st) {
	DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);
	if (state->index >= state->entry->childCount) {
		r->_errno = ENOENT;
		return -1;
	}
	DIR_ENTRY *entry = &state->entry->children[state->index++];
	strncpy(filename, entry->name, ISFS_MAXPATHLEN - 1);
	stat_entry(entry, st);
	return 0;
}

static int _ISFS_dirclose_r(struct _reent *r UNUSED, DIR_ITER *dirState) {
	DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);

	if(state->entry)
	{
		cleanup_recursive(state->entry);
		free(state->entry);
		state->entry = NULL;
	}
	return 0;
}

static int _ISFS_statvfs_r(struct _reent *r UNUSED, const char *path UNUSED, struct statvfs *buf) {

	u32 usedblocks = 0;
	u32 inodes = 0;

	s32 ret = ISFS_GetUsage("/", &usedblocks, &inodes);
	if(ret < 0) {
		r->_errno = ENOENT;
		return -1;
	}

	buf->f_bsize = SECTOR_SIZE;
	buf->f_frsize = SECTOR_SIZE;
	buf->f_blocks = FLASH_SIZE / SECTOR_SIZE; // 512 MB flash
	buf->f_bfree = buf->f_bavail = MAX(buf->f_blocks - usedblocks, 0);
	buf->f_files = inodes;
	buf->f_ffree = buf->f_favail = MAX(262144 - inodes, 0);
	buf->f_fsid = DEV_ID;
	buf->f_flag = ST_RDONLY;
	buf->f_namemax = ISFS_MAXPATH;

	return 0;
}

static int _ISFS_rename_r (struct _reent *r UNUSED, const char *oldName, const char *newName)
{
	if(READ_ONLY) {
		r->_errno = EACCES;
		return -1;
	}

	char *pAbsOldPath = malloc(ISFS_MAXPATHLEN);
	if(!pAbsOldPath) {
		r->_errno = ENOMEM;
		return -1;
	}

	char *pAbsNewPath = malloc(ISFS_MAXPATHLEN);
	if(!pAbsNewPath) {
		r->_errno = ENOMEM;
		free(pAbsOldPath);
		return -1;
	}

	char *ptr = strchr(oldName, ':');
	if (ptr != NULL)
		snprintf(pAbsOldPath, ISFS_MAXPATHLEN, "%s", ptr + 1);
	else
		snprintf(pAbsOldPath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, oldName);

	ptr = strchr(newName, ':');
	if (ptr != NULL)
		snprintf(pAbsNewPath, ISFS_MAXPATHLEN, "%s", ptr + 1);
	else
		snprintf(pAbsNewPath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, newName);

	RemoveDoubleSlash(pAbsOldPath);
	RemoveDoubleSlash(pAbsNewPath);

	s32 ret = ISFS_Rename(pAbsOldPath, pAbsNewPath);
	if(ret < 0) {
		r->_errno = ENOENT;
	}

	free(pAbsOldPath);
	free(pAbsNewPath);

	return ret;
}

static int _ISFS_unlink_r(struct _reent *r, const char *name)
{
	if(READ_ONLY) {
		r->_errno = EACCES;
		return -1;
	}

	char *pAbsPath = malloc(ISFS_MAXPATHLEN);
	if(!pAbsPath) {
		r->_errno = ENOMEM;
		return -1;
	}

	char *ptr = strchr(name, ':');
	if (ptr != NULL)
		snprintf(pAbsPath, ISFS_MAXPATHLEN, "%s", ptr + 1);
	else
		snprintf(pAbsPath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, name);

	RemoveDoubleSlash(pAbsPath);

	s32 ret = ISFS_Delete(pAbsPath);
	if(ret < 0) {
		r->_errno = ENOENT;
	}

	free(pAbsPath);

	return ret;
}

static int _ISFS_mkdir_r (struct _reent *r, const char *path, int mode)
{
	if(READ_ONLY) {
		r->_errno = EACCES;
		return -1;
	}

	char *pAbsPath = malloc(ISFS_MAXPATHLEN);
	if(!pAbsPath) {
		r->_errno = ENOMEM;
		return -1;
	}

	char *ptr = strchr(path, ':');
	if (ptr != NULL)
		snprintf(pAbsPath, ISFS_MAXPATHLEN, "%s", ptr + 1);
	else
		snprintf(pAbsPath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, path);

	RemoveDoubleSlash(pAbsPath);

	int iOwnerPerm = (mode / 100) % 10;
	int iGroupPerm = (mode / 10) % 10;
	int iOtherPerm = (mode % 10);

	s32 ret = ISFS_CreateDir(pAbsPath, 0, iOwnerPerm, iGroupPerm, iOtherPerm);
	if(ret < 0) {
		r->_errno = EBADF;
	}

	free(pAbsPath);

	return ret;
}

static int _ISFS_chmod_r(struct _reent *r, const char *path, mode_t mode)
{
	if(READ_ONLY) {
		r->_errno = EACCES;
		return -1;
	}

	char *pAbsPath = malloc(ISFS_MAXPATHLEN);
	if(!pAbsPath) {
		r->_errno = ENOMEM;
		return -1;
	}

	char *ptr = strchr(path, ':');
	if (ptr != NULL)
		snprintf(pAbsPath, ISFS_MAXPATHLEN, "%s", ptr + 1);
	else
		snprintf(pAbsPath, ISFS_MAXPATHLEN, "%s/%s", current->entry->abspath, path);

	RemoveDoubleSlash(pAbsPath);

	u32 uiOwnerID = 0;
	u16 uiGroupID = 0;
	u8 ucAttributes = 0, ucOwnerPerm, ucGroupPerm, ucOtherPerm;

	s32 ret = ISFS_GetAttr(pAbsPath, &uiOwnerID, &uiGroupID, &ucAttributes, &ucOwnerPerm, &ucGroupPerm, &ucOtherPerm);
	if(ret < 0) {
		r->_errno = ENOENT;
	}
	else {
		int iOwnerPerm = (mode / 100) % 10;
		int iGroupPerm = (mode / 10) % 10;
		int iOtherPerm = (mode % 10);

		ret = ISFS_SetAttr(pAbsPath, uiOwnerID, uiGroupID, ucAttributes, iOwnerPerm, iGroupPerm, iOtherPerm);
		if(ret < 0) {
			r->_errno = ENOENT;
		}
	}

	free(pAbsPath);

	return ret;
}

static const devoptab_t dotab_isfs = {
	DEVICE_NAME,
	sizeof(FILE_STRUCT),
	_ISFS_open_r,
	_ISFS_close_r,
	_ISFS_write_r,
	_ISFS_read_r,
	_ISFS_seek_r,
	_ISFS_fstat_r,
	_ISFS_stat_r,
	NULL, // _ISFS_link_r
	_ISFS_unlink_r,
	_ISFS_chdir_r,
	_ISFS_rename_r,
	_ISFS_mkdir_r,
	sizeof(DIR_STATE_STRUCT),
	_ISFS_diropen_r,
	_ISFS_dirreset_r,
	_ISFS_dirnext_r,
	_ISFS_dirclose_r,
	_ISFS_statvfs_r,
	NULL, // _ISFS_ftruncate_r,
	NULL, // _ISFS_fsync_r,
	NULL, /* Device data */
	_ISFS_chmod_r,
	NULL  // _ISFS_fchmod_r,
};

static bool read_isfs()
{
	current = malloc(sizeof(DIR_STATE_STRUCT));
	if(!current)
		return false;
	current->entry = malloc(sizeof(DIR_ENTRY));
	if(!current->entry)
	{
		free(current->entry);
		return false;
	}

	current->index = 0;
	bzero(current->entry, sizeof(DIR_ENTRY));
	current->entry->name = strdup("/");
	current->entry->abspath = strdup("/");
	return read_directory(current->entry);
}

bool ISFS_Mount(s32 read_only) {
	ISFS_Unmount();
	READ_ONLY = read_only;
	bool success = read_isfs() && (dotab_device = AddDevice(&dotab_isfs)) >= 0;
	if (!success) ISFS_Unmount();
	return success;
}

bool ISFS_IsMounted() {
	return (current != NULL);
}

bool ISFS_Unmount() {
	if(current && current->entry)
	{
		cleanup_recursive(current->entry);
		free(current->entry);
	}
	if(current)
	{
		free(current);
		current = NULL;
	}
	if (dotab_device >= 0) {
		dotab_device = -1;
		return !RemoveDevice(DEVICE_NAME ":");
	}
	return true;
}
