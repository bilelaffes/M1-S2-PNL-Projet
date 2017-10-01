/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pnl_outil.h"

/** Affiche tabCmd sur le terminal
 * @param tabCmd : le tableau contenant la commande
 * @param size : sa taille
 */
void print_tabCmd(char **tabCmd, int size)
{
	int i;
	
	for(i = 0; i < size; i++){
		printf("[DEBUG][OUTIL] tabCmd[%d] = %s\n", i, tabCmd[i]);
	}
}

/** Reset tabCmd (met ses cases a "")
 * @param tabCmd : le tableau contenant la commande
 * @param size : sa taille
 */
void clear_tabCmd(char **tabCmd, int size)
{
	int i;
	
	for(i = 0; i < size; i++){
		tabCmd[i] = "";
	}
}

int main(int argc, char const *argv[])
{
	char *token; /* il pointera vers la chaine à découper */
	char cmd[TAILLE_CMD]; /* contindra la commande envoyé de l'utilisateur en stdin */
	char *tabCmd[TAILLE_BUFFER]; /* coontiendra la commande découper */
	
	const char separateur[2] = " "; 

	int i, j=0, id, size = 0;
	
	/* variables qui seront envoyées au module et contiendront les réponses */
	struct task_run running; 
	struct info_module info;
	struct info_mem memoire;
	struct wait_task wait_task;
	struct envoie_signal signal;
	struct answer_fg answer;

	/* je mets finish à 0 pour qu'il ne rentre pas dans les if à la fin et j'initialise la buffer du signal*/
	running.finish = 0; memoire.finish = 0; info.finish = 0; memset(signal.buffer, 0, TAILLE_BUF);
	/* variables qui contiendront les codes d'erreur si y en a */
	int err_list =0, err_kill=0, err_meminfo=0, err_modinfo=0, err_fg=0;
	/* j'ouvre mon fichier qui me permettera de communiquer avec mon module */
	int fd_ioctl = open("/dev/commande", O_RDONLY, 0660);
	
	// Code de l'outil
	printf("\n");
	printf("  *****************************************************\n");
	printf("  ********  Menu de manipulation de l'outil  **********\n");
	printf("  *****************************************************\n");
		
	printf("0-quit\n");
	printf("1-list\n");
	printf("2-fg <id>\n");
	printf("3-kill <signal> <pid>\n");
	printf("4-wait <pid> [<pid>....]\n");
	printf("5-meminfo\n");
	printf("6-modinfo <name Module>\n");
	printf("7- result ( pour le resultat des commandes asynchrones) \n");
	printf("\n");
     
	do{
		printf("Veuillez rentrer une commande <cmd> <arg ... > <espace> (& si asynchrone) :\n");
		clear_tabCmd(tabCmd, size);
		fgets(cmd, TAILLE_CMD, stdin);
		
		// On decoupe la commande pour recuperer ses "arguments"
		i = 0;
		token = strtok(cmd, separateur);
		while(token != NULL){
			if(i == TAILLE_BUFFER){
				printf("[ERREUR][OUTIL] Une commande ne peut pas avoir plus de %d arguments\n", TAILLE_BUFFER);
				return -1;
			}
 			token[strcspn(token, "\n")] = 0;
			tabCmd[i] = token;
			i++;
			token = strtok(NULL, separateur);
		}
		size = i;

		if(strcmp(tabCmd[0], "list") == 0){
			printf("Je lance la commande LIST\n");
			if(i == 2 && strcmp(tabCmd[1], "&") == 0){		
				running.asyn = '&';
				err_list = ioctl(fd_ioctl, LIST, &running);
				printf("ID [%d]\n", running.id);
				
			}else{
				running.asyn = ' ';
				err_list = ioctl(fd_ioctl, LIST, &running);
				goto finish;
			}				
		
		}else if(strcmp(tabCmd[0], "fg") == 0){
			
			/* la commande fg vu qu'elle bloque le terminal, n'a pas besoin d'etre envoyer en mode asynchrone */
			answer.id = atoi(tabCmd[1]);
			printf("Je lance la Commande : %s %d\n", tabCmd[0], answer.id);
			err_fg = ioctl(fd_ioctl,FG,&answer);
			printf("%s\n", answer.buffer);
			running.finish = 0; 
			memoire.finish = 0; 
			info.finish = 0;
			goto finish;

		}else if(strcmp(tabCmd[0], "kill") == 0){
			
			signal.sig  = atoi(tabCmd[1]);
			signal.pid  = atoi(tabCmd[2]);

			if(signal.pid == 1 || signal.pid == getpid()){
				printf ("Ah tu ne peux pas me tuer, ni tuer ton initiateur\n");
				continue;
			}
			
			printf("Je lance la  Commande : %s %d %d\n", tabCmd[0], signal.sig, signal.pid);
			if(i == 4 && strcmp(tabCmd[3], "&") == 0){
				signal.asyn = '&';
				err_kill = ioctl(fd_ioctl,KILL,&signal);
				printf("ID [%d]\n", signal.id);
			
			}else{
				signal.asyn = ' ';
				err_kill = ioctl(fd_ioctl,KILL,&signal);
				goto finish;
			}
			

		}else if(strcmp(tabCmd[0], "wait") == 0){
			
			// On recupere les pid des processus a attendre
			for(i = 1; i < size; i++){
				wait_task.tabPid[j] = atoi(tabCmd[i]);
				j++;
			}
			wait_task.wait_size = j;			
			printf("je lance la Commande : %s", tabCmd[0]);
			
			for(i = 0; i < wait_task.wait_size; i++){
				printf(" %d", wait_task.tabPid[i]);
			}
			printf("\n");
			wait_task.wait_size = size;
			ioctl(fd_ioctl,WAIT,&wait_task);
			printf("Tâche %d est fini avec comme valeur de retour %d\n", wait_task.pid, wait_task.val_retour); 

		}else if(strcmp(tabCmd[0], "meminfo") == 0){
			
			printf("je lance la Commande : %s\n", tabCmd[0]);
			if(i == 2 && strcmp(tabCmd[1], "&") == 0){
				memoire.asyn = '&';
				err_meminfo = ioctl(fd_ioctl,MEMINFO,&memoire);
				printf("ID [%d]\n", memoire.id);
			
			}else{
				memoire.asyn = ' ';
				err_meminfo = ioctl(fd_ioctl,MEMINFO,&memoire);
				goto finish;
			
			}

			
		}else if(strcmp(tabCmd[0], "modinfo") == 0){
			
			printf("Je lance la  Commande : %s %s\n", tabCmd[0], tabCmd[1]);
			strcpy(info.arg, tabCmd[1]);

			if(i == 3  && strcmp(tabCmd[2], "&") == 0){
				info.asyn = '&';
				err_modinfo = ioctl(fd_ioctl, MODINFO, &info);
				printf("ID [%d]\n", info.id);
			
			}else{
				info.asyn = ' ';
				err_modinfo = ioctl(fd_ioctl, MODINFO, &info);
				goto finish;
			
			}

		}else if(strcmp(tabCmd[0], "result") == 0){
			/* permettera de connaitre les résultats des commandes asynchrones */
			goto finish;
		}
	
	  finish:

		/* affiche les erreurs des commandes ou leurs résultats */
		printf("%s", signal.buffer);
		memset(signal.buffer, 0, TAILLE_BUF);

		if(err_kill == ERREUR_KILL){
			err_kill = 0;
			printf("Erreur KILL PID\n");
			continue;
		
		}else if(err_kill == PID_INEXISTANT){
			err_kill = 0;
			printf("PID Inexistant\n");
			continue;
		}

		if(err_fg == ID_INEXISTANT){
			printf("id Inexistant\n");
			err_fg = 0;
			continue;
		
		}else if(err_fg == PAS_DE_TACHE){
			printf("Plus de tâche a mettre en avant plan\n");
			err_fg = 0;
			continue;
		}

		if(err_list == PAS_DE_TACHE){	
			printf("Plus de tâche Running\n");
			err_list = 0;
			continue; 
		}
   
		if(err_meminfo){
			printf("Erreur mem_info\n");
			err_meminfo = 0;
			continue;
		}
  
		if(err_modinfo == MODULE_INTROUVABLE){
			err_modinfo = 0;
			printf("Module Introuvable\n");
			continue;
		}
 	
			
		if(running.finish == 1){
               
			running.finish = 0;
			printf("Voici la list des commandes en cours d'exécution : \n");
			printf("%s\n", running.task_Running);
		}
				

		if(memoire.finish == 1){
            
			memoire.finish = 0;
			printf("%s\n", memoire.buffer);
		}

		if(info.finish == 1){

			info.finish = 0;
			printf("%s\n", info.buffer);
		}

	}while(strcmp(tabCmd[0], "quit") != 0);

	printf("Au revoir :)\n");
	return 0;
}
