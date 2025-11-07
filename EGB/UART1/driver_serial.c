#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/version.h>

#define DEVICE_NAME "pico_serial"
#define CLASS_NAME "pico"

static int major_number;
static struct class *pico_class = NULL;
static struct device *pico_device = NULL;
static struct cdev pico_cdev;

static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Pico Serial: Dispositivo abierto\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Pico Serial: Dispositivo cerrado\n");
    return 0;
}

static ssize_t device_write(struct file *file, const char __user *buffer, 
                           size_t length, loff_t *offset) {
    char *cmd_buffer;
    struct file *serial_file;
    ssize_t ret;
    loff_t pos = 0;
    
    cmd_buffer = kmalloc(length + 2, GFP_KERNEL);
    if (!cmd_buffer) return -ENOMEM;
    
    if (copy_from_user(cmd_buffer, buffer, length)) {
        kfree(cmd_buffer);
        return -EFAULT;
    }
    
    cmd_buffer[length] = '\n';
    cmd_buffer[length + 1] = '\0';
    
    printk(KERN_INFO "Pico Serial: Enviando: %s", cmd_buffer);
    
    // Usar ttyS0 que es el que existe en tu sistema
    serial_file = filp_open("/dev/ttyS0", O_WRONLY, 0);
    if (IS_ERR(serial_file)) {
        printk(KERN_ERR "Pico Serial: Error abriendo ttyS0: %ld\n", PTR_ERR(serial_file));
        kfree(cmd_buffer);
        return PTR_ERR(serial_file);
    }
    
    ret = kernel_write(serial_file, cmd_buffer, length + 1, &pos);
    filp_close(serial_file, NULL);
    kfree(cmd_buffer);
    
    if (ret < 0) {
        printk(KERN_ERR "Pico Serial: Error escribiendo: %zd\n", ret);
        return ret;
    }
    
    return length;
}

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .write = device_write,
};

static int __init pico_serial_init(void) {
    dev_t devno;
    
    printk(KERN_INFO "Pico Serial: Inicializando driver\n");
    
    // Registrar dispositivo de caracteres
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Pico Serial: Error registrando dispositivo\n");
        return major_number;
    }
    
    // Crear clase de dispositivo
    pico_class = class_create(CLASS_NAME);
    if (IS_ERR(pico_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Pico Serial: Error creando clase\n");
        return PTR_ERR(pico_class);
    }
    
    // Crear dispositivo
    devno = MKDEV(major_number, 0);
    pico_device = device_create(pico_class, NULL, devno, NULL, DEVICE_NAME);
    if (IS_ERR(pico_device)) {
        class_destroy(pico_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Pico Serial: Error creando dispositivo\n");
        return PTR_ERR(pico_device);
    }
    
    printk(KERN_INFO "Pico Serial: Driver cargado exitosamente (major: %d)\n", major_number);
    return 0;
}

static void __exit pico_serial_exit(void) {
    dev_t devno = MKDEV(major_number, 0);
    
    if (pico_device) {
        device_destroy(pico_class, devno);
    }
    if (pico_class) {
        class_destroy(pico_class);
    }
    if (major_number > 0) {
        unregister_chrdev(major_number, DEVICE_NAME);
    }
    
    printk(KERN_INFO "Pico Serial: Driver descargado\n");
}

module_init(pico_serial_init);
module_exit(pico_serial_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tu Nombre");
MODULE_DESCRIPTION("Driver para comunicación con Raspberry Pico");