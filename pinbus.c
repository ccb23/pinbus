
/* 
 * TODO:
 * start clock_irq only when device is open
 * global device pointer for free in module_exit
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/*------------------------------------------------------------------------------------------*\
\*------------------------------------------------------------------------------------------*/
#define dbg(level, format, arg...) do { if ( unlikely(pinbus_enable_dbg >= level) ) printk(KERN_WARNING ":[%s]: " format "\n", __FUNCTION__, ##arg); } while ( 0 )

#define NR_PINBUS_DEVS 1
#define END_OF_MSG_CHAR 0xFF

#define GPIO_BUS_PIN0 8
#define GPIO_BUS_PIN1 11
#define GPIO_BUS_PIN2 25
#define GPIO_BUS_PIN3 9
#define GPIO_BUS_PIN4 10
#define GPIO_BUS_PIN5 24
#define GPIO_BUS_PIN6 17
#define GPIO_BUS_PIN7 4 

unsigned int pinbus_busy_gpio   = 22;
unsigned int pinbus_clock_gpio  = 7; //currently not connected in our setup
unsigned int pinbus_status_gpio = 23;

unsigned int pinbus_wake_threshold        = 5;
unsigned int pinbus_kfifo_size            = 1024;
unsigned int pinbus_kfifo_timeout         = (HZ / 10);
unsigned int pinbus_bus_freq              = (HZ - 1);
unsigned int pinbus_use_clock_contraint   = 0; //pinbus_clock_gpio is not connected, so we disable clock_contraint
unsigned int pinbus_enable_dbg            = 0;
/*------------------------------------------------------------------------------------------*\
\*------------------------------------------------------------------------------------------*/

module_param(pinbus_wake_threshold,      uint, S_IRUGO | S_IWUSR );
module_param(pinbus_kfifo_size,          uint, S_IRUGO | S_IWUSR );
module_param(pinbus_kfifo_timeout,       uint, S_IRUGO | S_IWUSR );
module_param(pinbus_busy_gpio,           uint, S_IRUGO | S_IWUSR );
module_param(pinbus_clock_gpio,          uint, S_IRUGO | S_IWUSR );
module_param(pinbus_status_gpio,         uint, S_IRUGO | S_IWUSR );

module_param(pinbus_use_clock_contraint, uint, S_IRUGO | S_IWUSR );
module_param(pinbus_enable_dbg,          uint, S_IRUGO | S_IWUSR );

static int pinbus_major;
static int busy_irq_number;
static int stat_irq_number;

struct pinbus_dev {
    struct kfifo fifo;
    wait_queue_head_t wait_for_data;
    atomic_t device_available;
    struct cdev cdev;
};

static struct pinbus_dev *g_pinbus_dev = NULL; 

/*------------------------------------------------------------------------------------------*\
\*------------------------------------------------------------------------------------------*/

int pinbus_open(struct inode *inode, struct file *filp) {
    struct pinbus_dev *pin_dev;
    pin_dev = container_of(inode->i_cdev, struct pinbus_dev, cdev);

    if ( !atomic_dec_and_test(&pin_dev->device_available)){
        atomic_inc(&pin_dev->device_available);
        return -EBUSY;
    }

    filp->private_data = pin_dev;

    return 0;
}

int pinbus_release(struct inode *inode, struct file *filp) {
    struct pinbus_dev *pin_dev;
    pin_dev = filp->private_data;
    atomic_inc(&pin_dev->device_available);
    return 0;
}


ssize_t pinbus_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    struct pinbus_dev *pin_dev = filp->private_data;
    ssize_t retval = 0;
    ssize_t bytes_to_copy = 0;

    if (*f_pos) {
        printk(KERN_ERR "fpos = %llu", *f_pos);
        return -EINVAL;
    }

    while(kfifo_is_empty(&pin_dev->fifo)) {
        int reason = wait_event_interruptible_timeout( pin_dev->wait_for_data, ! kfifo_is_empty(&pin_dev->fifo), pinbus_kfifo_timeout );
        if (reason < 0)
            return reason;
        if (reason > 0)
            break;
    }

    bytes_to_copy = min(count, kfifo_len(&pin_dev->fifo));
    if ( kfifo_to_user(&pin_dev->fifo, buf, bytes_to_copy, &retval) != 0)
        retval = -EINVAL;

    return retval;

}

loff_t pinbus_llseek(struct file *filp, loff_t off, int whence) {
    return -ESPIPE; /* unseekable */
}

struct file_operations pinbus_fops = {
    .owner =     THIS_MODULE,
    .llseek =    pinbus_llseek,
    .read =      pinbus_read,
    .open =      pinbus_open,
    .release =   pinbus_release,
};

/*------------------------------------------------------------------------------------------*\
\*------------------------------------------------------------------------------------------*/

