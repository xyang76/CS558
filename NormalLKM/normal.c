/*lkm.c*/
 
#include <linux/module.h>    
#include <linux/kernel.h>   
#include <linux/init.h>    
#include <linux/vermagic.h> 

MODULE_INFO(vermagic, "3.2.0-126-generic SMP mod_unload modversions 686 ");
static int lkm_init(void)
{
    //Hide this module from kernel modules(lsmod) 
    //list_del_init(&__this_module.list);

    //Hide this module from /sys/modules
    //kobject_del(&THIS_MODULE->mkobj.kobj);
    printk("Hello world, module loaded\n");
    return 0;    
}
 
static void lkm_exit(void)
{
    printk("Hello world, module removed\n");
}
 
module_init(lkm_init);
module_exit(lkm_exit);