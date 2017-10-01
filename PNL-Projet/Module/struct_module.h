/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#ifndef STRCT_MODULE_H
#define STRCT_MODULE_H

#include <linux/dcache.h>
#include <linux/workqueue.h>
#include <linux/syscalls.h>
#include <linux/sysinfo.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/swap.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/syscall.h>
#include "../Outil/pnl_outil.h"

/***************************
 * allocation de memoire
 ***************************/
#define ALLOC(type, n) (type*) kmalloc(sizeof(type)*n, GFP_KERNEL)

/**************************************************
 * Structure qui correspondra à la tâche asynchrone
 **************************************************/
struct task_asynchrone{
	int id; /* l'ID de la tache */
	void *data; /* stockera le paramètre passée à la Worqueue */
	int *finish; /* aura l'adresse de la variable finish des structures */
	char type_task[7]; /* contindra le nom de la commande */
	struct delayed_work dwork; /* contiendra la fonction à executer */
	char adrEnvoie[TAILLE_BUF]; /* contindra l'adresse du buffer coté utilisateur */
	struct task_struct **task_tmp; /* contiendra le tableau des taches pour la commande Wait */
};

/***************************************
 * Liste des tâches en cours d'exécution
 ***************************************/
struct list_taskRunning{
	struct task_asynchrone task; /* la tache en cours d'execution */
	struct list_head list; /* la liste des taches */
};

/***************************************************
 * Variables globales partagées par tout les fichiers
 ***************************************************/
extern int id; /* variable qui sera incrémentée à chaque création d'une nouvelle tache, elle correspondra à son ID */
extern int flag; /* flag qui correspondra à la condition de sortie d'un wait_event */
extern int retval; /* valeur de retour de la fonction ioctl */
extern char whoFG[10]; /* contiendra le nom de la commande qui est mise en avant plan avec la commande fg */
extern struct mutex mutex_kill; /* mutex pour la fonction kill_process */
extern struct mutex mutex_wait; /* mutex pour la fonction wait_process */
extern struct mutex mutex_list; /* mutex pour la fonction print_list */
extern struct mutex mutex_remove; /* mutex pour la fonction remove_list */
extern struct mutex mutex_meminfo; /* mutex pour la fonction info_mem */
extern struct mutex mutex_modinfo; /* mutex pour la fonction info_mod */
extern char result_fg[TAILLE_BUF]; /* contiendra le résultat d'une commande asynchrone si elle a été mise en avant plan */
extern wait_queue_head_t task_wait; /* ma Waitqueue */
extern struct list_taskRunning *taskRunning; /* liste des taches en cours d'execution */

/************************************************
 * Fonctions d'initialisation des Works
 ************************************************/
int add_in_list(struct task_asynchrone *task); /* l'ajout d'une nouvelle tache dans la liste */
void remove_list(struct work_struct *work_test); /* suppression de la tache dans la liste */
struct task_asynchrone * init_task_asynch(char *type, void(*func)(struct work_struct *)); /* intialiser ma tache */


/******************************************
 * Fonctions exécutées par la Workqueue
 ******************************************/
void fg_cmd(struct work_struct *work);
void info_mem(struct work_struct *work);
void info_mod(struct work_struct *work);
void print_list(struct work_struct *work);
void wait_process(struct work_struct *work);
void kill_process(struct work_struct *work);


/**********************************************************
 * Handler appelé par le module dans le fonction d'ioctl
 **********************************************************/
void mem_handler(struct info_mem *me);
void list_handler(struct task_run *run);
void fg_handler(struct answer_fg *answer);
void mod_handler(struct info_module *mod);
void wait_handler(struct wait_task *wait_ta);
void kill_handler(struct envoie_signal *sign);

/****************************************************************
 * Fonction qui retrouve une structure a partir du champs dwork
 ****************************************************************/
struct task_asynchrone* task_asynch_of(struct work_struct *work_test);

#endif
