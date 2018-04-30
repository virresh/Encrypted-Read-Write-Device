#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>    /* for copy_to_user */

/*  
 *  Function Prototypes
 */
static int __init decdriver_init(void);
static void __exit decdriver_cleanup(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define DEVICE_NAME "decdev"    /* Device name as it appears in /proc/devices and /dev/  */
#define BUF_LEN 16             /* Length of blocks in which the device works */

static unsigned char device_buffer[BUF_LEN+1];
static int num_opens = 0;
static unsigned char shared_key[BUF_LEN+1];
static int deviceStatus = 0; // 0-> first write pending, 1-> initialised
static unsigned char buffer[BUF_LEN+1];
/*
 * Register functions for VFS to call
 */
static struct file_operations vfs_dev_fops =
{
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write
};

static struct miscdevice regInfo = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = DEVICE_NAME,
    .fops   = &vfs_dev_fops,
    .mode   = 0666
};

static void memFill(char *buf, int len, char fill)
{
    int i;
    for(i=0; i<len; i++)
    {
        buf[i] = fill;
    }
}

int stlen(char str[])
{
    int i=0;
    while(str[i]!='\0')
    {
        i++;
    }
    return i;
}

static int device_open(struct inode *i, struct file *f)
{
    if(num_opens == 0)
    {
        num_opens = 1;
    }
    else
    {
        printk("decdev: Device already open, cannot open again !!!");
        return -EBUSY;
    }
    printk(KERN_INFO "decdev: device opened\n");
    deviceStatus = 0;
    return 0;
}

static ssize_t device_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    int err_no = 0;
    printk(KERN_INFO "decdev: device read\n");
    if(len < BUF_LEN)
    {
        return -EFAULT;
    }

    err_no = copy_to_user(buf, device_buffer, BUF_LEN);
    if(err_no == 0)
    {
        printk("decdev: Send decoded block to user.\n");
        return BUF_LEN;
    }
    else
    {
        printk("decdev: Failed to write decoded block to user space.\n");
        return -EFAULT;
    } 
}

static ssize_t device_write(struct file *f, const char __user *buf, size_t lenPar, loff_t *off)
{
    size_t len = 0;
    int tLV = 0;
    printk(KERN_INFO "decdev: decdev write(%zu)\n", lenPar);

    if(lenPar > BUF_LEN)
    {
        printk(KERN_INFO "decdev: invalid length %zu, buf len: %d", lenPar, BUF_LEN);
        lenPar = BUF_LEN;
        // return -EINVAL;
    }


    if (copy_from_user(buffer, buf, lenPar) != 0)
    {
        return -EFAULT;
    }

    for(tLV = lenPar; tLV <BUF_LEN; tLV++)
    {
        buffer[tLV] = ' ';
    }

    // printk(KERN_INFO "<<<  %s  >>>", buffer);

    if(deviceStatus == 0)
    {
        int i;
        for(i=0; i<lenPar && i<BUF_LEN; i++)
        {
            shared_key[i] = (unsigned char)buffer[i] & 0xFF;
        }
        while(i<BUF_LEN)
        {
            shared_key[i] = 'v';
            i++;
        }
        printk(KERN_INFO "decdev: First write. Initiales PSK (Pre-Shared Key)\n");
        printk(KERN_INFO "decdev: PSK: <%s>", shared_key);
        len = BUF_LEN;
        deviceStatus = 1;
    }
    else if(buffer[0] != '\0')
    {
        int i;
        for(i=0; i<BUF_LEN; i++)
        {
            device_buffer[i] = shared_key[i] ^ buffer[i];
            shared_key[i] = buffer[i];
        }
        printk(KERN_INFO "decdev: Block Written and decoded.\n");
        len = i;
    }
    else
    {
        memFill(device_buffer,BUF_LEN,0);
        memFill(shared_key,BUF_LEN,0);
        printk(KERN_INFO "decdev: EOF encountered. File operation complete. ");
        len = 0;
        deviceStatus = 0;
    }
    return len;
}

static int device_release(struct inode *i, struct file *f)
{
    printk(KERN_INFO "decdev: Device close()\n");
    num_opens--;
    deviceStatus = 0;
    memFill(device_buffer,BUF_LEN,0);
    memFill(shared_key,BUF_LEN,0);
    return 0;
}

static int __init decdriver_init(void)
{
    int ret;
    ret = misc_register(&regInfo);
    if(ret)
    {
        printk(KERN_ERR "Unable to register 'decdev' device !!!\n");
    }
    else
    {
        printk(KERN_INFO "decdev: decdev device created, driver registered\n");
        memFill(device_buffer, BUF_LEN, '\0');
        memFill(shared_key, BUF_LEN, '\0');
        printk(KERN_INFO "decdev: initialised buffers and PSK to \\0\n");
    }
    return ret;
}

static void __exit decdriver_cleanup(void)
{
    misc_deregister(&regInfo);
    printk(KERN_INFO "decdev: decdev device destroyed, driver deregistered\n");
}

module_init(decdriver_init);
module_exit(decdriver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Viresh Gupta <viresh16118@iiitd.ac.in>");
MODULE_DESCRIPTION("Driver for decdev, to write files in encrypted format.");