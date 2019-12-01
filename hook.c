#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/pgtable_types.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
#include <asm/current.h>
#include <linux/string.h>

static char __current_filename[256];

MODULE_LICENSE("GPL");
MODULE_AUTHOR("D0hnuts");


unsigned long* find_sys_call_table(void)  
{
    return   (unsigned long *)kallsyms_lookup_name("sys_call_table");

}

static unsigned long *system_call_table_addr; 

static asmlinkage long (*__original_open) (const char __user *, int, int);
static asmlinkage long (*__original_write)(unsigned int, const char __user *, size_t);
int __open_method_index = __NR_open;
int __write_method_index = __NR_write;


static asmlinkage long __hook_open(const char __user *filename,                       
               int flags, int mode){
   
    int len = strnlen_user(filename, 256);
    copy_from_user (__current_filename,filename, len);
    printk(KERN_INFO "[OpenHook]: %s open %s", current->comm, __current_filename);
    return __original_open(filename, flags, mode);
}

static asmlinkage long __hook_write(unsigned int fd, const char __user * buf, size_t count){
 
    printk(KERN_INFO "[WriteHook]: %s  write %ld bytes", current->comm, count);

    return __original_write(fd, buf, count);
}


int make_rw(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
    }
    return 0;
}

int make_ro(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}



static int __init entry_point(void){
    printk(KERN_INFO "[Hook]: loaded..\n");
    printk(KERN_INFO "[Hook]: find syscall table..\n");
    system_call_table_addr = (unsigned long)find_sys_call_table();
    printk(KERN_INFO "[Hook]: %p\n", system_call_table_addr + __open_method_index);
    printk(KERN_INFO "[Hook]: hooking...\n");
    __original_open = (void*) *(system_call_table_addr + __open_method_index);
    __original_write = (void*) *(system_call_table_addr + __write_method_index);

    /*Disable page protection*/
    make_rw((unsigned long)system_call_table_addr);
    /*Change syscall to our syscall function*/
    *(system_call_table_addr + __open_method_index) = (unsigned long) __hook_open;
    *(system_call_table_addr + __write_method_index) = (unsigned long) __hook_write;

    make_ro((unsigned long)system_call_table_addr);

    printk(KERN_INFO "[Hook]: Hook success");
    return 0;
}
static void __exit exit_point(void){
    make_rw((unsigned long)system_call_table_addr);
    /*Restore original system call */
    *(system_call_table_addr + __open_method_index) = (unsigned long)__original_open;
    *(system_call_table_addr + __write_method_index) = (unsigned long)__original_write;

    /*Renable page protection*/
    make_ro((unsigned long)system_call_table_addr);
    printk(KERN_INFO "[Hook]: Unloaded successfully\n");
}
module_init(entry_point);
module_exit(exit_point);
