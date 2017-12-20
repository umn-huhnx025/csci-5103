#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

static char *charArg = "foo";
static int intArg = 25;

/* declare that intArg and charArg are args to the module and list their types
 * and permissions */
module_param(intArg, int, S_IRUGO);
module_param(charArg, charp, S_IRUGO);

/* module initialize function */
static int hello_init(void) {
  printk(KERN_INFO "HelloWorld: Hello, world\n");
  printk(KERN_INFO "HelloWorld: You passed: %d and %s\n", intArg, charArg);
  return 0;
}

/* module remove function */
static void hello_exit(void) {
  printk(KERN_INFO "HelloWorld: So long and thanks for all the fish..\n");
}

/* specify the module init and remove functions */
module_init(hello_init);
module_exit(hello_exit);
