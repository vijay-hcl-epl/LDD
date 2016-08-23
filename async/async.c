// #includes
#include<linux/module.h>        // Is it needed?
#include<linux/device.h>
#include<linux/kernel.h>        // Is it needed?
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
#include<linux/wait.h>          // To use Wait Queues
#include<linux/sched.h>
#include <linux/signal.h>

#define DEVICE_NAME	"async"

// Device structure
struct async
{
	int numopen;
	int numread;
	int numwrite;
	int numclose;
	char data[100];
	struct cdev cdev;
	struct fasync_struct  *async_queue;
};

struct async hlo = {0};
dev_t dev_num;
int maj_num;

DECLARE_WAIT_QUEUE_HEAD(wq);
int wq_flag = 0;

static int async_open(struct inode *inode, struct file *filp) {
	struct async *phlo  =  NULL;

	// Below two statements are needed in almost all char drivers

	// Get the device pointer from inode struct
	phlo = container_of(inode->i_cdev, struct async, cdev);

	// Store the device pointer in filp->private_data so that this device pointer could be used
	// in all functions of file operations 
	filp->private_data = phlo;

	++phlo->numopen;
	printk("async driver opened\n");

	return 0;
}

ssize_t async_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
	struct async *phlo  =  filp->private_data;

	++phlo->numread;
	printk("async driver read\n");

	// Remember filp are specific to process that opens this
	// so filp->f_pos stores the last accessed index
	copy_to_user(buffer, &phlo->data[filp->f_pos], count);

	// Always increment the *f_pos so that kernel remembers it
	// read and write should update the position using *f_pos
	// Kernel later uses this value and updates the value filp->f_pos
	*f_pos += count;

	return 0;
}

ssize_t async_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
	struct async *phlo  = filp->private_data; 

	++phlo->numwrite;
	printk("async driver write\n");

	if (phlo->async_queue)
		kill_fasync(&phlo->async_queue, SIGIO, POLL_IN); // SIgnal the interested process

	copy_from_user(&phlo->data[filp->f_pos], buffer, count);
	*f_pos += count;

	return 0;
}

int async_close(struct inode *inode, struct file *filp) {
	struct async *phlo  =  NULL;

	phlo = container_of(inode->i_cdev, struct async, cdev);

	++phlo->numclose;
	printk("async driver close\n");
	return 0;
}

// When F_SETFL is executed to turn on FASYNC, the driver’s fasync method is called.
static int async_p_fasync(int fd, struct file *filp, int mode)
{
	struct async *dev = filp->private_data;

	printk("async Driver fasync called..\n");

        // fasync_helper is invoked to add or remove entries from the list of interested processes
        // when the FASYNC flag changes for an open file.
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}

// File Operations struct
// Declare after fops function definitions
struct file_operations fops = {
	.owner   = THIS_MODULE,
	.read    = async_read,
	.write   = async_write,
	.open    = async_open,
	.release = async_close,
	.fasync  = async_p_fasync
};

static int async_init(void) {

	int ret = -1;

	// allocate a major number dynamically
	// Use register_chrdev_region if you know the device numbers you want
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("async : major number is %d\n",maj_num);
        printk("To create mknod for device file, execute 'mknod /dev/async c %d 0'\n", maj_num);

	//pcdev = cdev_alloc();  // Use it if you do not want to have your own device struct

        // Initialise the cdev field inside your structure
	cdev_init(&hlo.cdev, &fops);
	hlo.cdev.owner = THIS_MODULE;

	/* Now we created a cdev, we have to add it to the kernel
	   int cdev_add(struct cdev *dev, dev_t num, unsigned int count) */
	// You should not call cdev_add until your driver is completely 
	// ready to handle operations on the device.
	ret = cdev_add(&hlo.cdev, dev_num, 1);
	if(ret < 0) {
		printk("Helloworld : Unable to add cdev to kernel\n");
	}

	return 0;
}

static void async_exit(void) {

        // To remove a char device from the system
	cdev_del(&hlo.cdev);

        // Do this after doing cdev_del
	unregister_chrdev_region(dev_num, 1);
	printk("async : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(async_init);
module_exit(async_exit);


