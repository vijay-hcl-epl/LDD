// #includes
#include<linux/module.h>        // Is it needed?
#include<linux/kernel.h>        // Is it needed?
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user

#define DEVICE_NAME	"hello"

// Device structure
// Always form your own device structure and embedd cdev
// This is an example of inheritance in C language where
// cdev is analogous to base class and hello is analogous to derived class
struct hello
{
	// Below device variable just to track the no. of times
	// this device is opened,read,write and close
	int numopen;
	int numread;
	int numwrite;
	int numclose;
	char data[100]; //To store the data
	struct cdev cdev;
};

struct hello hlo = {0};
dev_t dev_num;
int maj_num;


static int hello_open(struct inode *inode, struct file *filp) {
	struct hello *phlo  =  NULL;

	// Below two statements are needed in almost all char drivers

	// Get the device pointer from inode struct
	phlo = container_of(inode->i_cdev, struct hello, cdev);

	// Store the device pointer in filp->private_data so that this device pointer could be used
	// in all functions of file operations 
	filp->private_data = phlo;

	++phlo->numopen;
	printk("Val of numopen is %d\n", phlo->numopen);

	return 0;
}

ssize_t hello_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
	struct hello *phlo  =  filp->private_data;

	++phlo->numread;
	printk("Val of numread is %d\n", phlo->numread);

	// The driver can read the value filp->f_pos but should not normally change it
	printk("Val of read fpos is %d\n", (int)filp->f_pos);

	// Remember filp are specific to process that opens this
	// so filp->f_pos stores the last accessed index
	copy_to_user(buffer, &phlo->data[filp->f_pos], count);

	// Always increment the *f_pos so that kernel remembers it
	// read and write should update the position using *f_pos
	// Kernel later uses this value and updates the value filp->f_pos
	*f_pos += count;

	return 0;
}

ssize_t hello_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
	struct hello *phlo  = filp->private_data; 

	++phlo->numwrite;
	printk("Val of numwrite is %d\n", phlo->numwrite);

	// The driver can read the value filp->f_pos but should not normally change it
	printk("Val of write fpos is %d\n", (int)filp->f_pos);

	// Remember filp are specific to process that opens this
	// so filp->f_pos stores the last accessed index
	copy_from_user(&phlo->data[filp->f_pos], buffer, count);

	// Always increment the *f_pos so that kernel remembers it
	// read and write should update the position using *f_pos
	// Kernel later uses this value and updates the value filp->f_pos
	*f_pos += count;

	return 0;
}

int hello_close(struct inode *inode, struct file *filp) {
	struct hello *phlo  =  NULL;

	phlo = container_of(inode->i_cdev, struct hello, cdev);

	++phlo->numclose;
	printk("Val of numclose is %d\n", phlo->numclose);
	return 0;
}

// File Operations struct
// Declare after fops function definitions
struct file_operations fops = {
	.owner   = THIS_MODULE,
	.read    = hello_read,
	.write   = hello_write,
	.open    = hello_open,
	.release = hello_close
};

static int hello_init(void) {

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
	printk("To create mknod for device file, execute 'mknod /dev/hello c %d 0'\n", maj_num);

	//pcdev = cdev_alloc(); // Use it if you do not want to have your own device struct

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

static void hello_exit(void) {

	// To remove a char device from the system
	cdev_del(&hlo.cdev);

	// Do this after doing cdev_del
	unregister_chrdev_region(dev_num, 1);
	printk("hello : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(hello_init);
module_exit(hello_exit);


