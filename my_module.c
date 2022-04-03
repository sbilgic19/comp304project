#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>

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
struct task_struct init_task;
void DFS(struct task_struct *task)
{   
    struct task_struct *child;
    struct list_head *list;

    printk("name: %s, pid: [%d], state: %li\n", task->comm, task->pid, task->state);
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

MODULE_LICENSE("GPL");
//MODULE_AUTHOR("Aditya");
//MODULE_DESCRIPTION("The character device driver");
