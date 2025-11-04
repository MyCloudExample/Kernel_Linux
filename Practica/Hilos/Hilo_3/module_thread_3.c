/*Este practico muestra la creacion de dos hilos usando lo aprendido por mi cuenta. 
* Se crean dos hilos los cuales se eejcutaran de froma alternada, es el blinky*/
/*====================================DECLARO LOS INCLUDE QUE UTILIZO===========================*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
/*=============================================================================================*/
/*====================================DEFINICIONES=============================================*/
#define AUTHOR "TD3"
#define MODULO "module_thread_3"
/*=============================================================================================*/
/*====================================VARIABLES GLOBALES=======================================*/
static struct task_struct *hilo_A;
static struct task_struct *hilo_B;
static int contador_A = 0;
static int contador_B = 0;
/*=============================================================================================*/
/*====================================FUNCIONES DE HILOS=======================================*/
static int hilo_funcion_A(void *data)
{
    while (!kthread_should_stop()) 
    {
        contador_A++;
        printk(KERN_INFO "TD3 - Hilo A ejecutándose (%d)\n", contador_A);
        msleep(2000);  // Hilo A espera 2 segundos
    }
    
    printk(KERN_INFO "TD3 - Hilo A terminado\n");
    return 0;
}

static int hilo_funcion_B(void *data)
{
    while (!kthread_should_stop()) 
    {
        contador_B++;
        printk(KERN_INFO "TD3 - Hilo B ejecutándose (%d)\n", contador_B);
        msleep(2000);  // Hilo B espera 2 segundos
    }
    
    printk(KERN_INFO "TD3 - Hilo B terminado\n");
    return 0;
}
/*=============================================================================================*/
/*====================================INICIALIZACIÓN===========================================*/
static int __init modulo_init(void)
{
    printk(KERN_INFO "[%s] - [%s] Iniciando módulo con 2 hilos simples\n",AUTHOR, MODULO);
    // Crear e iniciar hilo A (con delay inicial)
    hilo_A = kthread_create(hilo_funcion_A, NULL, "td3_hilo_A");
    if (IS_ERR(hilo_A)) 
    {
        printk(KERN_ERR "TD3 - Error creando hilo A\n");
        return PTR_ERR(hilo_A);
    }
    // Pequeño delay antes de crear hilo B para alternancia
    msleep(1000);
    // Crear e iniciar hilo B
    hilo_B = kthread_create(hilo_funcion_B, NULL, "td3_hilo_B");
    if (IS_ERR(hilo_B)) 
    {
        printk(KERN_ERR "TD3 - Error creando hilo B\n");
        kthread_stop(hilo_A);
        return PTR_ERR(hilo_B);
    }
    //Ejecuto los hilos
    wake_up_process(hilo_A);
    wake_up_process(hilo_B);
    printk(KERN_INFO "TD3 - Módulo cargado - 2 hilos iniciados\n");
    printk(KERN_INFO "TD3 - Hilos alternarán cada ~1 segundo\n");
    
    return 0;
}
/*=============================================================================================*/
/*====================================LIMPIEZA=================================================*/
static void __exit modulo_exit(void)
{
    printk(KERN_INFO "[%s] - [%s] Deteniendo hilos...\n", AUTHOR, MODULO);
    
    if (hilo_A && !IS_ERR(hilo_A)) {
        kthread_stop(hilo_A);
    }
    
    if (hilo_B && !IS_ERR(hilo_B)) {
        kthread_stop(hilo_B);
    }
    
    printk(KERN_INFO "TD3 - Estadísticas finales:\n");
    printk(KERN_INFO "TD3 - Ejecuciones Hilo A: %d\n", contador_A);
    printk(KERN_INFO "TD3 - Ejecuciones Hilo B: %d\n", contador_B);
    printk(KERN_INFO "TD3 - Módulo descargado\n");
}
/*=============================================================================================*/
module_init(modulo_init);
module_exit(modulo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Ejemplo simple: Dos hilos que se alternan con delays");