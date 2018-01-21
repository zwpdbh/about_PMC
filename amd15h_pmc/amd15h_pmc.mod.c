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
	{ 0x6b66aa4a, "module_layout" },
	{ 0x61b7b126, "simple_strtoull" },
	{ 0xf9b50079, "kobject_put" },
	{ 0x5a34a45c, "__kmalloc" },
	{ 0xd7dd777b, "reserve_perfctr_nmi" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0xe85060c1, "kobject_del" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xb87dd3df, "kobject_create_and_add" },
	{ 0x4d8c750, "release_perfctr_nmi" },
	{ 0x740a1b95, "reserve_evntsel_nmi" },
	{ 0xea147363, "printk" },
	{ 0x33262a75, "sysfs_create_group" },
	{ 0xb4390f9a, "mcount" },
	{ 0xe997667b, "wrmsr_on_cpu" },
	{ 0x341cbed2, "cpu_present_mask" },
	{ 0xa70fabbe, "release_evntsel_nmi" },
	{ 0x57adf756, "per_cpu__this_cpu_off" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x620ae6c0, "pv_cpu_ops" },
	{ 0x27a8d660, "kernel_kobj" },
	{ 0xb1cfad22, "rdmsr_on_cpu" },
	{ 0x37a0cba, "kfree" },
	{ 0x4cbbd171, "__bitmap_weight" },
	{ 0xb742fd7, "simple_strtol" },
	{ 0x69aef723, "per_cpu__cpu_info" },
	{ 0x9edbecae, "snprintf" },
	{ 0xc33f6f4c, "on_each_cpu" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "B4D2157EEBB4A57095A71BC");
