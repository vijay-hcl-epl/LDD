// #includes
#include<linux/module.h>
#include <device.h>
#include<linux/kernel.h>
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
#include<linux/wait.h>
#include<linux/sched.h>
#include <linux/signal.h>
#include "ioctl_basic.h"

#define DEVICE_NAME	"async"

// Device structure
struct async
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

struct async hlo = {0};
dev_t dev_num;
int maj_num;

DECLARE_WAIT_QUEUE_HEAD(wq);
int wq_flag = 0;

static int async_open(struct inode *inode, struct file *filp) {
        struct async *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct async, cdev);
        filp->private_data = phlo;

        ++phlo->numopen;
        printk("async driver opened\n");
        
	return 0;
}

ssize_t async_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
        struct async *phlo  =  filp->private_data;

        ++phlo->numread;
        printk("async driver read\n");
        
        copy_to_user(buffer, &phlo->data[filp->f_pos], count);
        *f_pos += count;

	return 0;
}

ssize_t async_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
        struct async *phlo  = filp->private_data; 

        ++phlo->numwrite;
        printk("async driver write\n");

        if (phlo->async_queue)
           kill_fasync(&phlo->async_queue, SIGIO, POLL_IN);

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

long async_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
    int ret=0;

switch(cmd) {

    case IOCTL_HELLO: 
        printk("Hello ioctl world\n");
        break;
    default: 
        printk("Wrong commandi\n");
        return 1;
} 
 
return ret;
 
}

static int async_p_fasync(int fd, struct file *filp, int mode)
{
    struct async *dev = filp->private_data;
    
    printk("async Driver fasync called..\n");

    return fasync_helper(fd, filp, mode, &dev->async_queue);
}
// File Operations struct
struct file_operations fops = {
        .owner   = THIS_MODULE,
        .read    = async_read,
        .write   = async_write,
        .open    = async_open,
        .release = async_close,
        .unlocked_ioctl = async_ioctl,
        .fasync  = async_p_fasync
};

static int async_init(void) {

        int ret = -1;
	
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("Hello : major number is %d\n",maj_num);
	printk("Use mknod for device file - mknod /dev/async c %d 0\n", maj_num);

	//pcdev = cdev_alloc();
        cdev_init(&hlo.cdev, &fops);
        hlo.cdev.owner = THIS_MODULE;
        printk("The addr of cdev is %x\n", (unsigned int)&hlo.cdev);

	/* Now we created a cdev, we have to add it to the kernel
	int cdev_add(struct cdev *dev, dev_t num, unsigned int count) */
	ret = cdev_add(&hlo.cdev, dev_num, 1);
	if(ret < 0) {
		printk("Helloworld : Unable to add cdev to kernel\n");
	}

	return 0;
}

static void async_exit(void) {

	cdev_del(&hlo.cdev);
	
	unregister_chrdev_region(dev_num, 1);
	printk("Helloworld : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(async_init);
module_exit(async_exit);


