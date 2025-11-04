/*Este practico muestra la creacion de un solo hilo usando lo aprendido por mi cuenta. 
* Se creara un hilo y se ejecutara el modulo, se usaran las funciones kthread_create() y kthread_wke_up()*/
/*====================================DECLARO LOS INCLUDE QUE UTILIZO==================================================*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
/*=====================================================================================================================*/
/*====================================DEFINICIONES=====================================================================*/
#define AUTHOR "TD3"
/*=====================================================================================================================*/
/*====================================DECLARO LAS VARAIBLES PARA CREAR UN SOLO HILO====================================*/
static struct task_struct *mi_hilo;
/*=====================================================================================================================*/
/*====================================CREO LA FUNCION QUE DEFINE A CADA HILO===========================================*/
static int funcion_hilo(void *data)
{
    while (!kthread_should_stop()) 
    {
        printk(KERN_INFO "TD3 -Usando kthread_create y wake_up_process - Hilo ejecutandose...\n");
        msleep(1000);
    }
    return 0;
}
/*=====================================================================================================================*/
/*====================================FUNCION QUE SE EJECUTARA AL CARGAR EL MODULO=====================================*/
static int __init modulo_init(void)
{
    mi_hilo = kthread_create(funcion_hilo, NULL, "mi_hilo_controlado");
    
    if (IS_ERR(mi_hilo)) {
        printk(KERN_ERR "TD3 - Error creando hilo\n");
        return PTR_ERR(mi_hilo);
    }
    
    printk(KERN_INFO "TD3 - Hilo creado (pausado), verificando...\n");
    
    wake_up_process(mi_hilo);
    printk(KERN_INFO "TD3 - Modulo cargado - Hilo iniciado\n");
    
    return 0;
}
/*=====================================================================================================================*/
/*====================================FUNCION QUE SE EJECUTARA AL REMOVER EL MODULO====================================*/
static void __exit modulo_exit(void)
{
    if (mi_hilo && !IS_ERR(mi_hilo)) 
    {
        kthread_stop(mi_hilo);
    }
    printk(KERN_INFO "TD3 - Modulo descargado - Hilo detenido\n");
}
/*=====================================================================================================================*/
module_init(modulo_init);
module_exit(modulo_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION("Ejemplo 2 de hilos: creacion de un hilo");