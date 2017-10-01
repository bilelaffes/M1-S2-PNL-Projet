/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include "struct_module.h"


int pid, sig; /* stockera le PID et le numéro de signal envoyé */
struct pid *p; /* contiendra la struct pid correspondant au pid */
struct task_asynchrone *asynch1; /* si kill est lancée en arrière plan, alors elle contiendra les infos de la tache */ 
static struct envoie_signal *signal; /* contiendra les info passées en paramètre de l'ioctl */


/*****************************************************************
 * Fonction qui envoit un signal au processus avec comme pid pid 
 *****************************************************************/
void kill_process(struct work_struct *work)
{

	struct task_asynchrone *task_tmp; /* pour récuperer la structure qui contiendra le Work */
	unsigned long restant; /* pour voir si le copy_to_user a bien fonctionné */
	char buf_tmp[TAILLE_BUF]; /* contiendra le résultat de la commande */

	mutex_lock(&mutex_kill);
	/* si j'essaye de tuer le module */
	if(pid == task_pid_nr(current)){
		pr_info ("Ah tu ne peux pas me tuer !!!\n");
		return;
	}

	/* si je suis en asynchrone */
	if(work != NULL){
		task_tmp = task_asynch_of(work); /* je récupère ma structure contenant ma tache */
	}

	p = find_get_pid(pid); /* je récupère la struct pid de mon pid */
	/* si le pid est bon et que la struct n'est pas null */
	if(p){
		/* si le kill ne se passe pas bien, alors Erreur */
		if(kill_pid(p, sig, 0) == -ESRCH){
			pr_info("Erreur Kill PID\n");
			retval = ERREUR_KILL;
			strcpy(buf_tmp, "Erreur Kill PID\n");
			
			if(work != NULL){
				/* je copie l'erreur dans l'espace utilisateur */
				restant = copy_to_user((char  *)task_tmp -> data, "Erreur Kill PID\n", strlen("Erreur Kill PID\n"));
				
				if(restant != 0){
					pr_info ("Erreur copie_to_user\n");
			   }
				goto out_kill;
			}
		}
	}else{
		/* le pid est inexistant */
		pr_info("PID inexistant\n");
		retval = PID_INEXISTANT;
		strcpy(buf_tmp, "PID inexistant\n");
		
		if(work != NULL){
			/* je copie l'erreur dans l'espace utilisateur */
			restant = copy_to_user((char  *)task_tmp -> data, "PID inexistant\n", strlen("PID inexistant\n"));
			
			if(restant != 0){
				pr_info ("Erreur copie_to_user\n");
			}
			goto out_kill;
		}
	}
  
  out_kill:
	/* si j'ai été mise en avant plan, je réveille celui qui l'a fait */
	if(strcmp(whoFG, "KILL") == 0){
		strcpy(result_fg,buf_tmp);
		flag = 1;
		wake_up(&task_wait);
	}

	/* je supprime la tache de la liste */
	if(work != NULL){
		remove_list(work);
		kfree(asynch1);
	}
	
	mutex_unlock(&mutex_kill);
}

void kill_handler(struct envoie_signal *sign)
{
	
	signal = sign;
	pid = signal -> pid;
	sig = signal -> sig;

	/* si je dois me lancer en asynchrone, j'initialise le Work */
	if(signal -> asyn == '&'){
		asynch1 = init_task_asynch("KILL",kill_process);
		asynch1 -> data = signal -> buffer;
		
		schedule_delayed_work(&(asynch1 -> dwork),300); /* les délais permet de tester la commande fg par exemple */
		
		signal -> pid_return = task_pid_nr(current);
		signal -> id = asynch1 -> id;
	}else{
		/* je me lance en synchrone */
		kill_process(NULL);
	}
}


