#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/pid.h>

int smunch(int pid, unsigned long bit_pattern) {
	struct task_struct *t;
	unsigned long flags;

	rcu_read_lock();
	t = find_task_by_vpid(pid);
	rcu_read_unlock();
	printk(KERN_EMERG "got the struct\n");
	
	if(!t || !lock_task_sighand(t, &flags)) {
		printk(KERN_EMERG "invalid struct or lock isn't acquired\n");
		return -1;
	}

	if(!thread_group_empty(t)) {
		printk(KERN_EMERG "multithreaded\n");
		unlock_task_sighand(t, &flags);
		return -1;
	}
	//if exit state is zombie or dead and bit pattern is set - (1 1)
	if((t->exit_state & (EXIT_ZOMBIE | EXIT_DEAD)) && bit_pattern & (1<<(SIGKILL-1)) ) {
		printk(KERN_EMERG "zombie and kill bit set\n");
		unlock_task_sighand(t, &flags);
		release_task(t);
		return 0;
	}
	else {	//only kill bit is set - (0 1)
		if(bit_pattern & (1<<(SIGKILL-1))){
			printk(KERN_EMERG "only kill bit set\n");
			sigaddset(&t->signal->shared_pending.signal, SIGKILL);
			wake_up_process(t);
		} 	//if any other state load bit pattern and wake the target process unconditionally - (0 0)
		else if(!(t->exit_state & (EXIT_ZOMBIE | EXIT_DEAD))) {		
			printk(KERN_EMERG "not zombie and not kill bit\n");
			t->signal->shared_pending.signal.sig[0] = bit_pattern;
			set_tsk_thread_flag(t, TIF_SIGPENDING);
			wake_up_process(t);
		}
		else {	//exit state is zombie and kill bit not set
			printk(KERN_EMERG "zombie but not kill bit\n");
		}
		unlock_task_sighand(t, &flags);
		return 0;
	}
}
