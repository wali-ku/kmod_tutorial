/*
 *  kmap.c - Illustrative kernel module show-casing memory mapped IO between
 *  	     user-space and kernel
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include "ku_map.h"

static int major_num;
static dev_t first_char_dev;
static struct cdev char_dev;
static struct class *dev_class;
static void *mmap_buffer = NULL;

static int device_open(struct inode *inode, struct file *filp)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *filp)
{
	module_put(THIS_MODULE);
	return 0;
}

static int device_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long page, pos;
	unsigned long start = (unsigned long)vma->vm_start;
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);

	if (size > MMAP_BUF_SIZE) {
		printk(KERN_ERR "mmap size exceeds limit=%d\n", MMAP_BUF_SIZE);
		return -EINVAL;
	}

	pos = (unsigned long)mmap_buffer;
	page = ((virt_to_phys((void *)pos)) >> PAGE_SHIFT);

	if (remap_pfn_range(vma, start, page, size, PAGE_SHARED)) {
		printk(KERN_ERR "Failed to remap the pages\n");
		return -EAGAIN;
	}

	return 0;
}

static const struct file_operations cdev_control_fops = {
	.mmap		= device_mmap,
	.open 		= device_open,
	.release	= device_release,
};

int init_module(void)
{
	void *ret = NULL;

	major_num = alloc_chrdev_region(&first_char_dev, 0, 1, DEV_NAME);
	if (major_num < 0) {
		printk(KERN_ERR "Failed to allocate character device\n");
		return major_num;
	}

	dev_class = class_create(THIS_MODULE, DEV_NAME);
	if (dev_class == NULL) {
		printk(KERN_ERR "Character device class creation failed\n");
		goto err_class;
	}

	ret = device_create(dev_class, NULL, first_char_dev, NULL, DEV_NAME);
	if (ret == NULL) {
		printk(KERN_ERR "Character device creation failed\n");
		goto err_dev;

	}

	cdev_init(&char_dev, &cdev_control_fops);
	if (cdev_add(&char_dev, first_char_dev, 1) == -1) {
		printk(KERN_ERR "Failed to init or add character device\n");
		goto err_init;
	}

	mmap_buffer = (void *) get_zeroed_page(GFP_KERNEL);
	if (mmap_buffer == NULL) {
		printk(KERN_ERR "Failed to allocate buffer for mmap\n");
		return -EINVAL;
	}

	return 0;

err_init:
	device_destroy(dev_class, first_char_dev);
err_dev:
	class_destroy(dev_class);
err_class:
	unregister_chrdev_region(first_char_dev, 1);
	return -EAGAIN;
}

void cleanup_module(void)
{
	printk(KERN_INFO "budget=%d\n",
			((struct user_info *)mmap_buffer)->budget);

	cdev_del(&char_dev);
	device_destroy(dev_class, first_char_dev);
	class_destroy(dev_class);
	unregister_chrdev_region(first_char_dev, 1);
	free_page((unsigned long)mmap_buffer);

	return;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Skidro <wali@ku.edu>");
