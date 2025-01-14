/*
 * (C) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef DRIVER_H
#define DRIVER_H

#include <linux/list.h>

#define MAX_DRIVER_NAME		32
#define FORMAT_DRIVER_MANE_ID	"%s%d"

#include <param.h>

/**
 * @file
 * @brief Main description of the device/driver model
 */

/** @page driver_model Main description of the device/driver model
 *
 * We follow a rather simplistic driver model here. There is a
 * @code struct device_d @endcode
 * which describes a particular device present in the system.
 *
 * On the other side a
 * @code struct driver_d @endcode
 * represents a driver present in the system.
 *
 * Both structs find together via the members 'type' (int) and 'name' (char *).
 * If both members match, the driver's probe function is called with the
 * struct device_d as argument.
 *
 * People familiar with the Linux platform bus will recognize this behaviour
 * and in fact many things were stolen from there. Some selected members of the
 * structs will be described in this document.
 */

/*@{*/	/* do not delete, doxygen relevant */

struct filep;
struct bus_type;

/** @brief Describes a particular device present in the system */
struct device_d {
	/*! This member (and 'type' described below) is used to match with a
	 * driver. This is a descriptive name and could be MPC5XXX_ether or
	 * imx_serial. */
	char name[MAX_DRIVER_NAME];
	/*! The id is used to uniquely identify a device in the system. The id
	 * will show up under /dev/ as the device's name. Usually this is
	 * something like eth0 or nor0. */
	int id;

	resource_size_t size;

	/*! For devices which are directly mapped into memory, i.e. NOR
	 * Flash or SDRAM. */
	resource_size_t map_base;

	void *platform_data; /*! board specific information about this device */

	/*! Devices of a particular class normaly need to store more
	 * information than struct device holds.
	 */
	void *priv;
	void *type_data;     /*! In case this device is a specific device, this pointer
			      * points to the type specific device, i.e. eth_device
			      */
	struct driver_d *driver; /*! The driver for this device */

	struct list_head list;     /* The list of all devices */
	struct list_head children; /* our children            */
	struct list_head sibling;
	struct list_head active;   /* The list of all devices which have a driver */

	struct device_d *parent;   /* our parent, NULL if not present */

	struct bus_type *bus;

	/*! The parameters for this device. This is used to carry information
	 * of board specific data from the board code to the device driver. */
	struct list_head parameters;

	struct list_head cdevs;
};

/** @brief Describes a driver present in the system */
struct driver_d {
	/*! The name of this driver. Used to match to
	 * the corresponding device. */
	char name[MAX_DRIVER_NAME];

	struct list_head list;

	/*! Called if an instance of a device is found */
	int     (*probe) (struct device_d *);

	/*! Called if an instance of a device is gone. */
	void     (*remove)(struct device_d *);

	void    (*info) (struct device_d *);
	void    (*shortinfo) (struct device_d *);

	unsigned long type;
	struct bus_type *bus;

	/*! This is somewhat redundant with the type data in struct device.
	 * Currently the filesystem implementation uses this field while
	 * ethernet drivers use the same field in struct device. Probably
	 * one of both should be removed. */
	void *type_data;
};

/*@}*/	/* do not delete, doxygen relevant */

#define RW_SIZE(x)      (x)
#define RW_SIZE_MASK    0x7

/* Register devices and drivers.
 */
int register_driver(struct driver_d *);
int register_device(struct device_d *);

/* Unregister a device. This function can fail, e.g. when the device
 * has children.
 */
int unregister_device(struct device_d *);

/* Organize devices in a tree. These functions do _not_ register or
 * unregister a device. Only registered devices are allowed here.
 */
int dev_add_child(struct device_d *dev, struct device_d *child);

/* Iterate over a devices children
 */
#define device_for_each_child(dev, child) \
	list_for_each_entry(child, &dev->children, sibling)

/* Iterate over a devices children - Safe against removal version
 */
#define device_for_each_child_safe(dev, tmpdev, child) \
	list_for_each_entry_safe(child, tmpdev, &dev->children, sibling)

/* Iterate through the devices of a given type. if last is NULL, the
 * first device of this type is returned. Put this pointer in as
 * 'last' to get the next device. This functions returns NULL if no
 * more devices are found.
 */
struct device_d *get_device_by_type(ulong type, struct device_d *last);
struct device_d *get_device_by_id(const char *id);
struct device_d *get_device_by_name(const char *name);

/* Find a free device id from the given template. This is archieved by
 * appending a number to the template. Dynamically created devices should
 * use this function rather than filling the id field themselves.
 */
int get_free_deviceid(const char *name_template);

char *deviceid_from_spec_str(const char *str, char **endp);

extern const char *dev_id(const struct device_d *dev);

static inline const char *dev_name(const struct device_d *dev)
{
	return dev_id(dev);
}

/* linear list over all available devices
 */
extern struct list_head device_list;

/* linear list over all available drivers
 */
extern struct list_head driver_list;

/* Iterate over all devices
 */
#define for_each_device(dev) list_for_each_entry(dev, &device_list, list)

