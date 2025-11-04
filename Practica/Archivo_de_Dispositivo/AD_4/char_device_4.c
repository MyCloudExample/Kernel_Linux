/*Este ejemplo es undriver que enciende un led cuando se ingresa el valor 1 y lo apaga cuando se ingresa el valor 0. El kernel de Linux
* no reconoce los pines de las misma forma que  al Raspberry Pi 3, en su lugar tiene su propia nomenclatura, se utilizo el pin 
* GPIO 2 sinedo en el kernel identificado como fisico 514*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define AUTHOR "TD3"
#define CHRDEV_MINOR 51
#define CHRDEV_COUNT 1

// CORRECCIÓN: Usar la numeración REAL del sistema (vista en debug/gpio)
// Los GPIOs en debug/gpio se muestran como gpio-512 = GPIO0 físico
#define GPIO_1 (2 + 512)    // gpio-514 = GPIO2 físico
#define GPIO_2 (3 + 512)    // gpio-515 = GPIO3 físico  
#define GPIO_3 (4 + 512)    // gpio-516 = GPIO4 físico
#define GPIO_4 (5 + 512)    // gpio-517 = GPIO5 físico
#define GPIO_5 (6 + 512)    // gpio-518 = GPIO6 físico
#define GPIO_6 (7 + 512)    // gpio-519 = GPIO7 físico

struct chrdev_data {
    int led_state;
    int led_gpio;
    struct cdev cdev;
};

dev_t chrdev_number;
struct class *chrdev_class;
struct chrdev_data *chrdev_data;

static int chrdev_open(struct inode *inode, struct file *file)
{
    struct chrdev_data *data = container_of(inode->i_cdev, struct chrdev_data, cdev);
    file->private_data = data;
    printk(KERN_INFO "%s: AD_4 abierto\n", AUTHOR);
    return 0;
}

static int chrdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "%s: AD_4 cerrado\n", AUTHOR);
    return 0;
}

static ssize_t chrdev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    struct chrdev_data *data = file->private_data;
    char buffer[64];
    int len;
    int current_state;
    
    if (*ppos > 0)
        return 0;

    current_state = gpio_get_value(data->led_gpio);
    
    len = snprintf(buffer, sizeof(buffer), 
                  "GPIO %d (fisico: %d): %s\nEstado: %d\n",
                  data->led_gpio - 512,  // Mostrar número físico
                  data->led_gpio,
                  current_state ? "ENCENDIDO" : "APAGADO",
                  current_state);

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
    char buffer[2];
    unsigned long val;
    int ret;
    
    if (count == 0)
        return 0;
        
    if (count > 2)
        return -EINVAL;
        
    if (copy_from_user(buffer, user_buf, count))
        return -EFAULT;
        
    buffer[count-1] = '\0';
    
    ret = kstrtoul(buffer, 10, &val);
    if (ret)
        return ret;
        
    if (val != 0 && val != 1)
        return -EINVAL;
        
    gpio_set_value(data->led_gpio, val);
    data->led_state = val;
    
    printk(KERN_INFO "%s: LED %s - GPIO %d (fisico: %d)\n", AUTHOR, 
           val ? "ENCENDIDO" : "APAGADO", data->led_gpio, data->led_gpio - 512);
    
    return count;
}

static struct file_operations chrdev_ops = {
    .owner = THIS_MODULE,
    .open = chrdev_open,
    .release = chrdev_release,
    .read = chrdev_read,
    .write = chrdev_write,
};

static int try_setup_gpio(int gpio)
{
    int ret;
    
    printk(KERN_INFO "%s: Probando GPIO %d (fisico: %d)...\n", 
           AUTHOR, gpio, gpio - 512);
    
    if (!gpio_is_valid(gpio)) {
        printk(KERN_ERR "%s: GPIO %d no es válido\n", AUTHOR, gpio);
        return -EINVAL;
    }
    
    ret = gpio_request(gpio, "AD_4_LED");
    if (ret) {
        printk(KERN_WARNING "%s: GPIO %d (fisico: %d) ocupado (error %d)\n", 
               AUTHOR, gpio, gpio - 512, ret);
        return ret;
    }
    
    ret = gpio_direction_output(gpio, 0);
    if (ret) {
        printk(KERN_ERR "%s: Error configurando GPIO %d\n", AUTHOR, gpio);
        gpio_free(gpio);
        return ret;
    }
    
    printk(KERN_INFO "%s: ✓ GPIO %d (fisico: %d) configurado exitosamente\n", 
           AUTHOR, gpio, gpio - 512);
    return 0;
}

static int setup_gpio(int *gpio_used)
{
    int gpios[] = {GPIO_1, GPIO_2, GPIO_3, GPIO_4, GPIO_5, GPIO_6};
    int num_gpios = sizeof(gpios) / sizeof(gpios[0]);
    int i, ret;
    
    printk(KERN_INFO "%s: === BUSCANDO GPIO DISPONIBLE ===\n", AUTHOR);
    
    for (i = 0; i < num_gpios; i++) {
        ret = try_setup_gpio(gpios[i]);
        if (ret == 0) {
            *gpio_used = gpios[i];
            printk(KERN_INFO "%s: ✅ USANDO GPIO %d (fisico: %d)\n", 
                   AUTHOR, gpios[i], gpios[i] - 512);
            return 0;
        }
    }
    
    printk(KERN_ERR "%s: ❌ TODOS LOS GPIOs ESTÁN OCUPADOS\n", AUTHOR);
    return -ENODEV;
}

static void cleanup_gpio(int gpio)
{
    if (gpio_is_valid(gpio)) {
        gpio_set_value(gpio, 0);
        gpio_free(gpio);
        printk(KERN_INFO "%s: GPIO %d (fisico: %d) liberado\n", 
               AUTHOR, gpio, gpio - 512);
    }
}
/*===========================FUNCION QUE EJECUTA EL CODIGO PRINCIPAL DEL MODULO==================================================*/
static int __init chrdev_init(void)
{
    int ret;
    int gpio_used = -1;

    printk(KERN_INFO "%s: ===== INICIANDO AD_4 CON GPIO REAL =====\n", AUTHOR);

    chrdev_data = kzalloc(sizeof(struct chrdev_data), GFP_KERNEL);
    if (!chrdev_data) {
        printk(KERN_ERR "%s: Error de memoria\n", AUTHOR);
        return -ENOMEM;
    }

    ret = setup_gpio(&gpio_used);
    if (ret) {
        kfree(chrdev_data);
        return ret;
    }

    chrdev_data->led_gpio = gpio_used;
    chrdev_data->led_state = 0;

    ret = alloc_chrdev_region(&chrdev_number, CHRDEV_MINOR, CHRDEV_COUNT, "AD_4");
    if (ret < 0) {
        printk(KERN_ERR "%s: Error registrando dispositivo\n", AUTHOR);
        cleanup_gpio(gpio_used);
        kfree(chrdev_data);
        return ret;
    }

    cdev_init(&chrdev_data->cdev, &chrdev_ops);
    chrdev_data->cdev.owner = THIS_MODULE;

    ret = cdev_add(&chrdev_data->cdev, chrdev_number, CHRDEV_COUNT);
    if (ret < 0) {
        printk(KERN_ERR "%s: Error agregando cdev\n", AUTHOR);
        cleanup_gpio(gpio_used);
        unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
        kfree(chrdev_data);
        return ret;
    }

    chrdev_class = class_create("AD_4_class");
    if (IS_ERR(chrdev_class)) {
        printk(KERN_ERR "%s: Error creando clase\n", AUTHOR);
        cdev_del(&chrdev_data->cdev);
        cleanup_gpio(gpio_used);
        unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
        kfree(chrdev_data);
        return PTR_ERR(chrdev_class);
    }

    device_create(chrdev_class, NULL, chrdev_number, NULL, "AD_4");

    printk(KERN_INFO "%s: ===== AD_4 CONFIGURADO EXITOSAMENTE =====\n", AUTHOR);
    printk(KERN_INFO "%s: GPIO utilizado: %d (fisico: %d)\n", AUTHOR, gpio_used, gpio_used - 512);
    printk(KERN_INFO "%s: Conectar LED: GPIO físico %d → Resistor → LED → GND\n", AUTHOR, gpio_used - 512);
    printk(KERN_INFO "%s: Comandos: echo 1 > /dev/AD_4 (ON)\n", AUTHOR);
    printk(KERN_INFO "%s: Comandos: echo 0 > /dev/AD_4 (OFF)\n", AUTHOR);
    printk(KERN_INFO "%s: ========================================\n", AUTHOR);
    
    return 0;
}
/*===========================FUNCION QUE SE EEJCUTAR AL REMOVER EL MODULO========================================================*/
static void __exit chrdev_exit(void)
{
    if (chrdev_data) {
        cleanup_gpio(chrdev_data->led_gpio);
    }
    
    if (chrdev_class) {
        device_destroy(chrdev_class, chrdev_number);
        class_destroy(chrdev_class);
    }
    
    if (chrdev_data) {
        cdev_del(&chrdev_data->cdev);
        kfree(chrdev_data);
    }
    
    unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
    
    printk(KERN_INFO "%s: AD_4 removido\n", AUTHOR);
}

module_init(chrdev_init);
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Controlador AD_4 con GPIO real para Raspberry Pi");
MODULE_VERSION("1.0");