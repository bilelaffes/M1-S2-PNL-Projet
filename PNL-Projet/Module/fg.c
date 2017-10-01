/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include "struct_module.h"

struct answer_fg *answ;

/*************************************************
 * Fonction qui remet une commande en avant plan 
 *************************************************/
void fg_cmd(struct work_struct *work)
{
         
	struct list_taskRunning *list_tmp; /* pour rechercher dans ma liste de taches */
	unsigned long restant; /* pour voir si le copy_to_user a bien fonctionné */
	int trouver = 0; /* si j'ai trouvé la tache */
	
	/* si la liste est vide ou si la liste est NULL alors return; */
	if(list_empty(&(taskRunning -> list)) || taskRunning == NULL){
		pr_info ("Plus de tâche à mettre en avant plan !!\n");
		retval = PAS_DE_TACHE;
		return;
	}

	/* je parcours ma liste pour rechercher la tache, si je la trouve, alors je fais un wait_event pour la 
           mettre en avant plan, puis je récupère son résultat pour l'afficher */
	list_for_each_entry(list_tmp, &(taskRunning->list), list){
		if(list_tmp->task.id == answ -> id){
			trouver = 1;
			strcpy(whoFG, list_tmp->task.type_task); /* je copie le nom de la commande dans la variable */
			wait_event(task_wait, flag != 0); /* je mets la commande en avant plan */
			strcpy(whoFG, ""); /* j'efface ce qu'il y avait dans la variable */
			flag = 0; /* je remets le flag à 0 pour pouvoir refaire un wait_event une autre fois */
			
			memset(answ -> buffer, 0, TAILLE_BUF);
			restant = copy_to_user((char *)(answ -> buffer), result_fg, strlen(result_fg));
			if(restant != 0){
				pr_info ("Erreur copie_to_user\n");
			}
			
			strcpy(result_fg,""); /* j'efface ce qu'il y avait dans la variable */	
			break;
		}
	}
	if(!trouver){
		retval = ID_INEXISTANT;
	}
}

void fg_handler(struct answer_fg *answer)
{
	answ = answer;
	fg_cmd(NULL);
}



