#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/cpumask.h>
#include <linux/string.h>

#define PROC_NAME "hw1"
#define NM 1000000
MODULE_AUTHOR("Kim, Kihyun");
MODULE_LICENSE("GPL");

char * fname = "";
int imsi = 0;
extern struct task_struct* prev_task[][10];
extern unsigned long long per_cpu_time[][10];
static void print_bar(struct seq_file *s){
    int i = 0;
    for(i = 0; i< 40; i++){
        seq_printf(s,"-");
    }
    seq_printf(s,"\n");
}

static void print_task(struct seq_file *s, struct task_struct *task, unsigned long long pct){ // get task
        if(task -> static_prio >= 100){ // CFS
            seq_printf(s, "%16s %5d %12lld CFS\n", task->comm, task->pid, pct/NM); // name, pid
        }else{  // RT
            seq_printf(s, "%16s %5d %12lld  RT\n", task->comm, task->pid, pct/NM);
        }
}


static void print_pid(struct seq_file *s, struct task_struct *task){
    print_bar(s);
    seq_printf(s, "System Programming Assignment 1]\n");
    seq_printf(s, "ID: 2015147506, Name: Kim, KiHyun\n");
    print_bar(s);
    seq_printf(s, "Command: %s\n", task->comm);
    seq_printf(s, "Type: ");
    if(task -> static_prio >= 100){ // CFS
        seq_printf(s, "CFS\n");
    }else{  // RT
        seq_printf(s, "RT\n");
    }
    seq_printf(s, "PID: %d\n", task->pid); //pid
    seq_printf(s, "Start time: %lld (ms)\n", task ->start_time/NM);
    seq_printf(s, "Last Scheduled Time: %lld (ms)\n", task -> last_sched_time/NM);// get from core.c
    seq_printf(s, "Last CPU #: %d\n", task ->recent_used_cpu); // recent cpu
    seq_printf(s, "Priority: %d\n", task -> static_prio); //pid
    unsigned long long tEt =0;
    int cpu;
    for_each_online_cpu(cpu){
        tEt += task->cpu_exec_time[cpu]/NM;  // sum up all exec time
    }
    seq_printf(s, "Total Execution time: %lld (ms)\n", tEt);
    for_each_possible_cpu(cpu);
    for_each_online_cpu(cpu){
        seq_printf(s, "- CPU %d: %lld(ms)\n", cpu, (task->cpu_exec_time[cpu]/NM)); // each exec time
    }
    if(task -> static_prio >= 100){ // CFS
        seq_printf(s, "Weight: %ld\n", task->se.load.weight); //weight
        seq_printf(s, "Virtual Runtime: %lld\n", task->se.vruntime); //runtime
    }else{  // RT
        
    }
}


/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 * − the /proc file is read (first time)
 * − after the function stop (end of sequence)
 *
 */
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

/**
 * This function is called after the beginning of a sequence.
 * It's called untill the return is NULL (this ends the sequence).
 *
 */
static void *sched_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    //unsigned long *tmp_v = (unsigned long *)v;
    //(*tmp_v)++;
    (*pos)++;
    return NULL;
}

/**
 * This function is called at the end of a sequence
 *
 */
static void sched_seq_stop(struct seq_file *s, void *v)
{
    /* nothing to do, we use a static value in start() */
}


/**
 * This function is called for each "step" of a sequence
 *
 */


static int sched_seq_show(struct seq_file *s, void *v)
{

    int count = 0;
    int c = 0;
    struct task_struct *task;
    for_each_process(task){
        count++;
        if(task->state == 0 ||task->state == 1||task->state == 2){ // running? blocked?
            c++;
        }
    }
    //seq_printf(s, "Total %d tasks\n", c);
    if(strcmp("top", fname)==0){
   
        //loff_t *spos = (loff_t *) v;
        print_bar(s);
        seq_printf(s, "System Programming Assignment 1]\n");
        seq_printf(s, "ID: 2015147506, Name: Kim, KiHyun\n");
        seq_printf(s, "Total %d tasks, %dHz\n", c, HZ);
        print_bar(s);
        int cpu;
        static int max_cpu = 0;
        for_each_online_cpu(cpu){
            if(cpu > max_cpu)max_cpu = cpu;
        }
        //seq_printf(s,"%d",max_cpu);
        //print_bar(s);
        for_each_online_cpu(cpu){
            seq_printf(s, "CPU %d\n", cpu);
            print_bar(s);
            print_task(s, prev_task[cpu][0], per_cpu_time[cpu][0]);
            print_task(s, prev_task[cpu][1], per_cpu_time[cpu][1]);
            print_task(s, prev_task[cpu][2], per_cpu_time[cpu][2]);
            print_task(s, prev_task[cpu][3], per_cpu_time[cpu][3]);
            print_task(s, prev_task[cpu][4], per_cpu_time[cpu][4]);
            print_task(s, prev_task[cpu][5], per_cpu_time[cpu][5]);
            print_task(s, prev_task[cpu][6], per_cpu_time[cpu][6]);
            print_task(s, prev_task[cpu][7], per_cpu_time[cpu][7]);
            print_task(s, prev_task[cpu][8], per_cpu_time[cpu][8]);
            print_task(s, prev_task[cpu][9], per_cpu_time[cpu][9]);
            print_bar(s);
            
        }
    }else{
        struct task_struct *t;
        int flag = 0;
        for_each_process(t){
            int pid;
            
            char pname[50] = "";
            sprintf(pname, "%d",t -> pid);
            if(strcmp(pname, fname) == 0){ // match available pid?
                flag = 1;
                break;
            }
            
        }
        if(flag == 1){
            print_pid(s,t);
        }

    }
    /////////////////////////
    //seq_printf(s, "%s", fname);

    for_each_process(task){
        //seq_printf(s, "pid: %d\n", task->pid);
    }


////////////////////////////


    return 0;
}

static struct seq_operations sched_seq_ops = {
    .start = sched_seq_start,
    .next = sched_seq_next,
    .stop = sched_seq_stop,
    .show = sched_seq_show
};

static int sched_proc_open(struct inode *inode, struct file *file) {
    fname = file->f_path.dentry->d_name.name;
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
    struct proc_dir_entry *proc_directory; // directory
    proc_directory = proc_mkdir("hw1",NULL); 
    struct proc_dir_entry *proc_file_entry; // top file
    proc_file_entry = proc_create("top", 0, proc_directory, &sched_file_ops);
    
    
    struct proc_dir_entry *proc_pid[32768]; // create all pid file possible does not ensure availability
    int i;
    for(i = 0; i<32768;i++){ // pid possible #
        char str[50] = "";
        sprintf(str, "%d",i); // change to string
        proc_pid[i] = proc_create(str, 0, proc_directory, &sched_file_ops); // create
    }
    return 0; 
}

static void __exit sched_exit(void) {
    remove_proc_entry(PROC_NAME, NULL); // erase all directory
}

module_init(sched_init);
module_exit(sched_exit);
