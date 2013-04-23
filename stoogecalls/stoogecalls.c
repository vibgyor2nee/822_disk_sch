#include <linux/linkage.h>
#include <linux/kernel.h>

int goober(int arg1) {
	printk(KERN_ALERT "Hello from goober!");
	printk(KERN_ALERT "Hello! %d \n", arg1);
	return(1);

}
