/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include "struct_module.h"

struct task_asynchrone *asynch2; /* si list est lancée en arrière plan, alors elle contiendra les infos de la tache */
static struct task_run *running; /* contiendra les info passées en paramètre de l'ioctl */

/*******************************************************
 * Fonction qui affiche les tâches en cours d'exécution 
 *******************************************************/
void print_list(struct work_struct *work)
{
	
	struct task_asynchrone *task_tmp; /* pour récuperer la structure qui contiendra le Work */
	struct list_taskRunning *list_tmp; /* pour parcourir ma liste des taches */
	char buffer[25]; /* tableau pour stocker chaque élement de la liste puis le concaténer avec task_Running */
	char buf_tmp [TAILLE_BUF]; /* contiendra le résultat de la commande */
	unsigned long restant; /* pour voir si le copy_to_user a bien fonctionné */
	int finish = 1; 
	
	mutex_lock(&mutex_list); /* synchronisation, si il y a plusieurs threads qui exécutent cette fonction */ 

	/* si je suis en asynchrone */
	if(work != NULL){
		task_tmp = task_asynch_of(work); /* je récupère ma structure contenant ma tache */
	}

	/* si ma liste est vide ou si elle est égale à null */
	if(list_empty(&(taskRunning -> list)) || taskRunning == NULL){
		pr_info ("Plus de tâche en cours!!\n");
		retval = PAS_DE_TACHE;

		if(work != NULL){
			/* je copie ce tableau dans l'espace utilisateur */
			restant = copy_to_user((char *) task_tmp -> data, "Plus de tâche en cours\n", strlen("Plus de tâche en cours\n"));
			if(restant != 0){
				pr_info ("Erreur copie_to_user\n");
			}
		
			/* je dis aussi que la tache est fini pour afficher le résultat */
			restant = copy_to_user((int  *)task_tmp -> finish, &finish, sizeof(int));
			if(restant != 0){
				pr_info ("Erreur copie_to_user\n");
			}
		}

		goto out_list;
	}
	
	/* je vide le tableau par précaution*/
	memset(buffer, 0, 25);
	memset(buf_tmp, 0, TAILLE_BUF);
	
	/* je parcours ma liste et je récupère les éléments */
	list_for_each_entry(list_tmp, &(taskRunning->list), list){
		sprintf(buffer, "ID : %d, TASK : %s\n", list_tmp->task.id, list_tmp->task.type_task);
		strcat(buf_tmp, buffer);
	}

	/* si je suis en asynchrone */
	if(work != NULL){
		memset(task_tmp -> adrEnvoie, 0, TAILLE_BUF); /* je vide le tableau */
		
		/* Dans le cas où je fais un fg juste après */
		strcpy(task_tmp -> adrEnvoie, buf_tmp);
		 	
		/* je copie ce tableau dans l'espace utilisateur */
		restant = copy_to_user((char *) task_tmp -> data, task_tmp -> adrEnvoie, strlen(task_tmp -> adrEnvoie));
		if(restant != 0){
			pr_info ("Erreur copie_to_user\n");
		}
		
		/* je dis aussi que la tache est finie pour afficher le résultat */
		restant = copy_to_user((int  *)task_tmp -> finish, &finish, sizeof(int));
		if(restant != 0){
			pr_info ("Erreur copie_to_user\n");
		}

		/* je supprime mon Work de la liste */ 
		remove_list(work);
		kfree(asynch2);

	}else{
	 	/* je suis en synchrone 
		   je vide le tableau qui stockera les éléments de ma liste */
		strcpy(running -> task_Running, "");
		strcpy(running -> task_Running, buf_tmp);
		running -> finish = 1; /* je dis que j'ai fini */
	}

	/* si j'ai été mise en avant plan, je reveille le processus qui l'a fait */
	if(strcmp(whoFG, "LIST") == 0){
	   strcpy(result_fg,buf_tmp);
		flag = 1;
		wake_up(&task_wait);
	}

  out_list:
	
	mutex_unlock(&mutex_list);
}

void list_handler(struct task_run *run)
{

	running = run;
	/* si je vais m'executer en asynchrone, je prépare mon insertion dans la Workqueue */
	if(running -> asyn == '&'){
		asynch2 = init_task_asynch("LIST",print_list);
		asynch2 -> data = running -> task_Running;
		asynch2 -> finish = &(running -> finish);

		schedule_delayed_work(&(asynch2 -> dwork),300); /* les délais permet de tester la commande fg par exemple */
		running -> pid = task_pid_nr(current);
		running -> id = asynch2 -> id;
	}else{
		/* je m'éxecute en mode synchrone */
		print_list(NULL);
	}
}


