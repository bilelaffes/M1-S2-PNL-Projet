/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include "struct_module.h"

struct task_asynchrone *asynch3; /* si meminfo est lancée en arrière plan, alors elle contiendra les infos de la tache */ 
static struct info_mem *memoire; /* contiendra les info passées en paramètre de l'ioctl */

/***********************************************
 * Fonction qui envoit les info sur la mémoire
 ***********************************************/
void info_mem(struct work_struct *work)
{
	
	struct task_asynchrone *task_tmp; /* pour récuperer la structure qui contiendra le Work */
	static struct sysinfo info; /* contiendra les infos de la mémoire */
	unsigned long restant; /* pour voir si le copy_to_user s'est bien déroulé */
	char buf_tmp[TAILLE_BUF]; /* contiendra le résultat de la commande */
	int finish = 1;
	
	mutex_lock(&mutex_meminfo);

	/* je récupère les infos de la mémoire */
	si_meminfo(&info); 
	si_swapinfo(&info);

	/* si je suis en asynchrone */
	if(work != NULL){
		task_tmp = task_asynch_of(work);
		memset(buf_tmp, 0, TAILLE_BUF); /* je vide le tableau */

		/* je recopie les infos dans le tableau */
		scnprintf(buf_tmp, TAILLE_BUF, "Voici l'état de la mémoire :\nTotal Ram : %lu ko\nTotal Swap : %lu ko\nTotal High : %luko\nFree Ram : %lu ko\nFree Swap : %lu ko\nFree High : %lu ko\nShred Ram : %lu ko\nBuffer Ram : %lu ko\nMem Unit : %lu ko\n", info.totalram, info.totalswap, info.totalhigh, info.freeram, info.freeswap, info.freehigh, (unsigned long)info.sharedram, (unsigned long)info.bufferram, (unsigned long)info.mem_unit);

		/* je copie ce tableau dans l'espace utilisateur */
		restant = copy_to_user((char *) task_tmp -> data, buf_tmp, strlen(buf_tmp));
		if(restant != 0){
			pr_info ("Erreur copie_to_user\n");
		}
		
		/* je dis aussi que la tache est finie pour afficher le résultat */
		restant = copy_to_user((int  *)task_tmp -> finish, &finish, sizeof(int));
		if(restant != 0){
			pr_info ("Erreur copie_to_user\n");
		}

		/* je supprime ma tache de la liste */
		remove_list(work);
		kfree(asynch3);

	}else{
		/* je suis en synchrone et je fais la meme chose que le mode asynchrone*/
		memset(memoire -> buffer, 0, TAILLE_BUF);

		scnprintf(memoire -> buffer, TAILLE_BUF, "Voici l'état de la mémoire :\nTotal Ram : %lu ko\nTotal Swap : %lu ko\nTotal High : %luko\nFree Ram : %lu ko\nFree Swap : %lu ko\nFree High : %lu ko\nShred Ram : %lu ko\nBuffer Ram : %lu ko\nMem Unit : %lu ko", info.totalram, info.totalswap, info.totalhigh, info.freeram, info.freeswap, info.freehigh, (unsigned long)info.sharedram, (unsigned long)info.bufferram, (unsigned long)info.mem_unit);
		
		memoire -> finish = 1;
	}

	/* si j'ai été mise en avant plan, je réveille ce lui qui l'a fait */
	if(strcmp(whoFG, "MEMINFO") == 0){
		strcpy(result_fg,buf_tmp); 
		flag = 1;
		wake_up(&task_wait);
	}

	mutex_unlock(&mutex_meminfo);
}

void mem_handler(struct info_mem *mem)
{

	memoire = mem;
	/* si c'est en asynchrone, je prépare mon work qui sera inseré dans la Workqueue */
	if(memoire->asyn == '&'){
		asynch3 = init_task_asynch("MEMINFO",info_mem);
		asynch3 -> data = memoire -> buffer;
		asynch3 -> finish = &(memoire -> finish);

		schedule_delayed_work(&(asynch3 -> dwork),600); /* faite varier la valeur si vous voulez tester fg par exemple */
		
		memoire -> pid = task_pid_nr(current);
		memoire -> id = asynch3 -> id;
	}else{
		/* je me lance en synchrone */
		info_mem(NULL);
	}
	
}


