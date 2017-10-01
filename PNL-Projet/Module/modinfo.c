/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include "struct_module.h"

char nomModule[512]; /* contiendra le nom du module */
struct task_asynchrone *asynch4; /* si modinfo est lancée en arrière plan, alors elle contiendra les infos de la tache */ 
static struct info_module *info_module; /* contiendra les info passées en paramètre de l'ioctl */


/*********************************************
 * Fonction qui envoit les infos d'un module 
 *********************************************/
void info_mod(struct work_struct *work)
{
	
	struct task_asynchrone *task_tmp; /* pour récuperer la structure qui contiendra le Work */
	struct info_module mod; /* on stockera les paramètres à l'interieur pour copier facilement après*/
	struct module *module; /* contindra le module demandé */
	unsigned long restant; /* pour voir si le copy_to_user a bien fonctionné */
	char buffer[TAILLE_BUF]; /* contiendra la valeur de chaque paramètre */
	char buf_tmp[TAILLE_BUF]; /* contiendra le résultat de la commande */
	unsigned int i=0;
	int finish = 1;

	mutex_lock(&mutex_modinfo);

	module = find_module(nomModule); /* je recherche le module */

	/* si je suis en asynchrone */
	if(work != NULL){
		task_tmp = task_asynch_of(work); /* je récupère ma structure contenant ma tache */
	}
	
	/* si il existe pas, donc introuvable */
	if((module == NULL)){
		retval = MODULE_INTROUVABLE;

		/* si je suis en asynchrone */
		if(work != NULL) {
			
			/* je copie ce tableau dans l'espace utilisateur */
			restant = copy_to_user((char  *)task_tmp -> data, "Module Introuvable\n", strlen("Module Introuvable\n")+1);
			if(restant != 0){
				pr_info ("Erreur copie_to_user\n");
			}

			/* je dis aussi que la tache est finie pour afficher le résultat */
			restant = copy_to_user((int  *)task_tmp -> finish, &finish, sizeof(int));
			if(restant != 0){
				pr_info ("Erreur copie_to_user\n");
			}

			/* si c'est en mode asynchrone, je supprime ma tache */
			remove_list(work);
			kfree(asynch4);
		}
		mutex_unlock(&mutex_modinfo);
		goto out;
	}

	
	/* je récupère les paramètres du module */
	memset(mod.param, 0, sizeof(mod.param));     
	for(i = 0; i < module -> num_kp; i++){
		module -> kp[i].ops -> get(buffer, &(module -> kp[i]));
		strcat(mod.param, module -> kp[i].name);
		strcat(mod.param, " = ");
		strcat(mod.param, buffer);
		strcat(mod.param, "  ");
	}

  
	/* si je suis en mode asynchrone */
	if(work != NULL){
		
		/* je vide le tableau */
		memset(buf_tmp, 0, TAILLE_BUF);
		
		/* je recopie les infos dans le tableau */
		scnprintf(buf_tmp, TAILLE_BUF, "Voici les info du module %s :\nname : %s\nversion: %s\nadresse: %p\nParamètres : %s",nomModule, module->name, module -> version, module, mod.param);
 
		/* je copie ce tableau dans l'espace utilisateur */
		restant = copy_to_user((char  *)task_tmp -> data, buf_tmp, strlen(buf_tmp));
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
		kfree(asynch4);
   
	}else{
		/* je suis en synchrone et je fais la meme chose que le mode asynchrone*/
		scnprintf(info_module -> buffer, TAILLE_BUF, "Voici les info du module %s :\nname : %s\nversion: %s\nadresse: %p\nParamètres : %s", nomModule, module->name, module -> version, module, mod.param);
		
		info_module -> finish = 1;     
	}

	/* je mets à jour le kref */
	if(module != NULL){
		module_put(module);
	}

  out:
	/* si j'ai été mise en avant plan, je réveille ce lui qui l'a fait */
	if(strcmp(whoFG, "MODINFO") == 0){
		strcpy(result_fg,buf_tmp);
		flag = 1;
		wake_up(&task_wait);
	}

	mutex_unlock(&mutex_modinfo);
}

void mod_handler(struct info_module *mod)
{
	
	info_module = mod;
	/* je recopie le nom du module, car utiliser directement arg créé des soucis en mode asynchrone */
	strcpy(nomModule, info_module -> arg);

	/* si c'est en asynchrone, je prépare mon work qui sera inseré dans la Workqueue */
	if(info_module -> asyn == '&'){
		asynch4 = init_task_asynch("MODINFO",info_mod);
		asynch4 -> data = info_module -> buffer;
		asynch4 -> finish = &(info_module -> finish);

		schedule_delayed_work(&(asynch4 -> dwork),500); /* faite varier la valeur si vous voulez tester fg par exemple */
		info_module -> pid = task_pid_nr(current);
		info_module -> id = asynch4 -> id;
	}else{
		/* je me lance en synchrone */
		info_mod(NULL);
	}
}



