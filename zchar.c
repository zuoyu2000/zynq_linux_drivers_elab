#include "zchar.h"

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include <asm/uaccess.h>
#include <xen/page.h>

static dev_t dev_num;		// Global variable for the device number
static struct cdev c_dev;	// Global variable for the character device structure
static struct class *cl;	// Global variable for the device class
struct resource *res;		// Device Resource Structure

char *zchar_addr;

static int zchar_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "<%s> file: open()\n", MODULE_NAME);
	return 0;
}

static int zchar_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "<%s> file: close()\n", MODULE_NAME);
	return 0;
}

static ssize_t zchar_read(struct file *f, char __user * buf, size_t
			  len, loff_t * off)
{
	printk(KERN_INFO "<%s> file: read()\n", MODULE_NAME);
	return simple_read_from_buffer(buf, len, off, zchar_addr, MEM_LENGTH);
}

static ssize_t zchar_write(struct file *f, const char __user * buf,
			   size_t len, loff_t * off)
{
	printk(KERN_INFO "<%s> file: write()\n", MODULE_NAME);
	if (len > (MEM_LENGTH - 1))
		return -EINVAL;

	copy_from_user(zchar_addr, buf, len);
	zchar_addr[len] = '\0';
	return len;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = zchar_open,
	.release = zchar_close,
	.read = zchar_read,
	.write = zchar_write,
};

// device match table to match with device node in device tree
static struct of_device_id zchar_of_match[] = {
	{ .compatible = "xlnx,top-conv-1.00.a", },
	{}
};
MODULE_DEVICE_TABLE(of, zchar_of_match);

static int zchar_probe(struct platform_device *pdev)
{
   res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "No memory resource\n");
		return -ENODEV;
	}
	
	if (!request_mem_region(res->start, MEM_LENGTH, pdev->name)) {
		dev_err(&pdev->dev, "Cannot request IO\n");
		return -ENXIO;
	}

	// device constructor:
	printk(KERN_INFO "<%s> init: registered\n", MODULE_NAME);
	if (alloc_chrdev_region(&dev_num, 0, 1, MODULE_NAME) < 0) {
		return -1;
	}
	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}
	if (device_create(cl, NULL, dev_num, NULL, MODULE_NAME) == NULL) {
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}
	cdev_init(&c_dev, &fops);
	if (cdev_add(&c_dev, dev_num, 1) == -1) {
		device_destroy(cl, dev_num);
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}

	// allocate mmap area:
	zchar_addr = ioremap(res->start, MEM_LENGTH);
	if (!zchar_addr) {
		printk(KERN_ERR "<%s> Error: allocating memory failed\n",
		       MODULE_NAME);

		return -ENOMEM;
	}

	return 0;
}

static void zchar_shutdown(struct platform_device *pdev)
{
	iowrite32(0, zchar_addr); // not really needed here, but ...
}

static int zchar_remove(struct platform_device *pdev)
{
   zchar_shutdown(pdev);
   
	// device destructor:
	cdev_del(&c_dev);
	device_destroy(cl, dev_num);
	class_destroy(cl);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_INFO "<%s> exit: unregistered\n", MODULE_NAME);

	// free mmap area:
	if (zchar_addr) {
		iounmap(zchar_addr);
	}
	
	// Release the region:
	release_mem_region(res->start, MEM_LENGTH);
	
	return 0;
}

// platform driver structure for zchar driver:
static struct platform_driver zchar_driver = {
	.driver = {
		   .name = MODULE_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = zchar_of_match},
	.probe = zchar_probe,
	.remove = zchar_remove,
	.shutdown = zchar_shutdown,
};

// Register zchar platform driver:
module_platform_driver(zchar_driver);

MODULE_AUTHOR("e-Lab");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(MODULE_NAME "zchar");
MODULE_ALIAS(MODULE_NAME);

