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
	{ 0xda9e78e9, "module_layout" },
	{ 0x6d044c26, "param_ops_uint" },
	{ 0x74738f6d, "cdev_del" },
	{ 0xeed38436, "malloc_sizes" },
	{ 0x37a0cba, "kfree" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xf20dabd8, "free_irq" },
	{ 0xd6b8e852, "request_threaded_irq" },
	{ 0xe3321d40, "cdev_add" },
	{ 0xc068440e, "__kfifo_alloc" },
	{ 0x63b87fc5, "__init_waitqueue_head" },
	{ 0x60c2503e, "cdev_init" },
	{ 0xd197d610, "kmem_cache_alloc" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x4578f528, "__kfifo_to_user" },
	{ 0x8893fa5d, "finish_wait" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0x75a17bed, "prepare_to_wait" },
	{ 0xb9e52429, "__wake_up" },
	{ 0xf23fcb99, "__kfifo_in" },
	{ 0x27e1a049, "printk" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "A5ED378E0BE4880D867B81B");
