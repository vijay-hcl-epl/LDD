// #includes
#include<linux/module.h>        // Is it needed?
#include<linux/device.h>
#include<linux/kernel.h>        // Is it needed?
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user
#include<linux/workqueue.h>          // Work Queue
#include<linux/aio.h>           // asynchronous io
#include<linux/slab.h>             // kmalloc
#include<linux/delay.h>         // msleep

#define DEVICE_NAME	"Taskasync"

// Device structure
struct Taskasync
{
	int numopen;
	int numread;
	int numwrite;
	int numclose;
	char data[100];
	struct cdev cdev;
};

struct Taskasync hlo = {0};
dev_t dev_num;
int maj_num;

static int Taskasync_open(struct inode *inode, struct file *filp) {
	struct Taskasync *phlo  =  NULL;

	// Below two statements are needed in almost all char drivers

	// Get the device pointer from inode struct
	phlo = container_of(inode->i_cdev, struct Taskasync, cdev);

	// Store the device pointer in filp->private_data so that this device pointer could be used
	// in all functions of file operations 
	filp->private_data = phlo;

	++phlo->numopen;
	printk("Taskasync driver opened\n");

	return 0;
}

ssize_t Taskasync_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
	struct Taskasync *phlo  =  filp->private_data;

	++phlo->numread;
	printk("Taskasync driver read\n");

	// Remember filp are specific to process that opens this
	// so filp->f_pos stores the last accessed index
	copy_to_user(buffer, &phlo->data[filp->f_pos], count);

	// Always increment the *f_pos so that kernel remembers it
	// read and write should update the position using *f_pos
	// Kernel later uses this value and updates the value filp->f_pos
	*f_pos += count;

	return 0;
}

ssize_t Taskasync_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
	struct Taskasync *phlo  = filp->private_data; 

	++phlo->numwrite;
	printk("Taskasync driver write\n");

	copy_from_user(&phlo->data[filp->f_pos], buffer, count);
	*f_pos += count;

	return 0;
}

int Taskasync_close(struct inode *inode, struct file *filp) {
	struct Taskasync *phlo  =  NULL;

	phlo = container_of(inode->i_cdev, struct Taskasync, cdev);

	++phlo->numclose;
	printk("Taskasync driver close\n");
	return 0;
}

struct async_work {
	struct kiocb *iocb;
	int result;
	struct work_struct work;
};

static void DoTask(struct work_struct *p)
{
	struct async_work *stuff = container_of(p, struct async_work, work);
        printk("In DoTask..\n");
	msleep(10000);
	printk("After Sleep in DeferTask..\n");
	aio_complete(stuff->iocb, stuff->result, 0);
	kfree(stuff);
}

//static int DeferTask(struct kiocb *iocb, const struct iovec * iv,unsigned long count, loff_t offset)
static int DeferTask(struct kiocb *iocb, char *buffer,size_t count, loff_t offset)
{
	struct async_work *stuff;
	int result;

        printk("In DeferTask..\n");
	/* If this is a synchronous IOCB, we return our status now. */
	if (is_sync_kiocb(iocb))
		return result;

        printk("Aftr is_sync_kiocb..\n");

	/* Otherwise defer the completion for a few milliseconds. */
	stuff = kmalloc (sizeof (*stuff), GFP_KERNEL);
	if (stuff == NULL)
		return result; /* No memory, just complete now */
	stuff->iocb = iocb;
	stuff->result = result;
	INIT_WORK(&stuff->work, DoTask);
	schedule_work(&stuff->work);
	return -EIOCBQUEUED;
}

//ssize_t Taskasync_aio_read (struct kiocb *iocb, const struct iovec * iv,unsigned long count, loff_t offset)
ssize_t Taskasync_aio_read (struct kiocb *iocb, char *buffer,size_t count, loff_t offset)
{
        printk("In Taskasync_aio_read..\n");
	//return DeferTask(iocb, iv , count, offset);
	return DeferTask(iocb, buffer, count, offset);
}

// File Operations struct
// Declare after fops function definitions
struct file_operations fops = {
	.owner   = THIS_MODULE,
	.read    = Taskasync_read,
	.write   = Taskasync_write,
	.open    = Taskasync_open,
	.release = Taskasync_close,
	.aio_read = Taskasync_aio_read,
};

static int Taskasync_init(void) {

	int ret = -1;

	// allocate a major number dynamically
	// Use register_chrdev_region if you know the device numbers you want
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret < 0) {
		printk("Hello : Failed to allocate major number\n");
		return ret;
	}
	maj_num = MAJOR(dev_num);

	printk("Taskasync : major number is %d\n",maj_num);
	printk("To create mknod for device file, execute 'mknod /dev/Taskasync c %d 0'\n", maj_num);

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

static void Taskasync_exit(void) {

	// To remove a char device from the system
	cdev_del(&hlo.cdev);

	// Do this after doing cdev_del
	unregister_chrdev_region(dev_num, 1);
	printk("Taskasync : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(Taskasync_init);
module_exit(Taskasync_exit);


