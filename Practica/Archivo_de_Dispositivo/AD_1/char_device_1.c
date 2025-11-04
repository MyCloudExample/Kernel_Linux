/*Este archivo crea un char device e implementa funciones para la lectura y escritura de lo que se ingrese por consola. Se crea un 
* modulo que creara un char device, comunicando el espacio de usuario con el espacio de kernel. Este codigo simplemente toma lo que
* se ingrese por teclado y se muestre en pantalla.*/
/*===========================ARCHIVOS DE CABECERA================================================================================*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
/*===========================DEFINICIONES========================================================================================*/
#define AUTHOR	"utn-fra-td3"	//ETIQUETA PARA EL AUTOR DEL MODULO
#define CHRDEV_MINOR	50		//MINOR NUMBER DEL DEVICE
#define CHRDEV_COUNT	1		//CANTIDAD DE DEVICES PARA RESERVAR
#define BUFFER_SIZE	1024		//TAMAÑO DEL BUFFER
/*===============================================================================================================================*/
/*===========================ESTRUCTURAS Y VARIABLES=============================================================================*/
struct chrdev_data {
    char buffer[BUFFER_SIZE];
    size_t data_size;
    struct cdev cdev;
};		//ESTRUCUTRA PARA DATOS DEL DISPOSITIVO
dev_t chrdev_number;					//VARIABLE QUE GURADA LOS AMJOR Y MINOR NUMBER DEL CHAR DEVICE
struct class *chrdev_class;				//CLASE DEL CAHR DEVICE
struct chrdev_data *chrdev_data;		//PUNTERO A LOS DATOS DEL DISPOSITIVO
/*===============================================================================================================================*/
/*===========================PROTOTIPO DE FUNCIONES==============================================================================*/
static int chrdev_open(struct inode *inode, struct file *file);
static int chrdev_release(struct inode *inode, struct file *file);
static ssize_t chrdev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos);
static ssize_t chrdev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos);
/*===============================================================================================================================*/
/*===========================OPERACIONES DE ARCHIVOS=============================================================================*/
static struct file_operations chrdev_ops = {
	.owner = THIS_MODULE,
	.open = chrdev_open,
	.release = chrdev_release,
	.read = chrdev_read,
	.write = chrdev_write,
};
/*===============================================================================================================================*/
/*===========================CUERPO DE LOS PROTOTIPOS DE FUNCIONES===============================================================*/
//==========================Función llamada al abrir el dispositivo
static int chrdev_open(struct inode *inode, struct file *file) {
	struct chrdev_data *data;
	
	// Obtener los datos del dispositivo desde inode->i_cdev
	data = container_of(inode->i_cdev, struct chrdev_data, cdev);
	file->private_data = data;
	
	printk(KERN_INFO "%s: Dispositivo abierto\n", AUTHOR);
	return 0;
}

//===========================Función llamada al cerrar el dispositivo
static int chrdev_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "%s: Dispositivo cerrado\n", AUTHOR);
	return 0;
}

//===========================Función para leer del dispositivo
static ssize_t chrdev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
	struct chrdev_data *data = file->private_data;
	ssize_t bytes_to_read;
	
	// Si ya leímos todo, retornar 0 (fin de archivo)
	if (*ppos >= data->data_size)
		return 0;
	
	// Calcular cuántos bytes podemos leer
	bytes_to_read = min(count, data->data_size - *ppos);
	
	// Copiar datos del kernel al espacio de usuario
	if (copy_to_user(user_buf, data->buffer + *ppos, bytes_to_read)) {
		return -EFAULT;
	}
	
	// Actualizar posición
	*ppos += bytes_to_read;
	
	printk(KERN_INFO "%s: Leídos %zd bytes del dispositivo\n", AUTHOR, bytes_to_read);
	return bytes_to_read;
}

//===========================Función para escribir en el dispositivo
static ssize_t chrdev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos) {
	struct chrdev_data *data = file->private_data;
	ssize_t bytes_to_write;
	
	// No escribir más allá del buffer
	bytes_to_write = min(count, (size_t)(BUFFER_SIZE - 1));
	
	// Copiar datos del espacio de usuario al kernel
	if (copy_from_user(data->buffer, user_buf, bytes_to_write)) {
		return -EFAULT;
	}
	
	// Actualizar tamaño de datos y posición
	data->data_size = bytes_to_write;
	data->buffer[bytes_to_write] = '\0'; // Null-terminate
	
	printk(KERN_INFO "%s: Escritos %zd bytes en el dispositivo: %s\n", 
	       AUTHOR, bytes_to_write, data->buffer);
	
	return bytes_to_write;
}
/*===========================INCIO DEL MODULO====================================================================================*/
static int __init chrdev_init(void) {
	int ret;
	
	// Reservar memoria para los datos del dispositivo
	chrdev_data = kzalloc(sizeof(struct chrdev_data), GFP_KERNEL);
	if (!chrdev_data) {
		printk(KERN_ERR "%s: No se pudo asignar memoria\n", AUTHOR);
		return -ENOMEM;
	}
	
	// Inicializar buffer
	strncpy(chrdev_data->buffer, "Hola desde el char device!\n", BUFFER_SIZE - 1);
	chrdev_data->data_size = strlen(chrdev_data->buffer);
	//Aca comienzo a crear el char device
	// Intento ver de reservar el char device
	if(alloc_chrdev_region(&chrdev_number, CHRDEV_MINOR, CHRDEV_COUNT, AUTHOR) < 0) {
		printk(KERN_ERR "%s: No se pudo crear el char device\n", AUTHOR);
		kfree(chrdev_data);
		return -1;
	}
	
	// Mensaje informativo para ver el major y minor number
	printk(KERN_INFO
		"%s: Reservada una region para un char device con major %d y minor %d\n",
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
		printk(KERN_ERR "%s: No se pudo agregar el char device\n", AUTHOR);
		kfree(chrdev_data);
		unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
		return ret;
	}

	// Crear estructura de clase
	chrdev_class = class_create(AUTHOR);
	if(IS_ERR(chrdev_class)) {
		printk(KERN_ERR "%s: No se pudo crear la clase del char device\n", AUTHOR);
		cdev_del(&chrdev_data->cdev);
		unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
		kfree(chrdev_data);
		return PTR_ERR(chrdev_class);
	}

	// Crear el archivo del char device
	if(IS_ERR(device_create(chrdev_class, NULL, chrdev_number, NULL, "michar"))) {
		printk(KERN_ERR "%s: No se pudo crear el char device\n", AUTHOR);
		class_destroy(chrdev_class);
		cdev_del(&chrdev_data->cdev);
		unregister_chrdev_region(chrdev_number, CHRDEV_COUNT);
		kfree(chrdev_data);
		return -1;
	}

	printk(KERN_INFO "%s: Char device creado exitosamente\n", AUTHOR);
	printk(KERN_INFO "%s: Usa: 'sudo cat /dev/michar' para leer\n", AUTHOR);
	printk(KERN_INFO "%s: Usa: 'echo \"texto\" | sudo tee /dev/michar' para escribir\n", AUTHOR);
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
	
	printk(KERN_INFO "%s: Char device removido\n", AUTHOR);
}

// Registro la funcion de inicializacion y salida
module_init(chrdev_init);
module_exit(chrdev_exit);

// Informacion del modulo
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Modulo que crea un char device con operaciones de lectura/escritura");