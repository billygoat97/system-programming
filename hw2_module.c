#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>
#include <linux/string.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/mm_types.h>
#include <linux/types.h>
#include <uapi/linux/sched.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/slab.h>

#define PROC_NAME "hw2"
#define NM 1000000
MODULE_AUTHOR("Kim, Kihyun");
MODULE_LICENSE("GPL");


static int pid = 0;
static int period = 10;
static int flag = 0;
static int count = 0;
module_param(pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(period, "An integer");
module_param(period, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(pid, "An integer");
static int save_permission[100000][3];      // to save permission
static int save_type[100000];               // save types => code data .. stack
static int save_pagelength[100000];         // vm end - vm start  / page size
static unsigned long save_vmstart[100000];  // vm start
static unsigned long save_vmend[100000];    //vmend
static unsigned long save_pmstart[100000];  //pm start
static unsigned long save_pmend[100000];    //pm end

//struct task_struct 
unsigned static long last_update_time;
static struct timer_list timer;
static int stack_flag = 0;

static void print_bar(struct seq_file *s){
    int i = 0;
    for(i = 0; i< 80; i++){
        seq_printf(s,"-");
    }
    seq_printf(s,"\n");
}
char * fname = "";


//static unsigned long last_update_time;
static unsigned long long pgd_base;
static void __func(unsigned long temp){
    
}

DECLARE_TASKLET(tasklet, __func, 0);
static void func(struct timer_list *temp){
    
    struct task_struct *task;
    
     for_each_process(task){
             if(task->pid == pid){
                 if(task->mm ==NULL){
                     return;
                 }else{
                    struct vm_area_struct *current_vm = &task->mm->mmap[0];
                    static int index = 0;
                    for (index = 0;;index++){
                         char * rable = "no";
                         char * wable = "no";
                         char * eable = "no";
                        save_permission[index][0] = 0; // is no permission
                        save_permission[index][0] = 0;
                        save_permission[index][0] = 0;

                         char *type = "";
                         if (task->mm->start_code<=current_vm->vm_start && current_vm->vm_start<task->mm->start_data){// 0
                             save_type[index] = 0;
                         }else if (task->mm->start_data<=current_vm->vm_start && current_vm->vm_start< task->mm->end_data){ //1
                             save_type[index] = 1;
                         }else if (task->mm->end_data<=current_vm->vm_start && current_vm->vm_start<task->mm->start_brk){ //2
                             save_type[index] = 2;
                         }else if (task->mm->start_brk <= current_vm->vm_start && current_vm->vm_start < task->mm->brk){ //3
                             save_type[index] = 3;
                         }else if (current_vm->vm_start<= task->mm-> start_stack&& task->mm-> start_stack<=current_vm->vm_end){ //4
                             save_type[index] = 4;
                             stack_flag = 1;
                         }else if(task->mm->brk<= current_vm->vm_start && current_vm->vm_start < task->mm->start_stack){ //5
                             save_type[index] = 5;
                         }


                        save_pagelength[index] = (int)((current_vm->vm_end - current_vm->vm_start)/PAGE_SIZE);  // save page #
                        if(current_vm->vm_flags&VM_READ){
                             save_permission[index][0] = 1;
                             rable="yes";
                        }
                        if(current_vm->vm_flags&VM_WRITE){
                            save_permission[index][1] = 1;
                            wable = "yes";
                        }
                        if(current_vm->vm_flags&VM_EXEC){
                            save_permission[index][2] = 1;
                            eable = "yes";
                        }
                   
                        save_vmstart[index] = current_vm->vm_start; // save each period
                        save_vmend[index] = current_vm->vm_end;

                        pgd_t *pgd_a = pgd_offset(task->mm, current_vm->vm_start); // pgd ~ pte
                        p4d_t *p4d_a = p4d_offset(pgd_a, current_vm->vm_start);
                        pud_t *pud_a = pud_offset(p4d_a, current_vm->vm_start);
                        pmd_t *pmd_a = pmd_offset(pud_a, current_vm->vm_start);
                        pte_t *pte_a = pte_offset_kernel(pmd_a, current_vm->vm_start);
                        unsigned long pte_value = pte_val(*pte_a);

                        unsigned long pm_start =  ((pte_value & PAGE_MASK) | (current_vm->vm_start& ~PAGE_MASK)); // get pm
                        save_pmstart[index] = pm_start;
                        pgd_t *pgd_b = pgd_offset(task->mm, current_vm->vm_end); // pgd ~ pte
                        p4d_t *p4d_b = p4d_offset(pgd_b, current_vm->vm_end);
                        pud_t *pud_b = pud_offset(p4d_b, current_vm->vm_end);
                        pmd_t *pmd_b = pmd_offset(pud_b, current_vm->vm_end);
                        pte_t *pte_b = pte_offset_kernel(pmd_b, current_vm->vm_end);
                        unsigned long pte_value_end = pte_val(*pte_b);

                        unsigned long pm_end =  ((pte_value_end & PAGE_MASK) | (current_vm->vm_end& ~PAGE_MASK)); // get vm
                        save_pmend[index] = pm_end;
                        if(current_vm->vm_next == NULL){
                            count = index; // save count later to call
                            break;
                        }else{
                            current_vm = current_vm->vm_next;
                        }
                        if(stack_flag == 1){
                            stack_flag = 0;
                            count = index;
                            break;
                        }
                    }

                }
            }
        }


    last_update_time = ktime_to_ms(ktime_get_boottime()); // save time actually
    tasklet_schedule(&tasklet); // tasklet itself
    mod_timer(&timer, get_jiffies_64() + period * HZ); // from now on~
}


static void no_pid(struct seq_file *s){
    print_bar(s);
    seq_printf(s, "[System Programming Assignment 2]\n");
    seq_printf(s, "ID: 2015147506, Name: Kim, KiHyun\n");
    seq_printf(s, "There is no information - PID: %d\n", pid);
    print_bar(s);
}

static void yes_pid(struct seq_file *s, struct task_struct *task){
    
    print_bar(s); 
    seq_printf(s, "[System Programming Assignment 2]\n");
    seq_printf(s, "ID: 2015147506, Name: Kim, KiHyun\n");
    seq_printf(s, "Command: %s, PID: %d\n", task->comm, task->pid);
    print_bar(s);

}

static void yes_pid_2(struct seq_file *s, struct task_struct *task){    
    //seq_printf(s, "Last update time: %llu ms\n", last_update_time);
    seq_printf(s, "Page Size: %ld KB\n", PAGE_SIZE);
    seq_printf(s, "Last Updated Time; %lld ms \n", last_update_time);
    seq_printf(s, "PGD Base Address: 0x%08lx\n", task->mm->pgd);
    print_bar(s);
    int index = 0;
    char * typeo = "";
    for(index = 0; index <= count; index++){
        if(save_type[index]==0){ // if code
            typeo = "Code";
        }else if(save_type[index]==1){ // data
            typeo = "Data";
        }else if(save_type[index]==2){
            typeo = "BSS"; //bss
        }else if(save_type[index]==3){ //heap
            typeo = "Heap";
        }else if(save_type[index]==4){ // stack
            typeo = "Stack";
        }else if(save_type[index]==5){ // shared libraries
            typeo = "Shared Libraries";
        }
        seq_printf(s, "VM Area #%d - %s, (%d pages)\n", index, typeo,(int)(save_pagelength[index]));
        char * rable = "no";
        char * wable = "no";
        char * eable = "no";
        if(save_permission[index][0]==1){ // if true
            rable = "yes";
        }
        if(save_permission[index][1]==1){ // if true
            wable = "yes";
        }
        if(save_permission[index][2]==1){ // if true
            eable = "yes";
        }
        seq_printf(s, "Permissions: R(%s), W(%s), X(%s)\n", rable, wable, eable); // print permission

        seq_printf(s, "Virtual Memory: 0x%08lx - 0x%08lx\n", save_vmstart[index], save_vmend[index]); // vm range
        seq_printf(s, "Physical Memory: 0x%08lx - 0x%08lx\n", save_pmstart[index], save_pmend[index]); // corresponding pm
        print_bar(s);
    }

}

static void *sched_seq_start(struct seq_file *s, loff_t *pos)
{
    static unsigned long counter = 0;
    /* beginning a new sequence ? */
    if (*pos == 0)
    {
        /* yes => return a non null value to begin the sequence */
        return &counter;
    }
    else
    {
        /* no => it's the end of the sequence, return end to stop reading */
        *pos = 0;
        return NULL;
    }
}

static void *sched_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    return NULL;
}

static void sched_seq_stop(struct seq_file *s, void *v)
{

}


static int sched_seq_show(struct seq_file *s, void *v)
{
    struct task_struct *task;
    
    //seq_printf(s, "%ld\n", last_update_time);
    for_each_process(task){
            if(task->pid == pid){
                if(task->mm ==NULL){ // nothing? you get nothing!
                    flag = 0;
                }else{
                yes_pid(s,task); // no need to call global byunsu
                yes_pid_2(s,task); // call global byunsu
                flag = 1;
                break;
                }
            }else{
                flag = 0;
            }
    }
    if(flag == 0){
        no_pid(s);
    }

    return 0;
}

static struct seq_operations sched_seq_ops = {
    .start = sched_seq_start,
    .next = sched_seq_next,
    .stop = sched_seq_stop,
    .show = sched_seq_show
};

static int sched_proc_open(struct inode *inode, struct file *file) {
    return seq_open(file, &sched_seq_ops);
}

static const struct file_operations sched_file_ops = {
    .owner = THIS_MODULE,
    .open = sched_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};

static int __init sched_init(void) { //initialize file
    timer_setup(&timer, func, 0); // set time
    mod_timer(&timer, get_jiffies_64()); // start immidiately
    struct proc_dir_entry *proc_file_entry; // hw2 file
    proc_file_entry = proc_create(PROC_NAME, 0, NULL, &sched_file_ops);
    return 0;
}

static void __exit sched_exit(void) {
    del_timer(&timer);
    remove_proc_entry(PROC_NAME, NULL); // erase all directory
}




module_init(sched_init);
module_exit(sched_exit);