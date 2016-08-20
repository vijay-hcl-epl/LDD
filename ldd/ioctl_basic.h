#include <linux/ioctl.h>
#define IOC_MAGIC 'v' // defines the magic number

#define IOCTL_ADDBUS _IO(IOC_MAGIC,0) // defines our ioctl call
#define IOCTL_REMBUS _IO(IOC_MAGIC,1) // defines our ioctl call
