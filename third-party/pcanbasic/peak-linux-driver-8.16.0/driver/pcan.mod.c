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
	{ 0xbbcac5c1, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x2d3385d3, "system_wq" },
	{ 0x85bd1608, "__request_region" },
	{ 0xbde1edaf, "kmalloc_caches" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xc4f0da12, "ktime_get_with_offset" },
	{ 0xf90a1e85, "__x86_indirect_thunk_r8" },
	{ 0xc1393493, "pci_free_irq_vectors" },
	{ 0xd2ca5874, "pci_write_config_word" },
	{ 0x53f9460a, "single_open" },
	{ 0x77358855, "iomem_resource" },
	{ 0xb17e1d3b, "dma_set_mask" },
	{ 0x28e3e0b5, "single_release" },
	{ 0x66ca4f0c, "usb_reset_endpoint" },
	{ 0x56a77833, "pci_disable_device" },
	{ 0x5be8e9ab, "i2c_transfer" },
	{ 0x20000329, "simple_strtoul" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0x6df0c3b1, "seq_printf" },
	{ 0x56470118, "__warn_printk" },
	{ 0xb43f9365, "ktime_get" },
	{ 0x56437f6f, "usb_kill_urb" },
	{ 0x778ebe7a, "remove_proc_entry" },
	{ 0xc7b5fbdd, "device_destroy" },
	{ 0x443c57ff, "__register_chrdev" },
	{ 0xb4911d2e, "driver_for_each_device" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xa66ae2be, "pci_release_regions" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x9fa7184a, "cancel_delayed_work_sync" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x3e47f023, "dma_free_attrs" },
	{ 0x498d075d, "device_create_with_groups" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x5525808d, "seq_read" },
	{ 0x114b8c79, "pv_ops" },
	{ 0x35da7435, "dma_set_coherent_mask" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0xa28f0369, "dma_get_required_mask" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xdac8906c, "param_ops_charp" },
	{ 0xdc545091, "pci_set_master" },
	{ 0x97934ecf, "del_timer_sync" },
	{ 0x6b807fa9, "pci_alloc_irq_vectors_affinity" },
	{ 0x341ce907, "_dev_warn" },
	{ 0xfb578fc5, "memset" },
	{ 0xdbdf6c92, "ioport_resource" },
	{ 0x1e1e140e, "ns_to_timespec64" },
	{ 0x275d5c56, "pci_iounmap" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0x1f0cd6a1, "current_task" },
	{ 0x37befc70, "jiffies_to_msecs" },
	{ 0x805540f1, "usb_deregister" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0xfef216eb, "_raw_spin_trylock" },
	{ 0xe1537255, "__list_del_entry_valid" },
	{ 0xe116abd, "sysfs_remove_file_from_group" },
	{ 0x449ad0a7, "memcmp" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0x573f071b, "class_unregister" },
	{ 0xde80cd09, "ioremap" },
	{ 0x1edb69d6, "ktime_get_raw_ts64" },
	{ 0xdb57f48d, "usb_set_interface" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0x9166fada, "strncpy" },
	{ 0x6b7bed4f, "usb_control_msg" },
	{ 0xff25c139, "pci_read_config_word" },
	{ 0x670ecece, "__x86_indirect_thunk_rbx" },
	{ 0x96764e55, "dma_alloc_attrs" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xc1317850, "__class_register" },
	{ 0x6da2c785, "_dev_err" },
	{ 0x68f31cbd, "__list_add_valid" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0xb0f14517, "pci_find_capability" },
	{ 0x167c5967, "print_hex_dump" },
	{ 0x64af98b1, "i2c_del_adapter" },
	{ 0x131a3825, "_dev_info" },
	{ 0x38709363, "usb_submit_urb" },
	{ 0x6383b27c, "__x86_indirect_thunk_rdx" },
	{ 0x12a38747, "usleep_range" },
	{ 0xb2fcb56d, "queue_delayed_work_on" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0xe6f49ee9, "usb_reset_device" },
	{ 0x24075d51, "usb_bulk_msg" },
	{ 0x1000e51, "schedule" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0xefd26ac6, "usb_clear_halt" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x1035c7c2, "__release_region" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xc61aa8e2, "pci_unregister_driver" },
	{ 0xcc5005fe, "msleep_interruptible" },
	{ 0xc50dfc48, "kmem_cache_alloc_trace" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0x68261de, "param_ops_byte" },
	{ 0x22d7b581, "pci_irq_vector" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x5ffe9da8, "seq_lseek" },
	{ 0x37a0cba, "kfree" },
	{ 0x69acdf38, "memcpy" },
	{ 0x71fa4c33, "pci_request_regions" },
	{ 0xa73be33d, "param_array_ops" },
	{ 0xedc03953, "iounmap" },
	{ 0xd8f6ad13, "__pci_register_driver" },
	{ 0xdad0cc97, "usb_register_driver" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x95dcc9b5, "sysfs_add_file_to_group" },
	{ 0xc7b61a54, "i2c_bit_add_bus" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xb0e602eb, "memmove" },
	{ 0x2d9f95a5, "pci_iomap" },
	{ 0xb29dd007, "param_ops_ushort" },
	{ 0xe362c6de, "proc_create" },
	{ 0x58501734, "usb_get_current_frame_number" },
	{ 0x5e515be6, "ktime_get_ts64" },
	{ 0x7f02188f, "__msecs_to_jiffies" },
	{ 0x613f89cf, "pci_enable_device" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xaa77e9b, "param_ops_ulong" },
	{ 0xc6337865, "param_ops_uint" },
	{ 0x351cc8d7, "usb_free_urb" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x2ee80948, "usb_alloc_urb" },
	{ 0xc1514a3b, "free_irq" },
};

MODULE_INFO(depends, "usbcore,i2c-algo-bit");

MODULE_ALIAS("pci:v0000001Cd00000001sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000003sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000004sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000005sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000006sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000007sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000008sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000009sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000002sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd0000000Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000010sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000013sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000016sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000017sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd00000019sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000001Cd0000001Asv*sd*bc*sc*i*");
MODULE_ALIAS("usb:v0C72p000Cd*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0C72p000Dd*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0C72p0012d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0C72p0011d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0C72p0013d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0C72p0014d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "197C71CFA85FACF4F5DF984");
