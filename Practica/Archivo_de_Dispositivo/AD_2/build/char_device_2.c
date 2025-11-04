/* 
 * Esqueleto básico de Char Device - Solo crea el archivo dispositivo
 * Sin funcionalidad de lectura/escritura
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

// Etiqueta para el autor del modulo
#define AUTHOR	"TD3"

// Minor number del device
#define CHRDEV_MINOR	50
// Cantidad de devices para reservar
#define CHRDEV_COUNT	1
// Tamaño del buffer
#define BUFFER_SIZE	1024

// Estructura para datos del dispositivo
struct chrdev_data {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    struct cdev cdev;
};

// Variable que guarda los major y minor numbers del char device
dev_t chrdev_number;
// Clase del char device
struct class *chrdev_class;
// Puntero a los datos del dispositivo
struct chrdev_data *chrdev_data;

// Prototipos de funciones
static int chrdev_open(struct inode *inode, struct file *file);
static int chrdev_release(struct inode *inode, struct file *file);
static ssize_t chrdev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
static ssize_t chrdev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);

// Operaciones de archivos COMPLETAS
static struct file_operations chrdev_ops = {
	.owner = THIS_MODULE,
	.open = chrdev_open,
	.release = chrdev_release,
	.read = chrdev_read,
	.write = chrdev_write,
};

/**
 * @brief Función llamada al abrir el dispositivo
 */
static int chrdev_open(struct inode *inode, struct file *file) {
	struct chrdev_data *data;
	
	// Obtener los datos del dispositivo desde inode->i_cdev
	data = container_of(inode->i_cdev, struct chrdev_data, cdev);
	file->private_data = data;
	
	printk(KERN_INFO "%s: [/dev/AD_2] Dispositivo abierto\n", AUTHOR);
	return 0;
}

/**
 * @brief Función llamada al cerrar el dispositivo
 */
static int chrdev_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "%s: [/dev/AD_2] Dispositivo cerrado\n", AUTHOR);
	return 0;
}

/**
 * @brief Función para leer del dispositivo
 */
static ssize_t chrdev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
	struct chrdev_data *data = file->private_data;
	ssize_t bytes_to_read;
	
	// Si ya leímos todo, retornar 0 (fin de archivo)
	if (*ppos >= data->data_size) {
		printk(KERN_INFO "%s: [/dev/AD_2] Lectura completada\n", AUTHOR);
		return 0;
	}
	
	// Calcular cuántos bytes podemos leer
	bytes_to_read = min(count, data->data_size - (size_t)*ppos);
	
	// Copiar datos del kernel al espacio de usuario
	if (copy_to_user(user_buf, data->buffer + *ppos, bytes_to_read)) {
		printk(KERN_ERR "%s: [/dev/AD_2] Error copiando datos a usuario\n", AUTHOR);
		return -EFAULT;
	}
	
	// Actualizar posición
	*ppos += bytes_to_read;
	
	printk(KERN_INFO "%s: [/dev/AD_2] Leídos %zd bytes: '%.*s'\n", 
	       AUTHOR, bytes_to_read, (int)bytes_to_read, data->buffer + (*ppos - bytes_to_read));
	return bytes_to_read;
}

/**
 * @brief Función para escribir en el dispositivo
 */
static ssize_t chrdev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos) {
	struct chrdev_data *data = file->private_data;
	ssize_t bytes_to_write;
	
	// No escribir más allá del buffer
	bytes_to_write = min(count, (size_t)(BUFFER_SIZE - 1));
	
	if (bytes_to_write == 0) {
		printk(KERN_WARNING "%s: [/dev/AD_2] Intento de escribir 0 bytes\n", AUTHOR);
		return -ENOSPC;
	}
	
	// Copiar datos del espacio de usuario al kernel
	if (copy_from_user(data->buffer, user_buf, bytes_to_write)) {
		printk(KERN_ERR "%s: [/dev/AD_2] Error copiando datos desde usuario\n", AUTHOR);
		return -EFAULT;
	}
	
	// Actualizar tamaño de datos y posición
	data->data_size = bytes_to_write;
	data->buffer[bytes_to_write] = '\0'; // Null-terminate
	
	printk(KERN_INFO "%s: [/dev/AD_2] Escritos %zd bytes: '%s'\n", 
	       AUTHOR, bytes_to_write, data->buffer);
	
	return bytes_to_write;
}

