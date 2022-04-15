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
	{ 0x769581a3, "module_layout" },
	{ 0x3a49759, "farfetch_default" },
	{ 0x3980d0bb, "farfetch_ptr" },
	{ 0x1b67a599, "set_page_dirty_lock" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0xc5850110, "printk" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x193f717f, "__put_task_struct" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0xf858fb98, "get_user_pages_remote" },
	{ 0x2d4e378f, "from_kuid_munged" },
	{ 0x2d5f69b3, "rcu_read_unlock_strict" },
	{ 0xf0fecfe5, "current_task" },
	{ 0xf8c6dda4, "put_pid" },
	{ 0xdceac98d, "get_pid_task" },
	{ 0x95b180c8, "find_get_pid" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x3ac44fb5, "__put_page" },
	{ 0x7350c06a, "put_devmap_managed_page" },
	{ 0x587f22d7, "devmap_managed_key" },
	{ 0x56470118, "__warn_printk" },
};

MODULE_INFO(depends, "");

