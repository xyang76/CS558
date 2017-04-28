#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
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
	{ 0xe24e6a03, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xacad263e, __VMLINUX_SYMBOL_STR(pv_cpu_ops) },
	{ 0x268cc6a2, __VMLINUX_SYMBOL_STR(sys_close) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xc5fdef94, __VMLINUX_SYMBOL_STR(call_usermodehelper) },
	{ 0x8235805b, __VMLINUX_SYMBOL_STR(memmove) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x6c2e3320, __VMLINUX_SYMBOL_STR(strncmp) },
	{ 0xd0d8621b, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0xb4390f9a, __VMLINUX_SYMBOL_STR(mcount) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

