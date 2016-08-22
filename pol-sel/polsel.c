// #includes
#include<linux/module.h>        // Is it needed?
#include<linux/kernel.h>        // Is it needed?
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
#include <linux/poll.h>         // To implement poll

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

static int polsel_open(struct inode *inode, struct file *filp) {
	struct polsel *phlo  =  NULL;

	// Below two statements are needed in almost all char drivers

	// Get the device pointer from inode struct
	phlo = container_of(inode->i_cdev, struct polsel, cdev);

	// Store the device pointer in filp->private_data so that this device pointer could be used
	// in all functions of file operations 
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

	// wrpos > rdpos means new data is avaialble for read
	if(phlo->wrpos > phlo->rdpos)
	{
		// If available data is less than the requested data
		// copy only available data to the user
		if((phlo->wrpos - phlo->rdpos) < count)
			count = phlo->wrpos - phlo->rdpos;

		// Remember filp are specific to process that opens this
		// so filp->f_pos stores the last accessed index`
		copy_to_user(buffer, &phlo->data[filp->f_pos], count);

		// Always increment the *f_pos so that kernel remembers it
		// read and write should update the position using *f_pos
		// Kernel later uses this value and updates the value filp->f_pos
		*f_pos += count;

		// Increment the device specific read ptr
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

	// Remember filp are specific to process that opens this
	// so filp->f_pos stores the last accessed index
	copy_from_user(&phlo->data[filp->f_pos], buffer, count);

	// Always increment the *f_pos so that kernel remembers it
	// read and write should update the position using *f_pos
	// Kernel later uses this value and updates the value filp->f_pos
	*f_pos += count;

	// Increment the device specific write ptr
	phlo->wrpos += count;

	return 0;
}

int polsel_close(struct inode *inode, struct file *filp) {
	struct polsel *phlo  =  NULL;

	phlo = container_of(inode->i_cdev, struct polsel, cdev);

	++phlo->numclose;
	printk("polsel driver close\n");
	return 0;
}

static unsigned int polsol_poll(struct file *filp, poll_table *wait)
{
	struct polsel *phlo  = filp->private_data;
	unsigned int mask = 0;

	// Wait for the specified time from user app
	// Giving infinite time hangs forever so give limited time
	poll_wait(filp, &wq, wait);   

	if(phlo->wrpos > phlo->rdpos)
	{
		mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

// File Operations struct
struct file_operations fops = {
	.owner   = THIS_MODULE,
	.read    = polsel_read,
	.write   = polsel_write,
	.open    = polsel_open,
	.release = polsel_close,
	.poll    = polsol_poll
};

static int polsel_init(void) {

	int ret = -1;

	// allocate a major number dynamically
	// Use register_chrdev_region if you know the device numbers you want	
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("polsel : major number is %d\n",maj_num);
	printk("To create mknod for device file, execute 'mknod /dev/polsel c %d 0'\n", maj_num);

	//pcdev = cdev_alloc();   // Use it if you do not want to have your own device struct

	// Initialise the cdev field inside your structure
	cdev_init(&hlo.cdev, &fops);
	hlo.cdev.owner = THIS_MODULE;

	/* Now we created a cdev, we have to add it to the kernel
	   int cdev_add(struct cdev *dev, dev_t num, unsigned int count) */
	// You should not call cdev_add until your driver is completely 
	// ready to handle operations on the device
	ret = cdev_add(&hlo.cdev, dev_num, 1);
	if(ret < 0) {
		printk("Helloworld : Unable to add cdev to kernel\n");
	}

	return 0;
}

static void polsel_exit(void) {
	// To remove a char device from the system
	cdev_del(&hlo.cdev);

	// Do this after doing cdev_del
	unregister_chrdev_region(dev_num, 1);
	printk("Helloworld : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(polsel_init);
module_exit(polsel_exit);
