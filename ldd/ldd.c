// #includes
#include<linux/module.h>
#include<linux/device.h>
#include<linux/kernel.h>
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
#include<linux/wait.h>
#include<linux/sched.h>
#include <linux/signal.h>
#include "ioctl_basic.h"

#define DEVICE_NAME	"ldd"

// Device structure
struct ldd
{
   int numopen;
   int numread;
   int numwrite;
   int numclose;
   char data[100];
   char pos;
   struct cdev cdev;
   struct fasync_struct  *async_queue;
};

struct ldd hlo = {0};
dev_t dev_num;
int maj_num;

DECLARE_WAIT_QUEUE_HEAD(wq);
int wq_flag = 0;

//#define BUSTYPE
#define DEVICE

#ifdef BUSTYPE
static int ldd_match(struct device *dev, struct device_driver *driver)
{
//return !strncmp(dev->bus_id, driver->name, strlen(driver->name));
return 0;
}
/*
static int ldd_hotplug(struct device *dev, char **envp, int num_envp,
char *buffer, int buffer_size)
{
envp[0] = buffer;
if (snprintf(buffer, buffer_size, "LDDBUS_VERSION=%s",
Version) >= buffer_size)
return -ENOMEM;
envp[1] = NULL;
return 0;
}
*/
struct bus_type ldd_bus_type = {
.name = "ldd",
.match = ldd_match,
//.hotplug = ldd_hotplug,
};

static ssize_t show_bus_version(struct bus_type *bus, char *buf)
{
//return snprintf(buf, PAGE_SIZE, "%s\n", Version);
printk("show_bus_version called..\n");
return 1;
}

static BUS_ATTR(version, S_IRUGO, show_bus_version, NULL);
#endif

#ifdef DEVICE
static void ldd_bus_release(struct device *dev)
{
printk(KERN_DEBUG "lddbus release\n");
}

static ssize_t show_dev_version(struct device *bus, char *buf)
{
//return snprintf(buf, PAGE_SIZE, "%s\n", Version);
printk("show_dev_version called..\n");
return 1;
}


struct device ldd_bus = {
.init_name    = "ldd0",
.release = ldd_bus_release
};

static DEVICE_ATTR(version, S_IRUGO, show_dev_version, NULL);
#endif

static int ldd_open(struct inode *inode, struct file *filp) {
        struct ldd *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct ldd, cdev);
        filp->private_data = phlo;

        ++phlo->numopen;
        printk("ldd driver opened\n");
        
	return 0;
}

ssize_t ldd_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
        struct ldd *phlo  =  filp->private_data;

        ++phlo->numread;
        printk("ldd driver read\n");
        
        copy_to_user(buffer, &phlo->data[filp->f_pos], count);
        *f_pos += count;

	return 0;
}

ssize_t ldd_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
        struct ldd *phlo  = filp->private_data; 

        ++phlo->numwrite;
        printk("ldd driver write\n");

        if (phlo->async_queue)
           kill_fasync(&phlo->async_queue, SIGIO, POLL_IN);

        copy_from_user(&phlo->data[filp->f_pos], buffer, count);
        *f_pos += count;

	return 0;
}

int ldd_close(struct inode *inode, struct file *filp) {
        struct ldd *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct ldd, cdev);

        ++phlo->numclose;
        printk("ldd driver close\n");
	return 0;
}

long ldd_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
    int ret=0;

switch(cmd) {

    case IOCTL_ADDBUS: 
        //ret = bus_register(&ldd_bus_type);
        //if (ret)
         // printk("bus_register FAILED..!!\n");

        break;
    case IOCTL_REMBUS: 
        //bus_unregister(&ldd_bus_type);

        break;
    default: 
        printk("Wrong commandi\n");
        return 1;
} 
 
return ret;
 
}

static int ldd_p_fasync(int fd, struct file *filp, int mode)
{
    struct ldd *dev = filp->private_data;
    
    printk("ldd Driver fasync called..\n");

    return fasync_helper(fd, filp, mode, &dev->async_queue);
}
// File Operations struct
struct file_operations fops = {
        .owner   = THIS_MODULE,
        .read    = ldd_read,
        .write   = ldd_write,
        .open    = ldd_open,
        .release = ldd_close,
        .unlocked_ioctl = ldd_ioctl,
        .fasync  = ldd_p_fasync
};

static int ldd_init(void) {

        int ret = -1;
	
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("Hello : major number is %d\n",maj_num);
	printk("Use mknod for device file - mknod /dev/ldd c %d 0\n", maj_num);

	//pcdev = cdev_alloc();
        cdev_init(&hlo.cdev, &fops);
        hlo.cdev.owner = THIS_MODULE;
        printk("The addr of cdev is %x\n", (unsigned int)&hlo.cdev);

#ifdef BUSTYPE
        ret = bus_register(&ldd_bus_type);
        if (ret)
          printk("bus_register FAILED..!!\n");

        if (bus_create_file(&ldd_bus_type, &bus_attr_version))
            printk(KERN_NOTICE "Unable to create version attribute\n");
#endif

#ifdef DEVICE
        ret = device_register(&ldd_bus);
        if (ret)
            printk("Unable to register ldd0\n");
        
        ret = device_create_file(&ldd_bus, &dev_attr_version);
        if(ret)
            printk("device_create_file failed..\n");
#endif

	/* Now we created a cdev, we have to add it to the kernel
	int cdev_add(struct cdev *dev, dev_t num, unsigned int count) */
	ret = cdev_add(&hlo.cdev, dev_num, 1);
	if(ret < 0) {
		printk("Helloworld : Unable to add cdev to kernel\n");
	}
     
	return 0;
}

static void ldd_exit(void) {
#ifdef BUSTYPE
        bus_remove_file(&ldd_bus_type, &bus_attr_version);
        bus_unregister(&ldd_bus_type);
#endif

#ifdef DEVICE
        device_remove_file(&ldd_bus, &dev_attr_version);
        device_unregister(&ldd_bus);
#endif

	cdev_del(&hlo.cdev);
	
	unregister_chrdev_region(dev_num, 1);
	printk("Helloworld : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(ldd_init);
module_exit(ldd_exit);

MODULE_LICENSE("GPL");
