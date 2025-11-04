/*Este ejemplo simula el uso de un pin GPIO para encender o pagar un led coenctado en el pin.*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define AUTHOR "TD3"
#define CHRDEV_MINOR 50
#define CHRDEV_COUNT 1

struct chrdev_data {
    int led_state;
    struct cdev cdev;
};

dev_t chrdev_number;
struct class *chrdev_class;
struct chrdev_data *chrdev_data;
/*En esta parte se crea el archivo que actuara como interfaz entre el modulo y el usuario*/
static int chrdev_open(struct inode *inode, struct file *file)
{
    struct chrdev_data *data = container_of(inode->i_cdev, struct chrdev_data, cdev);
    file->private_data = data;
    printk(KERN_INFO "%s: AD_3 abierto\n", AUTHOR);
    return 0;
}

static int chrdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "%s: AD_3 cerrado\n", AUTHOR);
    return 0;
}

static ssize_t chrdev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    struct chrdev_data *data = file->private_data;
    char buffer[32];
    int len;
    
    if (*ppos > 0)
        return 0;

    len = snprintf(buffer, sizeof(buffer), "STATE:%d\n", data->led_state);

    if (count < len)
        len = count;

    if (copy_to_user(user_buf, buffer, len)) {
        return -EFAULT;
    }

    *ppos += len;
    return len;
}

static ssize_t chrdev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    struct chrdev_data *data = file->private_data;
    char cmd;
    ssize_t bytes_processed = 0;
    
    printk(KERN_INFO "%s: chrdev_write llamado: count=%zu\n", AUTHOR, count);
    
    if (count == 0) {
        return 0;
    }

    // Procesar solo el primer carácter válido
    while (bytes_processed < count) {
        if (copy_from_user(&cmd, user_buf + bytes_processed, 1)) {
            return -EFAULT;
        }
        
        printk(KERN_INFO "%s: Caracter recibido: '%c' (0x%02x)\n", AUTHOR, cmd, cmd);

        if (cmd == '1') {
            data->led_state = 1;
            printk(KERN_INFO "%s: LED establecido a 1\n", AUTHOR);
            bytes_processed++;
        } 
        else if (cmd == '0') {
            data->led_state = 0;
            printk(KERN_INFO "%s: LED establecido a 0\n", AUTHOR);
            bytes_processed++;
        }
        else if (cmd == '\n' || cmd == ' ' || cmd == '\t') {
            // Ignorar espacios y nueva línea - continuar procesando
            bytes_processed++;
            continue;
        }
        else {
            printk(KERN_WARNING "%s: Comando invalido: '%c' (0x%02x)\n", AUTHOR, cmd, cmd);
            // Si encontramos un carácter inválido, retornar error
            return -EINVAL;
        }
        
        // Solo procesamos un comando válido por escritura
        break;
    }
    
    return bytes_processed ? bytes_processed : -EINVAL;
}

static struct file_operations chrdev_ops = {
    .owner = THIS_MODULE,
    .open = chrdev_open,
    .release = chrdev_release,
    .read = chrdev_read,
    .write = chrdev_write,
};
/*===============================================================================================================================*/
static int __init chrdev_init(void)
{
    int ret;

    printk(KERN_INFO "%s: Iniciando AD_3\n", AUTHOR);

    chrdev_data = kzalloc(sizeof(struct chrdev_data), GFP_KERNEL);
    if (!chrdev_data) {
        printk(KERN_ERR "%s: Error de memoria\n", AUTHOR);
        return -ENOMEM;
    }

    chrdev_data->led_state = 0;

    ret = alloc_chrdev_region(&chrdev_number, CHRDEV_MINOR, CHRDEV_COUNT, "AD_3");
    if (ret < 0) {
        printk(KERN_ERR "%s: Error registrando dispositivo\n", AUTHOR);
        kfree(chrdev_data);
        return ret;
    }

    cdev_init(&chrdev_data->cdev, &chrdev_ops);
    chrdev_data->cdev.owner = THIS_MODULE;

    ret = cdev_add(&chrdev_data->cdev, chrdev_number, CHRDEV_COUNT);
    if (ret < 0) {
        printk(KERN_ERR "%s: Error agregando cdev\n", AUTHOR);
        unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
        kfree(chrdev_data);
        return ret;
    }

    chrdev_class = class_create("AD_3_class");
    if (IS_ERR(chrdev_class)) {
        printk(KERN_ERR "%s: Error creando clase\n", AUTHOR);
        cdev_del(&chrdev_data->cdev);
        unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
        kfree(chrdev_data);
        return PTR_ERR(chrdev_class);
    }

    device_create(chrdev_class, NULL, chrdev_number, NULL, "AD_3");

    printk(KERN_INFO "%s: AD_3 creado exitosamente\n", AUTHOR);
    printk(KERN_INFO "%s: Comandos: echo 1 > /dev/AD_3 (ON), echo 0 > /dev/AD_3 (OFF)\n", AUTHOR);
    return 0;
}

static void __exit chrdev_exit(void)
{
    device_destroy(chrdev_class, chrdev_number);
    class_destroy(chrdev_class);
    cdev_del(&chrdev_data->cdev);
    unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
    kfree(chrdev_data);
    
    printk(KERN_INFO "%s: AD_3 removido\n", AUTHOR);
}

module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Controlador AD_3 - Version final");