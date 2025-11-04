#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>

#define AUTHOR "utn-fra-td3"
#define CDEV_NAME "td3_uart"
#define CDEV_MINOR 50
#define CDEV_COUNT 1
#define BUFFER_SIZE 32

typedef struct {
    struct cdev cdev;
    dev_t cdev_number;
    struct class *cdev_class;
} td3_cdev_t;

static td3_cdev_t td3_cdev;

/**
 * @brief Envía comando al Arduino probando diferentes puertos USB
 */
static int enviar_comando_arduino(const char *comando) {
    struct file *tty_file;
    int ret;
    char comando_completo[BUFFER_SIZE];
    int longitud;
    loff_t pos = 0;
    
    // Lista de puertos a probar - ORDEN CORREGIDO
    const char *puertos[] = {
        "/dev/ttyUSB0",    // Arduino con chip CH340 (tu caso)
        "/dev/ttyUSB1",    
        "/dev/ttyACM0",    // Arduino Uno R3 oficial
        "/dev/ttyACM1",    
        "/dev/ttyAMA0",    // UART GPIO
        "/dev/ttyS0",      // UART secundario
        NULL
    };
    int i = 0;
    
    printk(KERN_INFO "%s: Intentando enviar: %s\n", AUTHOR, comando);
    
    // Probar diferentes puertos serie
    while (puertos[i]) {
        printk(KERN_INFO "%s: Probando puerto: %s\n", AUTHOR, puertos[i]);
        
        tty_file = filp_open(puertos[i], O_WRONLY | O_SYNC, 0);
        if (!IS_ERR(tty_file)) {
            printk(KERN_INFO "%s: ✓ Puerto %s abierto exitosamente\n", AUTHOR, puertos[i]);
            
            // Formatear comando con newline
            longitud = snprintf(comando_completo, sizeof(comando_completo), "%s\n", comando);
            
            printk(KERN_INFO "%s: Enviando comando: %s (%d bytes)\n", AUTHOR, comando, longitud);
            
            // Escribir en el puerto serie
            ret = kernel_write(tty_file, comando_completo, longitud, &pos);
            
            // Cerrar el archivo
            filp_close(tty_file, NULL);
            
            if (ret == longitud) {
                printk(KERN_INFO "%s: ✓ Comando enviado exitosamente por %s: %s\n", AUTHOR, puertos[i], comando);
                return 0;
            } else if (ret < 0) {
                printk(KERN_ERR "%s: ✗ Error escribiendo en %s: %d\n", AUTHOR, puertos[i], ret);
            } else {
                printk(KERN_ERR "%s: ✗ Error parcial en %s (escritos: %d, esperados: %d)\n", 
                       AUTHOR, puertos[i], ret, longitud);
            }
        } else {
            printk(KERN_DEBUG "%s: ✗ Puerto %s no disponible\n", AUTHOR, puertos[i]);
        }
        i++;
    }
    
    printk(KERN_ERR "%s: ✗ No se pudo enviar el comando. Ningún puerto disponible.\n", AUTHOR);
    return -ENODEV;
}
/**
 * @brief Se llama cuando se escribe el archivo (con echo)
 */
static ssize_t cdev_uart_write(struct file *f, const char __user *buff, size_t size, loff_t *off) {
    char *comando;
    int bytes_a_copiar;
    int no_copiados;
    int ret = 0;
    
    // Limitar tamaño
    bytes_a_copiar = (size > BUFFER_SIZE - 1) ? BUFFER_SIZE - 1 : size;
    
    // Allocar memoria para el comando
    comando = kmalloc(bytes_a_copiar + 1, GFP_KERNEL);
    if (!comando) {
        printk(KERN_ERR "%s: Error alocando memoria\n", AUTHOR);
        return -ENOMEM;
    }
    
    // Copiar datos desde usuario
    no_copiados = copy_from_user(comando, buff, bytes_a_copiar);
    comando[bytes_a_copiar] = '\0';
    
    // Remover newline si existe
    if (bytes_a_copiar > 0 && comando[bytes_a_copiar - 1] == '\n') {
        comando[bytes_a_copiar - 1] = '\0';
    }
    
    printk(KERN_INFO "%s: Comando recibido desde usuario: '%s'\n", AUTHOR, comando);
    
    // Validar y enviar comando
    if (strcmp(comando, "LED_ON") == 0) {
        ret = enviar_comando_arduino("LED_ON");
    } else if (strcmp(comando, "LED_OFF") == 0) {
        ret = enviar_comando_arduino("LED_OFF");
    } else if (strcmp(comando, "TEST") == 0) {
        ret = enviar_comando_arduino("TEST");
    } else {
        printk(KERN_WARNING "%s: Comando no válido: '%s'\n", AUTHOR, comando);
        printk(KERN_INFO "%s: Comandos válidos: LED_ON, LED_OFF, TEST\n", AUTHOR);
        ret = -EINVAL;
    }
    
    kfree(comando);
    
    if (ret < 0) {
        return ret;
    }
    
    return bytes_a_copiar - no_copiados;
}

