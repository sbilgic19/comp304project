#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/list.h>
/**
 * Performs a DFS on a given task's children.
 *
 * @void

*/




/*
for_each_process(task)
{
   printk("Name: %s PID: [%d]\n", task->comm, task->pid);
}
*/


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Serkan Berk Bilgiç - ALi Oktay");


int processID = 0;
module_param(processID, int, 0);
struct task_struct init_task = current;

//struct list_head *list;

/*
list_for_each(list, &init_task->children) {
    task = list_entry(list, struct task_struct, sibling);
}
 */

void DFS(struct task_struct *task)
{   
    struct task_struct *child;
    struct list_head *list;

    printk(KERN_INFO "name: %s, pid: [%d], state: %li\n", task->comm, task->pid, task->state);
    printk(KERN_INFO "Ali");
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
        DFS(child);
    }
}

/**
 * This function is called when the module is loaded. 
 *
 * @return 0  upon success
 */ 
int task_lister_init(void)
{
    printk(KERN_INFO "Loading Task Lister Module...\n");
    DFS(&init_task);

    return 0;
}
void task_lister_exit(void)
{
    printk(KERN_INFO "Removing Task Lister Module...\n");
}
// Macros for registering module entry and exit points.
module_init(task_lister_init);
module_exit(task_lister_exit);
