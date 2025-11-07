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
	{ 0x92997ed8, "_printk" },
	{ 0x418c10ec, "__register_chrdev" },
	{ 0x59c02473, "class_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x2c9a4c10, "device_create" },
	{ 0x6775d5d3, "class_destroy" },
	{ 0x8b970f46, "device_destroy" },
	{ 0x52c5c991, "__kmalloc_noprof" },
	{ 0xdcb764ad, "memset" },
	{ 0x37a0cba, "kfree" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x1e477abd, "filp_open" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x9a4a2a5c, "kernel_write" },
	{ 0x9b13921b, "filp_close" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "4C5B72DC3E4250A0CC2E551");