/**
 * @brief Operación read - no implementada
 */
static ssize_t cdev_uart_read(struct file *f, char __user *buff, size_t size, loff_t *off) {
    return 0;
}

/**
 * @brief Operación open
 */
static int cdev_uart_open(struct inode *i, struct file *f) {
    printk(KERN_DEBUG "%s: Dispositivo abierto\n", AUTHOR);
    return 0;
}

/**
 * @brief Operación close
 */
static int cdev_uart_close(struct inode *i, struct file *f) {
    printk(KERN_DEBUG "%s: Dispositivo cerrado\n", AUTHOR);
    return 0;
}

// Operaciones del archivo del dispositivo
static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = cdev_uart_open,
    .release = cdev_uart_close,
    .read = cdev_uart_read,
    .write = cdev_uart_write,
};

/**
 * @brief Se llama cuando se carga el modulo en el kernel
 */
static int __init td3_uart_init(void) {
    int ret;
    
    printk(KERN_INFO "%s: Iniciando controlador UART para Arduino USB\n", AUTHOR);
    
    // Crear char device
    ret = alloc_chrdev_region(&td3_cdev.cdev_number, CDEV_MINOR, CDEV_COUNT, CDEV_NAME);
    if(ret < 0) {
        printk(KERN_ERR "%s: Error creando char device: %d\n", AUTHOR, ret);
        return ret;
    }
    
    printk(KERN_INFO "%s: Char device en major %d, minor %d\n", 
           AUTHOR, MAJOR(td3_cdev.cdev_number), MINOR(td3_cdev.cdev_number));
    
    // Inicializar char device
    cdev_init(&td3_cdev.cdev, &fops);
    td3_cdev.cdev.owner = THIS_MODULE;
    
    // Añadir char device al sistema
    ret = cdev_add(&td3_cdev.cdev, td3_cdev.cdev_number, CDEV_COUNT);
    if(ret < 0) {
        printk(KERN_ERR "%s: Error añadiendo char device: %d\n", AUTHOR, ret);
        unregister_chrdev_region(td3_cdev.cdev_number, CDEV_COUNT);
        return ret;
    }
    
    // Crear clase del dispositivo
    td3_cdev.cdev_class = class_create(CDEV_NAME);
    if(IS_ERR(td3_cdev.cdev_class)) {
        ret = PTR_ERR(td3_cdev.cdev_class);
        printk(KERN_ERR "%s: Error creando clase: %d\n", AUTHOR, ret);
        cdev_del(&td3_cdev.cdev);
        unregister_chrdev_region(td3_cdev.cdev_number, CDEV_COUNT);
        return ret;
    }
    
    // Crear dispositivo en /dev/
    if(IS_ERR(device_create(td3_cdev.cdev_class, NULL, td3_cdev.cdev_number, NULL, CDEV_NAME))) {
        printk(KERN_ERR "%s: Error creando dispositivo\n", AUTHOR);
        class_destroy(td3_cdev.cdev_class);
        cdev_del(&td3_cdev.cdev);
        unregister_chrdev_region(td3_cdev.cdev_number, CDEV_COUNT);
        return -1;
    }
    
    printk(KERN_INFO "%s: Controlador UART USB inicializado CORRECTAMENTE\n", AUTHOR);
    printk(KERN_INFO "%s: Conecte el Arduino por USB y use:\n", AUTHOR);
    printk(KERN_INFO "%s:   echo 'LED_ON' > /dev/%s\n", AUTHOR, CDEV_NAME);
    printk(KERN_INFO "%s:   echo 'LED_OFF' > /dev/%s\n", AUTHOR, CDEV_NAME);
    printk(KERN_INFO "%s:   echo 'TEST' > /dev/%s\n", AUTHOR, CDEV_NAME);
    
    return 0;
}

/**
 * @brief Se llama cuando se remueve el modulo del kernel
 */
static void __exit td3_uart_exit(void) {
    printk(KERN_INFO "%s: Descargando controlador UART USB\n", AUTHOR);
    
    // Limpiar recursos
    device_destroy(td3_cdev.cdev_class, td3_cdev.cdev_number);
    class_destroy(td3_cdev.cdev_class);
    cdev_del(&td3_cdev.cdev);
    unregister_chrdev_region(td3_cdev.cdev_number, CDEV_COUNT);
    
    printk(KERN_INFO "%s: Controlador UART USB descargado\n", AUTHOR);
}

module_init(td3_uart_init);
module_exit(td3_uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Controlador UART para Arduino USB");