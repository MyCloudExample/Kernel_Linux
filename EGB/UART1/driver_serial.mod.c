#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x122c3a7e, "_printk" },
	{ 0xd75c6742, "__register_chrdev" },
	{ 0xf311fc60, "class_create" },
	{ 0x93ab9e33, "device_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x4a41ecb3, "class_destroy" },
	{ 0xe095e43a, "device_destroy" },
	{ 0x52c5c991, "__kmalloc_noprof" },
	{ 0xdcb764ad, "memset" },
	{ 0x213a7d94, "filp_open" },
	{ 0xbe8b47af, "kernel_write" },
	{ 0xf91e01de, "filp_close" },
	{ 0x37a0cba, "kfree" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x39ff040a, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "32BC13D10756DE28FA8FF7F");
