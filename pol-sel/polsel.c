// #includes
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
//#include<linux/wait.h>
//#include<linux/sched.h>
#include <linux/poll.h>
#include "ioctl_basic.h"

#define DEVICE_NAME	"polsel"

// Device structure
struct polsel
{
   int numopen;
   int numread;
   int numwrite;
   int numclose;
   char data[100];
   char rdpos;
   char wrpos;
   struct cdev cdev;
};

struct polsel hlo = {0};
dev_t dev_num;
int maj_num;

DECLARE_WAIT_QUEUE_HEAD(wq);
int wq_flag = 0;

static int polsel_open(struct inode *inode, struct file *filp) {
        struct polsel *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct polsel, cdev);
        filp->private_data = phlo;

        ++phlo->numopen;
        printk("polsel driver opened\n");
        
	return 0;
}

ssize_t polsel_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
        struct polsel *phlo  =  filp->private_data;

        ++phlo->numread;
        printk("polsel driver read\n");

        printk("In Read phlo->wrpos is %d\n", phlo->wrpos);
        printk("In Read phlo->rdpos is %d\n", phlo->rdpos);
        
        if(phlo->wrpos > phlo->rdpos)
        {
            if((phlo->wrpos - phlo->rdpos) < count)
                count = phlo->wrpos - phlo->rdpos;

            copy_to_user(buffer, &phlo->data[filp->f_pos], count);
            *f_pos += count;
            phlo->rdpos += count;
        }

	return 0;
}

ssize_t polsel_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
        struct polsel *phlo  = filp->private_data; 

        ++phlo->numwrite;
        printk("polsel driver write\n");

        printk("In Write phlo->wrpos is %d\n", phlo->wrpos);
        printk("In Write phlo->rdpos is %d\n", phlo->rdpos);
        //if(phlo->wrpos == phlo->rdpos)
        {
            copy_from_user(&phlo->data[filp->f_pos], buffer, count);
            *f_pos += count;
            phlo->wrpos += count;
        }

	return 0;
}

int polsel_close(struct inode *inode, struct file *filp) {
        struct polsel *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct polsel, cdev);

        ++phlo->numclose;
        printk("polsel driver close\n");
	return 0;
}

long polsel_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
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

static unsigned int polsol_poll(struct file *filp, poll_table *wait)
{
    struct polsel *phlo  = filp->private_data;
    unsigned int mask = 0;

        printk("In Poll phlo->wrpos is %d\n", phlo->wrpos);
        printk("In Poll phlo->rdpos is %d\n", phlo->rdpos);

    poll_wait(filp, &wq, wait);   
    printk("Out of poll_wait..\n");
    
    if(phlo->wrpos > phlo->rdpos)
    {
        printk("Setting MASKS..\n");
        mask |= POLLIN | POLLRDNORM;
    }
    
    printk("returning..\n");
    return mask;
}

// File Operations struct
struct file_operations fops = {
        .owner   = THIS_MODULE,
        .read    = polsel_read,
        .write   = polsel_write,
        .open    = polsel_open,
        .release = polsel_close,
        .unlocked_ioctl = polsel_ioctl,
        .poll    = polsol_poll
};

static int polsel_init(void) {

        int ret = -1;
	
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("Hello : major number is %d\n",maj_num);
	printk("Use mknod for device file - mknod /dev/polsel c %d 0\n", maj_num);

	//pcdev = cdev_alloc();
        cdev_init(&hlo.cdev, &fops);
        hlo.cdev.owner = THIS_MODULE;
        //printk("The addr of cdev is %x\n", (unsigned int)&hlo.cdev);

	/* Now we created a cdev, we have to add it to the kernel
	int cdev_add(struct cdev *dev, dev_t num, unsigned int count) */
	ret = cdev_add(&hlo.cdev, dev_num, 1);
	if(ret < 0) {
		printk("Helloworld : Unable to add cdev to kernel\n");
	}

	return 0;
}

static void polsel_exit(void) {

	cdev_del(&hlo.cdev);
	
	unregister_chrdev_region(dev_num, 1);
	printk("Helloworld : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(polsel_init);
module_exit(polsel_exit);