/*Este codigo permite tomar datos del tipo int desde el exterior. Se realizaron pruebas con diferentes valores, estas pruebas
* se pueden obsrvar de menera rapida usando el script tes_param.sh. Tambien se realizo una prueba pasando datos por la terminal
* para ellos es nesesario usar el comando:
* sudo insmod build/param.ko variable1= valor1 variable2= valor2 variable3= valor3 ... variablen= valorn*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
/*=====================================DEFINICIONES====================================================================*/
#define AUTHOR "utn-fra-td3"
#define MODULO "param_2"
/*=====================================================================================================================*/
/*=====================================DEFINICION DE VARIABLES=========================================================*/
static int mi_numero = 10;
static int repeticiones = 1;
static int debug = 0;
/*=====================================================================================================================*/
/*=====================================================================================================================*/
module_param(mi_numero, int, 0644);
module_param(repeticiones, int, 0644);
module_param(debug, int, 0644); //Actua como un boton de encendido
/*=====================================================================================================================*/
/*=====================================DOCUMENTACION DEL SOFTWARE======================================================*/
MODULE_PARM_DESC(mi_numero, "Número entero a procesar");
MODULE_PARM_DESC(repeticiones, "Número de veces a repetir (1-20)");
MODULE_PARM_DESC(debug, "Modo debug (0=Off, 1=On)");
/*=====================================================================================================================*/
static int __init param_int_init(void) 
{
    int i;
    
    if (debug)
        printk("%s | %s: DEBUG - Iniciando módulo con número=%d, repeticiones=%d\n", AUTHOR, MODULO, mi_numero, repeticiones);
    
    // Validar parámetros
    if (repeticiones < 1 || repeticiones > 20) 
    {
        printk("%s | %s: ERROR: repeticiones debe estar entre 1-20. Usando 1\n", AUTHOR, MODULO);
        repeticiones = 1;
    }
    
    printk("%s | %s: === MÓDULO ENTERO INICIADO ===\n", AUTHOR, MODULO);
    printk("%s | %s: Número recibido: %d\n", AUTHOR, MODULO, mi_numero);
    printk("%s | %s: Repeticiones: %d\n", AUTHOR, MODULO, repeticiones);
    
    // Procesar el número
    printk("%s | %s: --- PROCESANDO NÚMERO ---\n", AUTHOR, MODULO);
    
    for (i = 0; i < repeticiones; i++) 
    {
        printk("%s | %s: Iteración %d: Número = %d\n", AUTHOR, MODULO, i+1, mi_numero);
        
        // Algunas operaciones con el número
        if (debug) 
        {
            printk("%s | %s: DEBUG - Cuadrado: %d, Cubo: %d\n", 
                   AUTHOR, MODULO, mi_numero * mi_numero, mi_numero * mi_numero * mi_numero);
        }
    }
    
    // Información adicional sobre el número
    if (mi_numero > 0) {
        printk("%s | %s: El número es POSITIVO\n", AUTHOR, MODULO);
    } 
    else if (mi_numero < 0) 
    {
        printk("%s | %s: El número es NEGATIVO\n", AUTHOR, MODULO);
    } 
    else 
    {
        printk("%s | %s: El número es CERO\n", AUTHOR, MODULO);
    }
    
    if (mi_numero % 2 == 0) 
    {
        printk("%s | %s: El número es PAR\n", AUTHOR, MODULO);
    } 
    else 
    {
        printk("%s | %s: El número es IMPAR\n", AUTHOR, MODULO);
    }
    
    if (debug)
        printk("%s | %s: DEBUG - Procesamiento completado\n", AUTHOR, MODULO);
        
    return 0;
}

static void __exit param_int_exit(void) 
{
    printk("%s | %s: === MÓDULO ENTERO DESCARGADO ===\n", AUTHOR, MODULO);
    printk("%s | %s: Último número procesado: %d\n", AUTHOR, MODULO, mi_numero);
}

module_init(param_int_init);
module_exit(param_int_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Módulo que procesa números enteros desde parámetros");
MODULE_VERSION("1.0");
