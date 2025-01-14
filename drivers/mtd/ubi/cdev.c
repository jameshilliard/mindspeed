#include <common.h>
#include <fcntl.h>
#include <fs.h>
#include <ioctl.h>
#include "ubi-barebox.h"
#include "ubi.h"

struct ubi_volume_cdev_priv {
	struct ubi_device *ubi;
	struct ubi_volume *vol;
	int written;
};

static ssize_t ubi_volume_cdev_read(struct cdev *cdev, void *buf, size_t size,
		unsigned long offset, unsigned long flags)
{
	struct ubi_volume_cdev_priv *priv = cdev->priv;
	struct ubi_volume *vol = priv->vol;
	struct ubi_device *ubi = priv->ubi;
	int err, lnum, off, len;
	size_t count_save = size;
	unsigned long long tmp;
	loff_t offp = offset;
	int usable_leb_size = vol->usable_leb_size;

	printf("%s: %d @ 0x%08lx\n", __func__, size, offset);

	len = size > usable_leb_size ? usable_leb_size : size;

	tmp = offp;
	off = do_div(tmp, usable_leb_size);
	lnum = tmp;
	do {
		if (off + len >= usable_leb_size)
			len = usable_leb_size - off;

		err = ubi_eba_read_leb(ubi, vol, lnum, buf, off, len, 0);
		if (err) {
			printf("read err %x\n", err);
			break;
		}
		off += len;
		if (off == usable_leb_size) {
			lnum += 1;
			off -= usable_leb_size;
		}

		size -= len;
		offp += len;

		buf += len;
		len = size > usable_leb_size ? usable_leb_size : size;
	} while (size);

	return count_save;
}

static ssize_t ubi_volume_cdev_write(struct cdev* cdev, const void *buf,
		size_t size, unsigned long offset, unsigned long flags)
{
	struct ubi_volume_cdev_priv *priv = cdev->priv;
	struct ubi_volume *vol = priv->vol;
	struct ubi_device *ubi = priv->ubi;
	int err;

	if (!priv->written) {
		err = ubi_start_update(ubi, vol, vol->used_bytes);
		if (err < 0) {
			printf("Cannot start volume update\n");
			return err;
		}
	}

	err = ubi_more_update_data(ubi, vol, buf, size);
	if (err < 0) {
		printf("Couldnt or partially wrote data \n");
		return err;
	}
	priv->written += size;

	return size;
}

static int ubi_volume_cdev_open(struct cdev *cdev, unsigned long flags)
{
	struct ubi_volume_cdev_priv *priv = cdev->priv;

	priv->written = 0;

	return 0;
}

static int ubi_volume_cdev_close(struct cdev *cdev)
{
	struct ubi_volume_cdev_priv *priv = cdev->priv;
	struct ubi_volume *vol = priv->vol;
	struct ubi_device *ubi = priv->ubi;
	int err;

	if (priv->written) {
		int remaining = vol->usable_leb_size -
				(priv->written % vol->usable_leb_size);

		if (remaining) {
			void *buf = xzalloc(remaining);

			memset(buf, 0xff, remaining);

			err = ubi_more_update_data(ubi, vol, buf, remaining);

			free(buf);

			if (err < 0) {
				printf("Couldnt or partially wrote data \n");
				return err;
			}
		}
		err = ubi_finish_update(ubi, vol);
		if (err)
			return err;

		err = ubi_check_volume(ubi, vol->vol_id);
		if (err < 0) {
			printf("check failed: %s\n", strerror(err));
			return err;
		}

		if (err) {
			ubi_warn("volume %d on UBI device %d is corrupted",
					vol->vol_id, ubi->ubi_num);
			vol->corrupted = 1;
		}

		vol->checked = 1;
		ubi_gluebi_updated(vol);
	}

	return 0;
}

static off_t ubi_volume_cdev_lseek(struct cdev *cdev, off_t ofs)
{
	struct ubi_volume_cdev_priv *priv = cdev->priv;

	/* We can only update ubi volumes sequentially */
	if (priv->written)
		return -EINVAL;

	return ofs;
}

static struct file_operations ubi_volume_fops = {
	.open	= ubi_volume_cdev_open,
	.close	= ubi_volume_cdev_close,
	.read   = ubi_volume_cdev_read,
	.write  = ubi_volume_cdev_write,
	.lseek	= ubi_volume_cdev_lseek,
};

int ubi_volume_cdev_add(struct ubi_device *ubi, struct ubi_volume *vol)
{
	struct cdev *cdev = &vol->cdev;
	struct ubi_volume_cdev_priv *priv;
	int ret;

	priv = xzalloc(sizeof(*priv));

	priv->vol = vol;
	priv->ubi = ubi;

	cdev->ops = &ubi_volume_fops;
	cdev->name = asprintf("ubi%d.%s", ubi->ubi_num, vol->name);
	cdev->priv = priv;
	cdev->size = vol->used_bytes;
	printf("registering %s as /dev/%s\n", vol->name, cdev->name);
	ret = devfs_create(cdev);
	if (ret) {
		free(priv);
		free(cdev->name);
	}

	return 0;
}

void ubi_volume_cdev_remove(struct ubi_volume *vol)
{
	struct cdev *cdev = &vol->cdev;
	struct ubi_volume_cdev_priv *priv = cdev->priv;

	devfs_remove(cdev);
	free(cdev->name);
	free(priv);
}

static int ubi_cdev_ioctl(struct cdev *cdev, int cmd, void *buf)
{
	struct ubi_volume_desc *desc;
	struct ubi_device *ubi = cdev->priv;
	struct ubi_mkvol_req *req = buf;

	switch (cmd) {
	case UBI_IOCRMVOL:
		desc = ubi_open_volume_nm(ubi->ubi_num, req->name,
                                           UBI_EXCLUSIVE);
		if (IS_ERR(desc))
			return PTR_ERR(desc);
		ubi_remove_volume(desc);
		break;
	case UBI_IOCMKVOL:
		if (!req->bytes)
			req->bytes = ubi->avail_pebs * ubi->leb_size;
		return ubi_create_volume(ubi, req);
	};

	return 0;
}


static struct file_operations ubi_fops = {
	.ioctl	= ubi_cdev_ioctl,
};

int ubi_cdev_add(struct ubi_device *ubi)
{
	struct cdev *cdev = &ubi->cdev;
	int ret;

	cdev->ops = &ubi_fops;
	cdev->name = asprintf("ubi%d", ubi->ubi_num);
	cdev->priv = ubi;
	cdev->size = 0;

	printf("registering /dev/%s\n", cdev->name);
	ret = devfs_create(cdev);
	if (ret)
		free(cdev->name);

	return ret;
}

void ubi_cdev_remove(struct ubi_device *ubi)
{
	struct cdev *cdev = &ubi->cdev;

	printf("removing %s\n", cdev->name);

	devfs_remove(cdev);
	free(cdev->name);
}
