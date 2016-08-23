// #includes
#include<linux/module.h>        // Is it needed?
#include<linux/device.h>
#include<linux/kernel.h>        // Is it needed?
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user

#define DEVICE_NAME	"ldd"

// Device structure
struct ldd
{
   int numopen;
   int numread;
   int numwrite;
   int numclose;
   char data[100];
   struct cdev cdev;
};

struct ldd hlo = {0};
dev_t dev_num;
int maj_num;

// Any one  of these below 2 macros should be uncommented for 
// this project to work
#define BUSTYPE
//#define DEVICE

#ifdef BUSTYPE
static int lddbus_match(struct device *dev, struct device_driver *driver)
{
return 0;
}

struct bus_type ldd_bus_type = {
.name = "lddbus",
.match = lddbus_match,
};

static ssize_t show_bus_version(struct bus_type *bus, char *buf)
{
printk("show_bus_version called..\n");
return 1;
}

static BUS_ATTR(version, S_IRUGO, show_bus_version, NULL);
#endif

#ifdef DEVICE
static void ldd_dev_release(struct device *dev)
{
printk(KERN_DEBUG "lddbus release\n");
}

static ssize_t show_dev_version(struct device *bus, char *buf)
{
printk("show_dev_version called..\n");
return 1;
}


struct device ldd_dev = {
.init_name    = "ldd0",
.release = ldd_dev_release
};

static DEVICE_ATTR(version, S_IRUGO, show_dev_version, NULL);
#endif

static int ldd_open(struct inode *inode, struct file *filp) {
        struct ldd *phlo  =  NULL;

	// Below two statements are needed in almost all char drivers

	// Get the device pointer from inode struct
        phlo = container_of(inode->i_cdev, struct ldd, cdev);

	// Store the device pointer in filp->private_data so that this device pointer could be used
	// in all functions of file operations 
        filp->private_data = phlo;

        ++phlo->numopen;
        printk("ldd driver opened\n");
        
	return 0;
}

ssize_t ldd_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
        struct ldd *phlo  =  filp->private_data;

        ++phlo->numread;
        printk("ldd driver read\n");
        
	// Remember filp are specific to process that opens this
	// so filp->f_pos stores the last accessed index
        copy_to_user(buffer, &phlo->data[filp->f_pos], count);

	// Always increment the *f_pos so that kernel remembers it
	// read and write should update the position using *f_pos
	// Kernel later uses this value and updates the value filp->f_pos
        *f_pos += count;

	return 0;
}

ssize_t ldd_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
        struct ldd *phlo  = filp->private_data; 

        ++phlo->numwrite;
        printk("ldd driver write\n");

	// Remember filp are specific to process that opens this
	// so filp->f_pos stores the last accessed index
        copy_from_user(&phlo->data[filp->f_pos], buffer, count);

	// Always increment the *f_pos so that kernel remembers it
	// read and write should update the position using *f_pos
	// Kernel later uses this value and updates the value filp->f_pos
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

// File Operations struct
// Declare after fops function definitions
struct file_operations fops = {
        .owner   = THIS_MODULE,
        .read    = ldd_read,
        .write   = ldd_write,
        .open    = ldd_open,
        .release = ldd_close,
};

static int ldd_init(void) {

        int ret = -1;
	
	// allocate a major number dynamically
	// Use register_chrdev_region if you know the device numbers you want
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("Hello : major number is %d\n",maj_num);
        printk("To create mknod for device file, execute 'mknod /dev/ldd c %d 0'\n", maj_num);

	//pcdev = cdev_alloc();  // Use it if you do not want to have your own device struct

        // Initialise the cdev field inside your structure
        cdev_init(&hlo.cdev, &fops);
        hlo.cdev.owner = THIS_MODULE;

#ifdef BUSTYPE
        // Register the bus type
        ret = bus_register(&ldd_bus_type);
        if (ret)
          printk("bus_register FAILED..!!\n");

        // Create the attribute file
        if (bus_create_file(&ldd_bus_type, &bus_attr_version))
            printk(KERN_NOTICE "Unable to create version attribute\n");
#endif

#ifdef DEVICE
        // Register the device
        ret = device_register(&ldd_dev);
        if (ret)
            printk("Unable to register ldd0\n");
        
        // Create the attribute file
        ret = device_create_file(&ldd_dev, &dev_attr_version);
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
        device_remove_file(&ldd_dev, &dev_attr_version);
        device_unregister(&ldd_dev);
#endif

        // To remove a char device from the system
	cdev_del(&hlo.cdev);
	
        // Do this after doing cdev_del
	unregister_chrdev_region(dev_num, 1);
	printk("ldd : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(ldd_init);
module_exit(ldd_exit);

MODULE_LICENSE("GPL");