static irqreturn_t pinbus_busy_interrupt(int irq, void* data) {
    unsigned char bus_char = 0;
    struct pinbus_dev *pin_dev = (struct pinbus_dev *)data;

    //Clock Constraint Check

    if ( !pinbus_use_clock_contraint || gpio_get_value( pinbus_clock_gpio ) ){
        bus_char =  ( gpio_get_value( GPIO_BUS_PIN0 ) << 0) |
            ( gpio_get_value( GPIO_BUS_PIN1 ) << 1) |
            ( gpio_get_value( GPIO_BUS_PIN2 ) << 2) |
            ( gpio_get_value( GPIO_BUS_PIN3 ) << 3) |
            ( gpio_get_value( GPIO_BUS_PIN4 ) << 4) |
            ( gpio_get_value( GPIO_BUS_PIN5 ) << 5) |
            ( gpio_get_value( GPIO_BUS_PIN6 ) << 6) |
            ( gpio_get_value( GPIO_BUS_PIN7 ) << 7) ;
        kfifo_in(&pin_dev->fifo,&bus_char, 1);
        dbg(1, "gpio busdata event on irq=%d, data=%c", irq, bus_char );

    } else {
        dbg(1, "clock is down, ignore irq=%d", irq);
    }

    if ( kfifo_len(&pin_dev->fifo) >= pinbus_wake_threshold ){
        wake_up_interruptible(&pin_dev->wait_for_data);
        dbg(1, "Wake Reader" );
    }

    return IRQ_HANDLED;
}

static irqreturn_t pinbus_stat_interrupt(int irq, void* data) {
    struct pinbus_dev *pin_dev = (struct pinbus_dev *)data;
    unsigned char bus_char = END_OF_MSG_CHAR;
    dbg(1, "gpio event stat on irq=%d", irq );
    kfifo_in(&pin_dev->fifo,&bus_char, 1);
    if ( kfifo_len(&pin_dev->fifo) >= pinbus_wake_threshold ){
        wake_up_interruptible(&pin_dev->wait_for_data);
        dbg(1, KERN_ERR "Wake Reader" );
    }
    return IRQ_HANDLED;
}

/*------------------------------------------------------------------------------------------*\
\*------------------------------------------------------------------------------------------*/


static int pinbus_init(void){
    dev_t dev = 0;
    int devno;
    int result;

    printk(KERN_INFO "Loading Pinbus Module\n");

    result = alloc_chrdev_region(&dev, 0, NR_PINBUS_DEVS, "pinbus");
    pinbus_major = MAJOR(dev);
    if (result < 0)
        return result;

    g_pinbus_dev = kmalloc(sizeof (struct pinbus_dev), GFP_KERNEL);
    if (!g_pinbus_dev) {
        unregister_chrdev_region(dev, NR_PINBUS_DEVS);
        return -ENOMEM;
    }

    devno = MKDEV(pinbus_major, 0);
    cdev_init(&g_pinbus_dev->cdev, &pinbus_fops);
    g_pinbus_dev->cdev.owner = THIS_MODULE;
    g_pinbus_dev->cdev.ops = &pinbus_fops;
    atomic_set(&g_pinbus_dev->device_available, 1);
    init_waitqueue_head(&g_pinbus_dev->wait_for_data);

    if (kfifo_alloc(&g_pinbus_dev->fifo, pinbus_kfifo_size, GFP_KERNEL)){
        unregister_chrdev_region(dev, NR_PINBUS_DEVS);
        kfree(g_pinbus_dev);
        return -ENOMEM;
    }

    result = cdev_add (&g_pinbus_dev->cdev, devno, 1);
    if (result)
        printk(KERN_NOTICE "Error %d adding pinbus\n", result);

    busy_irq_number = gpio_to_irq(pinbus_busy_gpio);
    stat_irq_number = gpio_to_irq(pinbus_status_gpio);

    if ( request_irq(busy_irq_number, pinbus_busy_interrupt, IRQF_TRIGGER_FALLING|IRQF_ONESHOT, "gpiof_busy_pinbus", g_pinbus_dev) ) {
        printk(KERN_ERR "GPIO: trouble requesting IRQ %d\n", busy_irq_number);
        return(-EIO);
    } else {
        printk(KERN_INFO "GPIO: requesting IRQ %d-> fine\n", busy_irq_number);
    }

    if ( request_irq(stat_irq_number, pinbus_stat_interrupt, IRQF_TRIGGER_FALLING|IRQF_ONESHOT, "gpiof_stat_pinbus", g_pinbus_dev) ) {
        printk(KERN_ERR "GPIO: trouble requesting IRQ %d\n",stat_irq_number);
        free_irq(busy_irq_number, g_pinbus_dev);
        return(-EIO);
    } else {
        printk(KERN_INFO "GPIO: requesting IRQ %d-> fine\n", stat_irq_number);
    }

    printk(KERN_INFO "Loading Pinbus Module successful\n");
    return 0;

}

static void pinbus_exit(void) {

    printk(KERN_INFO "Shutting down Pinbus Module\n");
    free_irq(busy_irq_number, g_pinbus_dev);
    free_irq(stat_irq_number, g_pinbus_dev);
    cdev_del(&g_pinbus_dev->cdev);
    kfree(g_pinbus_dev);
    unregister_chrdev_region(MKDEV (pinbus_major, 0), NR_PINBUS_DEVS);
    printk(KERN_INFO "Shutting down Pinbus Module successful\n");

}

module_init(pinbus_init);
module_exit(pinbus_exit);
MODULE_LICENSE("GPL");
