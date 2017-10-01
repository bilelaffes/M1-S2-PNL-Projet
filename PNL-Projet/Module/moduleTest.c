/*3361270 AFFES BILEL
3601846 AKRAM BENTAMANSOURT*/

#include <linux/module.h>
#include <linux/init.h>



/*******************************************************************************
 * Parameters
 ******************************************************************************/
static int LED=1;
static int BIN=1;
static int STAT=1;
static int BUI=1;
static int ARTYUI=1;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BILEL, 2017");
MODULE_VERSION("version V0.1");
MODULE_DESCRIPTION("Module, aussitot insere, aussitot efface");

module_param(LED, int, 0000);
module_param(BIN, int, 0000);
module_param(STAT, int, 0000);
module_param(BUI, int, 0000);
module_param(ARTYUI, int, 0000);
/*******************************************************************************
 * Module Basics
 ******************************************************************************/
static int __init mon_module_init(void)
{
   printk(KERN_DEBUG "Hello Bilel !\n");
   printk(KERN_DEBUG "LED=%d !\n", LED);
	printk(KERN_DEBUG "BIN=%d !\n", BIN);
   return 0;
}

static void __exit mon_module_cleanup(void)
{
   printk(KERN_DEBUG "Goodbye Bilel !\n");
}

module_init(mon_module_init);
module_exit(mon_module_cleanup);

/* on pourra savoir qu'il a été lu avec dmesg, on remarque que le printk affiche LED=2 donc le paramètre a bien été lu */ 
