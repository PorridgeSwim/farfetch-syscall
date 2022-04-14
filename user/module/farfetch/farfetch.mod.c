#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xf8d9c039, "module_layout" },
	{ 0x3a49759, "farfetch_default" },
	{ 0x3980d0bb, "farfetch_ptr" },
	{ 0x67852479, "set_page_dirty_lock" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xc5850110, "printk" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0x6fbdae70, "__put_task_struct" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x1b06510e, "pv_ops" },
	{ 0x3372cfe3, "from_kuid_munged" },
	{ 0x2d5f69b3, "rcu_read_unlock_strict" },
	{ 0x81317164, "current_task" },
	{ 0xcb4080e3, "put_pid" },
	{ 0x942f0b36, "get_pid_task" },
	{ 0xdb93ac96, "find_get_pid" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x56470118, "__warn_printk" },
};

MODULE_INFO(depends, "");

