#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x51198477, "module_layout" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x456e4056, "cdev_del" },
	{ 0xead65a40, "cdev_add" },
	{ 0x7fa6a0b4, "cdev_init" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x4f8b5ddb, "_copy_to_user" },
	{ 0xfa66f77c, "finish_wait" },
	{ 0x5c8b5ce8, "prepare_to_wait" },
	{ 0x1000e51, "schedule" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x4f6b400b, "_copy_from_user" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xcf21d241, "__wake_up" },
	{ 0x69371301, "current_task" },
	{ 0x27e1a049, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "036B2AAD5D35822E3E49E08");