/**
 * @brief Se llama cuando el modulo se carga en el kernel
 */
static int __init chrdev_init(void) {
	int ret;
	
	// Reservar memoria para los datos del dispositivo
	chrdev_data = kzalloc(sizeof(struct chrdev_data), GFP_KERNEL);
	if (!chrdev_data) {
		printk(KERN_ERR "%s: [/dev/AD_2] No se pudo asignar memoria\n", AUTHOR);
		return -ENOMEM;
	}
	
	// Inicializar buffer con mensaje por defecto
	strncpy(chrdev_data->buffer, "Bienvenido a AD_2 - Dispositivo funcional!\n", BUFFER_SIZE - 1);
	chrdev_data->data_size = strlen(chrdev_data->buffer);

	// Reservar el char device
	ret = alloc_chrdev_region(&chrdev_number, CHRDEV_MINOR, CHRDEV_COUNT, "AD_2");
	if (ret < 0) {
		printk(KERN_ERR "%s: [/dev/AD_2] No se pudo crear el char device\n", AUTHOR);
		kfree(chrdev_data);
		return ret;
	}
	
	// Mensaje informativo para ver el major y minor number
	printk(KERN_INFO
		"%s: [/dev/AD_2] Reservada region con major %d y minor %d\n",
		AUTHOR,
		MAJOR(chrdev_number),
		MINOR(chrdev_number)
	);

	// Inicializar el char device con sus operaciones
	cdev_init(&chrdev_data->cdev, &chrdev_ops);
	chrdev_data->cdev.owner = THIS_MODULE;
	
	// Asociar el char device con la region reservada
	ret = cdev_add(&chrdev_data->cdev, chrdev_number, CHRDEV_COUNT);
	if (ret < 0) {
		printk(KERN_ERR "%s: [/dev/AD_2] No se pudo agregar el char device\n", AUTHOR);
		kfree(chrdev_data);
		unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
		return ret;
	}

	// Crear estructura de clase
	chrdev_class = class_create("AD_2_class");
	if(IS_ERR(chrdev_class)) {
		printk(KERN_ERR "%s: [/dev/AD_2] No se pudo crear la clase\n", AUTHOR);
		cdev_del(&chrdev_data->cdev);
		unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
		kfree(chrdev_data);
		return PTR_ERR(chrdev_class);
	}

	// Crear el archivo del char device
	if(IS_ERR(device_create(chrdev_class, NULL, chrdev_number, NULL, "AD_2"))) {
		printk(KERN_ERR "%s: [/dev/AD_2] No se pudo crear el dispositivo\n", AUTHOR);
		class_destroy(chrdev_class);
		cdev_del(&chrdev_data->cdev);
		unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
		kfree(chrdev_data);
		return -1;
	}

	printk(KERN_INFO "%s: [/dev/AD_2] Dispositivo creado EXITOSAMENTE con operaciones completas\n", AUTHOR);
	printk(KERN_INFO "%s: Usa: 'cat /dev/AD_2' para leer\n", AUTHOR);
	printk(KERN_INFO "%s: Usa: 'echo \"texto\" | sudo tee /dev/AD_2' para escribir\n", AUTHOR);
	return 0;
}

/**
 * @brief Se llama cuando el modulo se quita del kernel
 */
static void __exit chrdev_exit(void) {
	// Liberar recursos en orden inverso
	device_destroy(chrdev_class, chrdev_number);
	class_destroy(chrdev_class);
	cdev_del(&chrdev_data->cdev);
	unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
	kfree(chrdev_data);
	
	printk(KERN_INFO "%s: [/dev/AD_2] Dispositivo removido\n", AUTHOR);
}

// Registro la funcion de inicializacion y salida
module_init(chrdev_init);
module_exit(chrdev_exit);

// Informacion del modulo
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("AD_2 - Dispositivo de caracteres completamente funcional");
MODULE_VERSION("1.0");