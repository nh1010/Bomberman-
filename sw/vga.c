/* * Device driver for the VGA video generator
 *
 * A Platform device implemented using the misc subsystem
 *
 * Stephen A. Edwards
 * Columbia University
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 * "make" to build
 * insmod vga_ball.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree vga_ball.c
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "hello.h"

#define DRIVER_NAME "vga_ball"

/* Device registers */
#define PLAYER1X(x) (x)
#define PLAYER1Y(x) ((x)+2)
#define TANK1_P(x) ((x)+4)
#define TANK1_D(x) ((x)+6)
#define ENEMY1_P(x) ((x)+8)
#define ENEMY1_D(x) ((x)+10)
#define ENEMY2_P(x) ((x)+12)
#define ENEMY2_D(x) ((x)+14)
#define ENEMY3_P(x) ((x)+16)
#define ENEMY3_D(x) ((x)+18)
#define ENEMY4_P(x) ((x)+20)
#define ENEMY4_D(x) ((x)+22)
#define BULLET1(x) ((x)+24)
#define BULLET2(x) ((x)+26)
#define BULLET3(x) ((x)+28)
#define BULLET4(x) ((x)+30)
#define BULLET5(x) ((x)+32)
#define RESERVE4(x) ((x)+34)
/*
 * Information about our device
 */
struct vga_ball_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
        game_info_t background;
} dev;

/*
 * Write segments of a single digit
 * Assumes digit is in range and the device information has been set up
 */
static void write_background(game_info_t *background)
{
	iowrite16(background->playerinfo00, PLAYER1X(dev.virtbase) );   //display player
	iowrite16(background->playerinfo01, PLAYER1Y(dev.virtbase) );   
	iowrite16(background->playerinfo10, TANK1_P(dev.virtbase) );
	iowrite16(background->playerinfo11, TANK1_D(dev.virtbase) );
	iowrite16(background->map_change_0, ENEMY1_P(dev.virtbase) );
	iowrite16(background->map_change_1, ENEMY1_D(dev.virtbase) ); 
	iowrite16(background->map_change_2, ENEMY2_P(dev.virtbase) );
	iowrite16(background->map_change_3, ENEMY2_D(dev.virtbase) ); 
	iowrite16(background->map_change_4, ENEMY3_P(dev.virtbase) );
	iowrite16(background->map_change_5, ENEMY3_D(dev.virtbase) ); 
	iowrite16(background->map_change_6, ENEMY4_P(dev.virtbase) );
	iowrite16(background->map_change_7, ENEMY4_D(dev.virtbase) ); 
    iowrite16(background->map_change_8, BULLET1(dev.virtbase) );
    iowrite16(background->map_change_9, BULLET2(dev.virtbase) );
    iowrite16(background->map_change_10, BULLET3(dev.virtbase) );
    iowrite16(background->map_change_11, BULLET4(dev.virtbase) );
    iowrite16(background->reserved_0, BULLET5(dev.virtbase) );
    iowrite16(background->reserved_1, RESERVE4(dev.virtbase) );


 	//iowrite16(0x7320, TANK1_D(dev.virtbase) );
	dev.background = *background;
}

/*
 * Handle ioctl() calls from userspace:
 * Read or write the segments on single digits.
 * Note extensive error checking of arguments
 */
static long vga_ball_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	vga_ball_arg_t vla;

	switch (cmd) {
	case VGA_BALL_WRITE_BACKGROUND:
		if (copy_from_user(&vla, (vga_ball_arg_t *) arg,
				   sizeof(vga_ball_arg_t)))
			return -EACCES;
		write_background(&vla.background);
		break;

	case VGA_BALL_READ_BACKGROUND:
	  	vla.background = dev.background;
		if (copy_to_user((vga_ball_arg_t *) arg, &vla,
				 sizeof(vga_ball_arg_t)))
			return -EACCES;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations vga_ball_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = vga_ball_ioctl,
};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice vga_ball_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &vga_ball_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init vga_ball_probe(struct platform_device *pdev)
{
        game_info_t beige = {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
                 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
                 0x0000,0x0000};
	int ret;

	/* Register ourselves as a misc device: creates /dev/vga_ball */
	ret = misc_register(&vga_ball_misc_device);

	/* Get the address of our registers from the device tree */
	ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	if (ret) {
		ret = -ENOENT;
		goto out_deregister;
	}

	/* Make sure we can use these registers */
	if (request_mem_region(dev.res.start, resource_size(&dev.res),
			       DRIVER_NAME) == NULL) {
		ret = -EBUSY;
		goto out_deregister;
	}

	/* Arrange access to our registers */
	dev.virtbase = of_iomap(pdev->dev.of_node, 0);
	if (dev.virtbase == NULL) {
		ret = -ENOMEM;
		goto out_release_mem_region;
	}
        
	/* Set an initial color */
        write_background(&beige);
		  

	return 0;

out_release_mem_region:
	release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
	misc_deregister(&vga_ball_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int vga_ball_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&vga_ball_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id vga_ball_of_match[] = {
	{ .compatible = "csee4840,vga_ball-1.0" },
	{},
};
MODULE_DEVICE_TABLE(of, vga_ball_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver vga_ball_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(vga_ball_of_match),
	},
	.remove	= __exit_p(vga_ball_remove),
};

/* Called when the module is loaded: set things up */
static int __init vga_ball_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	return platform_driver_probe(&vga_ball_driver, vga_ball_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit vga_ball_exit(void)
{
	platform_driver_unregister(&vga_ball_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(vga_ball_init);
module_exit(vga_ball_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stephen A. Edwards, Columbia University");
MODULE_DESCRIPTION("VGA ball driver");