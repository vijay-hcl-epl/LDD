// #includes
#include<linux/module.h>        // Is it needed?
#include<linux/kernel.h>        // Is it needed?
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
#include<linux/wait.h>          // To use Wait Queues
#include<linux/sched.h>         // To use current macro
#include "ioctl_basic.h"

#define DEVICE_NAME	"sleep"

// Device structure
// Device structure
// Always form your own device structure and embedd cdev
// This is an example of inheritance in C language where
// cdev is analogous to base class and hello is analogous to derived class
struct sleep
{
   // Below device variable just to track the no. of times
   // this device is opened,read,write and close
   int numopen;
   int numread;
   int numwrite;
   int numclose;
   char data[100];   //To store the data
   struct cdev cdev;
};

struct sleep hlo = {0};
dev_t dev_num;
int maj_num;

DECLARE_WAIT_QUEUE_HEAD(wq);
int wq_flag = 0;

static int sleep_open(struct inode *inode, struct file *filp) {
        struct sleep *phlo  =  NULL;

        // Below two statements are needed in almost all char drivers

        // Get the device pointer from inode struct
        phlo = container_of(inode->i_cdev, struct sleep, cdev);

        // Store the device pointer in filp->private_data so that this device pointer could be used
        // in all functions of file operations 
        filp->private_data = phlo;

        ++phlo->numopen;
        printk("Sleep driver opened\n");
        
	return 0;
}

ssize_t sleep_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
        struct sleep *phlo  =  filp->private_data;

        ++phlo->numread;
        printk("Sleep driver read\n");
        
        // Sleep until the condition (wq_flag != 0) is true
        // This version returns an integer value that you should check;
        // a nonzero value means your sleep was interrupted by signal, 
        // and your driver should probably return -ERESTARTSYS.
        wait_event_interruptible(wq, (wq_flag != 0));

        // Reset the flag so that next read waits for write to set it
        wq_flag = 0;
        printk("Awakens %s(%d)\n", current->comm, current->pid);

        // Remember filp are specific to process that opens this
        // so filp->f_pos stores the last accessed index
        copy_to_user(buffer, &phlo->data[filp->f_pos], count);

        // Always increment the *f_pos so that kernel remembers it
        // read and write should update the position using *f_pos
        // Kernel later uses this value and updates the value filp->f_pos
        *f_pos += count;

	return 0;
}

ssize_t sleep_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
        struct sleep *phlo  = filp->private_data; 

        ++phlo->numwrite;
        printk("Sleep driver write\n");

        // Remember filp are specific to process that opens this
        // so filp->f_pos stores the last accessed index
        copy_from_user(&phlo->data[filp->f_pos], buffer, count);

        // Always increment the *f_pos so that kernel remembers it
        // read and write should update the position using *f_pos
        // Kernel later uses this value and updates the value filp->f_pos
        *f_pos += count;

        // Wakeup all the sleeping process in the wait queue wq
        printk("Process %s(%d) awakening the readers..\n", current->comm, current->pid);
        wq_flag = 1;
        wake_up_interruptible(&wq);

	return 0;
}

int sleep_close(struct inode *inode, struct file *filp) {
        struct sleep *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct sleep, cdev);

        ++phlo->numclose;
        printk("Sleep driver close\n");
	return 0;
}

long sleep_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
    int ret=0;

switch(cmd) {

    case IOCTL_HELLO: 
        printk("Hello ioctl world\n");
        break;
    default: 
        printk("Wrong command\n");
        return 1;
} 
 
return ret;
 
}

// File Operations struct
// Declare after fops function definitions
struct file_operations fops = {
        .owner   = THIS_MODULE,
        .read    = sleep_read,
        .write   = sleep_write,
        .open    = sleep_open,
        .release = sleep_close,
        .unlocked_ioctl = sleep_ioctl,
};

static int sleep_init(void) {

        int ret = -1;
	
        // allocate a major number dynamically
        // Use register_chrdev_region if you know the device numbers you want
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("sleep : major number is %d\n",maj_num);
        printk("To create mknod for device file, execute 'mknod /dev/sleep c %d 0'\n", maj_num);

	//pcdev = cdev_alloc();   // Use it if you do not want to have your own device struct

        // Initialise the cdev field inside your structure
        cdev_init(&hlo.cdev, &fops);
        hlo.cdev.owner = THIS_MODULE;

	/* Now we created a cdev, we have to add it to the kernel
	int cdev_add(struct cdev *dev, dev_t num, unsigned int count) */
        // You should not call cdev_add until your driver is completely 
        // ready to handle operations on the device.
	ret = cdev_add(&hlo.cdev, dev_num, 1);
	if(ret < 0) {
		printk("sleep : Unable to add cdev to kernel\n");
	}

	return 0;
}

static void sleep_exit(void) {

        // To remove a char device from the system
	cdev_del(&hlo.cdev);
	
        // Do this after doing cdev_del
	unregister_chrdev_region(dev_num, 1);
	printk("sleep : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(sleep_init);
module_exit(sleep_exit);

