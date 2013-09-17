#ifndef __FS_H
#define __FS_H

#include <driver.h>

#define PATH_MAX       1024        /* include/linux/limits.h */

struct partition;
struct node_d;
struct stat;

struct dirent {
	char d_name[256];
};

typedef struct dir {
	struct device_d *dev;
	struct fs_driver_d *fsdrv;
	struct node_d *node;
	struct dirent d;
	void *priv; /* private data for the fs driver */
} DIR;

typedef struct filep {
	struct device_d *dev; /* The device this FILE belongs to              */
	ulong pos;            /* current position in stream                   */
	ulong size;           /* The size of this inode                       */
	ulong flags;          /* the O_* flags from open                      */

	void *inode;         /* private to the filesystem driver              */

	/* private fields. Mapping between FILE and filedescriptor number     */
	int no;
	char in_use;
} FILE;

#define FS_DRIVER_NO_DEV	1

struct fs_driver_d {
	char *name;
	int (*probe) (struct device_d *dev);
	int (*mkdir)(struct device_d *dev, const char *pathname);
	int (*rmdir)(struct device_d *dev, const char *pathname);

	/* create a file. The file is guaranteed to not exist */
	int (*create)(struct device_d *dev, const char *pathname, mode_t mode);
	int (*unlink)(struct device_d *dev, const char *pathname);

	/* Truncate a file to given size */
	int (*truncate)(struct device_d *dev, FILE *f, ulong size);

	int (*open)(struct device_d *dev, FILE *f, const char *pathname);
	int (*close)(struct device_d *dev, FILE *f);
	int (*read)(struct device_d *dev, FILE *f, void *buf, size_t size);
	int (*write)(struct device_d *dev, FILE *f, const void *buf, size_t size);
	int (*flush)(struct device_d *dev, FILE *f);
	off_t (*lseek)(struct device_d *dev, FILE *f, off_t pos);

	struct dir* (*opendir)(struct device_d *dev, const char *pathname);
	struct dirent* (*readdir)(struct device_d *dev, struct dir *dir);
	int (*closedir)(struct device_d *dev, DIR *dir);
	int (*stat)(struct device_d *dev, const char *file, struct stat *stat);

	int (*ioctl)(struct device_d *dev, FILE *f, int request, void *buf);
	int (*erase)(struct device_d *dev, FILE *f, size_t count,
			unsigned long offset);
	int (*protect)(struct device_d *dev, FILE *f, size_t count,
			unsigned long offset, int prot);

	int (*memmap)(struct device_d *dev, FILE *f, void **map, int flags);

	struct driver_d drv;

	unsigned long flags;

	struct list_head list;
};

struct mtab_entry {
	char path[PATH_MAX];
	struct mtab_entry *next;
	struct device_d *dev;
	struct device_d *parent_device;
};

struct fs_device_d {
	char *backingstore; /* the device we are associated with */
	struct device_d dev; /* our own device */

	struct fs_driver_d *driver;

	struct mtab_entry mtab;
};

/*
 * standard posix file functions
 */
int open(const char *pathname, int flags, ...);
int creat(const char *pathname, mode_t mode);
int unlink(const char *pathname);
int close(int fd);
int flush(int fd);
int stat(const char *filename, struct stat *s);
int read(int fd, void *buf, size_t count);
int ioctl(int fd, int request, void *buf);
ssize_t write(int fd, const void *buf, size_t count);

#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

off_t lseek(int fildes, off_t offset, int whence);
int mkdir (const char *pathname, mode_t mode);

/* Create a directory and its parents */
int make_directory(const char *pathname);
int rmdir (const char *pathname);

const char *getcwd(void);
int chdir(const char *pathname);

DIR *opendir(const char *pathname);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);

int mount (const char *device, const char *fsname, const char *path);
int umount(const char *pathname);

/* not-so-standard functions */
int erase(int fd, size_t count, unsigned long offset);
int protect(int fd, size_t count, unsigned long offset, int prot);
int protect_file(const char *file, int prot);
void *memmap(int fd, int flags);

#define PROT_READ	1
#define PROT_WRITE	2

#define LS_RECURSIVE	1
#define LS_SHOWARG	2
#define LS_COLUMN	4
int ls(const char *path, ulong flags);

char *mkmodestr(unsigned long mode, char *str);

/*
 * Information about mounted devices.
 * Note that we only support mounting on directories lying
 * directly in / and of course the root directory itself
 */
struct mtab_entry *get_mtab_entry_by_path(const char *path);
struct mtab_entry *mtab_next_entry(struct mtab_entry *entry);
const char *fsdev_get_mountpoint(struct fs_device_d *fsdev);

/*
 * Read a file into memory. Memory is allocated with malloc and must
 * be freed with free() afterwards. This function allocates one
 * byte more than actually needed and sets this to zero, so that
 * it can be used for text files.
 * If size is nonzero it s set to the file size.
 */
void *read_file(const char *filename, size_t *size);

/*
 * This function turns 'path' into an absolute path and removes all occurrences
 * of "..", "." and double slashes. The returned string must be freed wit free().
 */
char *normalise_path(const char *path);

/* Register a new filesystem driver */
int register_fs_driver(struct fs_driver_d *fsdrv);

#endif /* __FS_H */
