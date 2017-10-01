/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include "struct_module.h"

int size; /* la taille de mon tableau de taches */
int pid_tache; /* stockera le pid de la tâche terminé */
int code_retour; /* stockera la valeur de retour de la tâche terminé */
struct task_asynchrone *asynch; /* contiendra la tache pour attendre la terminaison d'autres taches */ 


/*********************************************************************
 * Fonction qui attend la terminaison d'un processus dans le tableau 
 *********************************************************************/
void wait_process(struct work_struct *work)
{
	struct task_asynchrone *task_tmp; /* variable qui contiendra ma structre correspondante à mon Work */
	struct task_struct *task; /* variable temporaire */

	int i=0;
	
	task_tmp = task_asynch_of(work); /* je récupère ma structure task_asynchrone */
	for (i = 0; i < size -1; i++) { /* je parcours mon tableau task_tmp qui contiendra les taches dont on doit attendre leur terminaison */

		task = (task_tmp -> task_tmp)[i]; /* je récupère ma tache i */
		if (task != NULL) {
			task_lock(task);
			/* si elle n'est plus en vie, elle est donc terminée, je réveille le processus mis en attente */
			if (!pid_alive(task)) {
				pr_info("Process pid %d finished.", task -> pid);
				task_unlock(task);
				pid_tache = task -> pid;
				code_retour = task -> exit_code;
				put_task_struct(task);
				flag = 1;
				wake_up(&task_wait);
				remove_list(work); /* je supprime cette tache de la liste des taches en cours */
				return;
			} else {
				/* elle est encore en vie */
				task_unlock(task);
			}
		}
	}
	
	/* si je suis sorti de la boucle et qu'aucune tache ne s'est terminées, alors je ré-éxecute la fonction */
	schedule_delayed_work(&(task_tmp -> dwork),10);
}

/*********************************************************************
 * Fonction qui attend la terminaison d'un processus dans le tableau 
 *********************************************************************/
void wait_handler(struct wait_task *wait_ta)
{
	
	int i=0;
	int found = 0, pid;
	struct pid *str_pid;
	struct task_struct *task[wait_ta -> wait_size]; /* tableau des taches à attendre */
	struct task_struct *task_tmp;
	
	size = wait_ta -> wait_size; /* la taille du tableau */
	
	for (i = 0; i < size -1; i++) { /* je parcours mon tableau de PID */
		pid = wait_ta -> tabPid[i];
		pr_info("PID : %d\n", pid);
		
		/* je récupère la struct pid correspondante au pid */
		str_pid = find_get_pid(pid);
		if (str_pid > 0) {
			/* je récupère sa tache et je l'insère dans le tableau des taches */
			task_tmp = get_pid_task(str_pid, PIDTYPE_PID);
			if (!task_tmp) {
				pr_err("can't find task for pid %d\n", pid);
			} else {
				found = 1;
				task[i] = task_tmp;
				put_task_struct(task[i]); /* mettre à jour les kref */
			}
			put_pid(str_pid); /* mettre à jour les kref */
		} else {
			pr_err("can't find struct pid for pid %d\n", pid);
		}
	}

	if (found) {
		/* si j'en ait trouvé au moins un alors je lance ma Work pour l'attente */ 
		asynch = init_task_asynch("WAIT",wait_process);
		asynch -> task_tmp = task;
		schedule_delayed_work(&(asynch -> dwork),10);
		wait_event(task_wait, flag != 0);
		wait_ta -> pid = pid_tache;
		wait_ta -> val_retour = code_retour;
		flag = 0;
		kfree(asynch);
	} else {
		pr_info("Il n ya aucun PID \n");
	}
}

