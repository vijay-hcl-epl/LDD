// #includes
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
#include<linux/wait.h>
#include<linux/sched.h>
#include "ioctl_basic.h"

#define DEVICE_NAME	"sleep"

// Device structure
struct sleep
{
   int numopen;
   int numread;
   int numwrite;
   int numclose;
   char data[100];
   char pos;
   struct cdev cdev;
};

struct sleep hlo = {0};
dev_t dev_num;
int maj_num;

DECLARE_WAIT_QUEUE_HEAD(wq);
int wq_flag = 0;

static int sleep_open(struct inode *inode, struct file *filp) {
        struct sleep *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct sleep, cdev);
        filp->private_data = phlo;

        ++phlo->numopen;
        printk("Sleep driver opened\n");
        
	return 0;
}

ssize_t sleep_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
        struct sleep *phlo  =  filp->private_data;

        ++phlo->numread;
        printk("Sleep driver read\n");
        
        wait_event_interruptible(wq, (wq_flag != 0));
        wq_flag = 0;
        printk("Awaken %s(%d)\n", current->comm, current->pid);
        copy_to_user(buffer, &phlo->data[filp->f_pos], count);
        *f_pos += count;

	return 0;
}

ssize_t sleep_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
        struct sleep *phlo  = filp->private_data; 

        ++phlo->numwrite;
        printk("Sleep driver write\n");

        printk("Process %s(%d) awakening the readers..\n", current->comm, current->pid);

        wq_flag = 1;
        wake_up_interruptible(&wq);

        copy_from_user(&phlo->data[filp->f_pos], buffer, count);
        *f_pos += count;

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
        printk("Wrong commandi\n");
        return 1;
} 
 
return ret;
 
}

// File Operations struct
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
	
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("Hello : major number is %d\n",maj_num);
	printk("Use mknod for device file\n");

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

static void sleep_exit(void) {

	cdev_del(&hlo.cdev);
	
	unregister_chrdev_region(dev_num, 1);
	printk("Helloworld : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(sleep_init);
module_exit(sleep_exit);