/* Iterate over all drivers
 */
#define for_each_driver(drv) list_for_each_entry(drv, &driver_list, list)

/* Find a driver with the given name. Currently the filesystem implementation
 * uses this to get the driver from the name the user specifies with the
 * mount command
 */
struct driver_d *get_driver_by_name(const char *name);

struct cdev;

int     dev_protect(struct device_d *dev, size_t count, unsigned long offset, int prot);

/* These are used by drivers which work with direct memory accesses */
ssize_t mem_read(struct cdev *cdev, void *buf, size_t count, ulong offset, ulong flags);
ssize_t mem_write(struct cdev *cdev, const void *buf, size_t count, ulong offset, ulong flags);
int mem_memmap(struct cdev *cdev, void **map, int flags);

/* Use this if you have nothing to do in your drivers probe function */
int dummy_probe(struct device_d *);

/* Iterate over all activated devices (i.e. the ones with drivers and shut
 * them down.
 */
void devices_shutdown(void);

int generic_memmap_ro(struct cdev *dev, void **map, int flags);
int generic_memmap_rw(struct cdev *dev, void **map, int flags);

static inline off_t dev_lseek_default(struct cdev *cdev, off_t ofs)
{
	return ofs;
}

static inline int dev_open_default(struct device_d *dev, struct filep *f)
{
	return 0;
}

static inline int dev_close_default(struct device_d *dev, struct filep *f)
{
	return 0;
}

/* debugging and troubleshooting/diagnostic helpers. */

#define dev_printf(dev, format, arg...)	\
	printf("%s@%s: " format , (dev)->name , \
	       dev_name(dev) , ## arg)

#define dev_emerg(dev, format, arg...)		\
	dev_printf((dev) , format , ## arg)
#define dev_alert(dev, format, arg...)		\
	dev_printf((dev) , format , ## arg)
#define dev_crit(dev, format, arg...)		\
	dev_printf((dev) , format , ## arg)
#define dev_err(dev, format, arg...)		\
	dev_printf(dev , format , ## arg)
#define dev_warn(dev, format, arg...)		\
	dev_printf((dev) , format , ## arg)
#define dev_notice(dev, format, arg...)		\
	dev_printf((dev) , format , ## arg)
#define dev_info(dev, format, arg...)		\
	dev_printf((dev) , format , ## arg)

#if defined(DEBUG)
#define dev_dbg(dev, format, arg...)		\
	dev_printf((dev) , format , ## arg)
#else
#define dev_dbg(dev, format, arg...)		\
	({ if (0) dev_printf((dev), format, ##arg); 0; })
#endif

struct bus_type {
	char *name;
	int (*match)(struct device_d *dev, struct driver_d *drv);
	int (*probe)(struct device_d *dev);
	void (*remove)(struct device_d *dev);

	struct list_head list;
};

extern struct bus_type platform_bus;

struct file_operations {
	/*! Called in response of reading from this device. Required */
	ssize_t (*read)(struct cdev*, void* buf, size_t count, ulong offset, ulong flags);

	/*! Called in response of write to this device. Required */
	ssize_t (*write)(struct cdev*, const void* buf, size_t count, ulong offset, ulong flags);

	int (*ioctl)(struct cdev*, int, void *);
	off_t (*lseek)(struct cdev*, off_t);
	int (*open)(struct cdev*, unsigned long flags);
	int (*close)(struct cdev*);
	int (*flush)(struct cdev*);
	int (*erase)(struct cdev*, size_t count, unsigned long offset);
	int (*protect)(struct cdev*, size_t count, unsigned long offset, int prot);
	int (*memmap)(struct cdev*, void **map, int flags);
};

struct cdev {
	struct file_operations *ops;
	void *priv;
	struct device_d *dev;
	struct list_head list;
	struct list_head devices_list;
	char *name;
	unsigned long offset;
	size_t size;
	unsigned int flags;
	int open;
	struct mtd_info *mtd;
};

int devfs_create(struct cdev *);
int devfs_remove(struct cdev *);
struct cdev *cdev_by_name(const char *filename);
struct cdev *cdev_open(const char *name, unsigned long flags);
void cdev_close(struct cdev *cdev);
int cdev_flush(struct cdev *cdev);
ssize_t cdev_read(struct cdev *cdev, void *buf, size_t count, ulong offset, ulong flags);
ssize_t cdev_write(struct cdev *cdev, const void *buf, size_t count, ulong offset, ulong flags);
int cdev_ioctl(struct cdev *cdev, int cmd, void *buf);
int cdev_erase(struct cdev *cdev, size_t count, unsigned long offset);

#define DEVFS_PARTITION_FIXED		(1 << 0)
#define DEVFS_PARTITION_READONLY	(1 << 1)
#define DEVFS_IS_PARTITION		(1 << 2)
#define DEVFS_RDWR			(1 << 3)

int devfs_add_partition(const char *devname, unsigned long offset, size_t size,
		int flags, const char *name);
int devfs_del_partition(const char *name);

struct memory_platform_data {
	char *name;
	unsigned int flags;
};

#endif /* DRIVER_H */

