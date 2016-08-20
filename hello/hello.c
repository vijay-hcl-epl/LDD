// #includes
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>		//file operations
#include<linux/cdev.h>		//used to register the driver
#include<asm/uaccess.h>		//copy to user;copy from user

#define DEVICE_NAME	"hello"

// Device structure
struct hello
{
   int numopen;
   int numread;
   int numwrite;
   int numclose;
   char data[100];
   char pos;
   struct cdev cdev;
};

struct hello hlo = {0};
dev_t dev_num;
int maj_num;


static int hello_open(struct inode *inode, struct file *filp) {
        struct hello *phlo  =  NULL;

        phlo = container_of(inode->i_cdev, struct hello, cdev);
        filp->private_data = phlo;

        ++phlo->numopen;
        printk("Val of numopen is %d\n", phlo->numopen);
        
	return 0;
}

ssize_t hello_read(struct file *filp, char *buffer, size_t count, loff_t *f_pos) {
        struct hello *phlo  =  filp->private_data;

        ++phlo->numread;
        printk("Val of numread is %d\n", phlo->numread);
        printk("Val of fpos is %d\n", (int)filp->f_pos);
        printk("Val of count is %d\n", (int)count);
        
        copy_to_user(buffer, &phlo->data[filp->f_pos], count);
        *f_pos += count;

	return 0;
}

ssize_t hello_write(struct file *filp, const char *buffer, size_t count, loff_t *f_pos) {
        struct hello *phlo  = filp->private_data; 

        ++phlo->numwrite;
        printk("Val of numwrite is %d\n", phlo->numwrite);
        printk("Val of fpos is %d\n", (int)filp->f_pos);
        printk("Val of count is %d\n", (int)count);

        copy_from_user(&phlo->data[filp->f_pos], buffer, count);
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
struct file_operations fops = {
        .owner   = THIS_MODULE,
        .read    = hello_read,
        .write   = hello_write,
        .open    = hello_open,
        .release = hello_close
};

static int hello_init(void) {

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

static void hello_exit(void) {

	cdev_del(&hlo.cdev);
	
	unregister_chrdev_region(dev_num, 1);
	printk("Helloworld : Exit Module\n");
}


//Inform kernel where to start and stop with our module/driver
module_init(hello_init);
module_exit(hello_exit);


