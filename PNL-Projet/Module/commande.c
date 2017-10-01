/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include "struct_module.h"
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Axel-Bilel-Anas, Projet_PNL, SAR_UPMC");
MODULE_DESCRIPTION("Module, exec_commandes");

/***************************************************
 * Variables globales partagées par tous les fichiers
 ***************************************************/
int id; /* variable qui sera incrémentée à chaque création d'une nouvelle tache, elle correspondra à son ID */
int major; /* le numéro major du module lors de l'enregistrement */
int flag; /* flag qui correspondra à la condition de sortie d'un wait_event */
int retval; /* valeur de retour de la fonction ioctl */
char whoFG[10]; /* contiendra le nom de la commande qui est mise en avant plan avec la commande fg */
struct mutex mutex_kill; /* mutex pour la fonction kill_process */
struct mutex mutex_wait; /* mutex pour la fonction wait_process */
struct mutex mutex_list; /* mutex pour la fonction print_list */
struct mutex mutex_remove; /* mutex pour la fonction remove_list */
struct mutex mutex_meminfo; /* mutex pour la fonction info_mem */
struct mutex mutex_modinfo; /* mutex pour la fonction info_mod */
char result_fg[TAILLE_BUF]; /* contiendra le résultat d'une commande asynchrone si elle a été mis en avant plan */
wait_queue_head_t task_wait; /* ma Waitqueue */
struct list_taskRunning *taskRunning; /* list des taches en cours d'execution */

/****************************************************************
 * Fonction qui retrouve une structure a partir du champs dwork
 ****************************************************************/
struct task_asynchrone* task_asynch_of(struct work_struct *work_test)
{
	return (struct task_asynchrone*)((void*)work_test - (void*)&((struct task_asynchrone*)0)->dwork.work);
}

/*****************************************************
 * Fonction qui supprime une tâche finie de la liste 
 *****************************************************/
void remove_list(struct work_struct *work_test)
{
	struct list_taskRunning *list_tmp;
	struct task_asynchrone  *task_tmp;
	
	mutex_lock(&mutex_remove);
	/* je récupère la structure qui contient la tâche */
	task_tmp =  task_asynch_of(work_test);

	/* je parcours ma liste des tâches */
	list_for_each_entry(list_tmp, &(taskRunning->list), list){
		if(list_tmp->task.id == task_tmp-> id){
			/* si j'ai trouvé la tâche, je l'enleve de la liste et je libère son espace */
			list_del(&(list_tmp -> list));
			//kfree(list_tmp);
			//kfree(task_tmp);
			break;
		}
	}
  
	mutex_unlock(&mutex_remove);
}


/*************************************************
 * Fonction qui ajoute une nouvelle tâche à la liste
 *************************************************/
int add_in_list(struct task_asynchrone *task)
{
	struct list_taskRunning * list = ALLOC(struct list_taskRunning, 1);
       
	if(list == NULL){
		pr_info("Error Alloc\n");
		return -EINVAL;
	}

	list -> task = *task; 
        list_add_tail(&(list -> list),&(taskRunning -> list));
	return 0;
}




/*****************************************************
 * Fonction qui initialise ma struct task_asynchrone
 *****************************************************/
struct task_asynchrone * init_task_asynch(char *type, void(*func)(struct work_struct *))
{
	
	struct task_asynchrone * task = ALLOC(struct task_asynchrone, 1);
	
	if(task == NULL){
		pr_info("Error Alloc\n");
		return NULL;
	}
	
	
	task -> id = ++id; /* l'ID de la tache */
	task -> finish = 0; /* mettre finish à 0 car la tache va commencer */
	strcpy(task -> type_task, type); /* je recopie le nom de la commande qui sera utile pour la fonction fg */
	INIT_DELAYED_WORK(&(task -> dwork), func); /* j'initialise ma work_struct */
	/* j'ajoute dans la list des taches en cours */
	if(add_in_list(task) != 0){
		pr_info("Erreur ajout\n");
		return NULL;
	}
	return task;
}

/*******************************************************************************
 * Fonction IOCTL
 ******************************************************************************/
static long ioctl_cmd(struct file *file, unsigned int cmd, unsigned long arg) 
{
	
	retval = 0;
	flag = 0;

	switch(cmd){
	
	case LIST :
		list_handler((struct task_run *)arg);
		break;
	
	case FG :
		fg_handler((struct answer_fg*) arg);
		break;
	
	case KILL :
		kill_handler((struct envoie_signal *)arg);
		break;
	
	case WAIT :
		wait_handler((struct wait_task *)arg);
		break;
	
	case MEMINFO :
		mem_handler((struct info_mem*)arg);
		break;
	
	case MODINFO :
		mod_handler((struct info_module *)arg);
		break;
	
	default :
		retval = -EINVAL;
		break;
	}
	
	return retval;
}

struct file_operations var_ioctl =
{
	.unlocked_ioctl = ioctl_cmd,
};

/*******************************************************************************
 * Module Basics
 ******************************************************************************/
static int __init commande_init(void)
{
	/* j'alloue de l'espace pour ma liste des taches */
	taskRunning = ALLOC(struct list_taskRunning, 1);
	major = register_chrdev(0, "commande", &var_ioctl);
	pr_info("Major : %d !\n",major);

	/* j'initialise ma liste de taches, ainsi que ma Waitqueue et les mutex */
	INIT_LIST_HEAD(&(taskRunning -> list)); 
	init_waitqueue_head(&(task_wait));
	mutex_init(&mutex_meminfo);
	mutex_init(&mutex_modinfo);
	mutex_init(&mutex_remove);
	mutex_init(&mutex_list);
	mutex_init(&mutex_kill);
	mutex_init(&mutex_wait);
	return 0;
}

static void __exit commande_exit(void)
{
	unregister_chrdev(major, "commande");
	kfree(taskRunning);
	pr_info("Goodbye Axel-Bilel-Anas\n");
}

module_init(commande_init);
module_exit(commande_exit);
