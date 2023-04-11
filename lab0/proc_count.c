#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>

// static int __init proc_count_init(void)
// {
// 	pr_info("proc_count: init\n");
// 	return 0;
// }

// static void __exit proc_count_exit(void)
// {
// 	pr_info("proc_count: exit\n");
// }




//Define a pointer to a proc_dir_entry struct called entry. 
//This will be used to create and remove the virtual file in the /proc filesystem.
static struct proc_dir_entry *entry;

//Define the example function, which takes a pointer to a seq_file struct and a void pointer. 
//This function is called when the virtual file is read. 
static int proc_count(struct seq_file *m, void *v){
	//It first prints "hello world" to the seq_file, 
	seq_printf(m, "hello world\n");
	struct task_struct *p;
	int sqr_sum = 0;
	//then iterates over each process in the system using the for_each_process macro. 
	for_each_process(p) {
		//For each process, it prints "hello world" to the seq_file. 
		seq_printf(m, "hello world\n");
	}
	//The function then returns 0 to indicate success.
	return 0;
}

//Called when the module is loaded into the kernel. 
static int __init proc_count_init(void)
{
	//This function creates the virtual file "example_virtual_file" 
	//in the /proc filesystem with a proc_create_single call, 
	//using the example function as a callback for file reads.
	entry = proc_create_single("count", 0, NULL, proc_count);
	//Then, it prints "example: init" to the kernel log to indicate that the module has been initialized. 
	//The function returns 0 to indicate success.
	pr_info("example: init\n");
	return 0;
}

//It is called when the module is unloaded from the kernel. 
//This function removes the virtual file from the /proc filesystem 
//using the proc_remove function and then prints "example: exit" to the kernel log 
//to indicate that the module has been removed.
static void __exit proc_count_exit(void)
{
	proc_remove(entry);
	pr_info("example: exit\n");
}

module_init(proc_count_init);
module_exit(proc_count_exit);

MODULE_AUTHOR("Minhao Ren");
MODULE_DESCRIPTION("Count and return the current number of running processes");
MODULE_LICENSE("GPL");