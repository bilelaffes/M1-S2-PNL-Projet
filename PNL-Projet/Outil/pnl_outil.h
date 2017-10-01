/* Auteur :
	Axel ARCHAMBAULT            3300807
	Anas EZOUHRI                3208760
 	Bilel AFFES                 3361270 */

#ifndef PNL_OUTIL_H_
#define PNL_OUTIL_H_
#include <linux/ioctl.h>

#define LIST    _IOR('N', 40, int)
#define FG      _IOR('N', 44, struct answer_fg)
#define KILL    _IOR('N', 46, int)
#define WAIT    _IOR('N', 48, int)
#define MEMINFO _IOR('N', 50, struct info_mem)
#define MODINFO _IOR('N', 52, struct info_module)

#define PAS_DE_TACHE 30
#define ID_INEXISTANT 31
#define MODULE_INTROUVABLE 32
#define PID_INEXISTANT 33
#define ERREUR_KILL 34

#define TAILLE_BUFFER 8
#define TAILLE_CMD 32
#define SIZE       10
#define TAILLE_BUF 512


/********************************************************************
 * Commande modinfo : Structure qui contiendra les infos d'un module
 ********************************************************************/
struct info_module {
	int id; /* L'ID de la tache si envoyée en asynchrone pour informer l'utilisateur si il veut faire un fg après */
	int pid; /* pid du processus qui exécute la commande */
	char asyn; /* si elle doit etre en mode asynchrone ou pas */
	int finish; /* si elle est finie pour pouvoir afficher le résultat, elle sert surtout pour les fonctions asynchrones */
	char arg[30]; /* l'argument passé au module, ici le nom du module */
	char param[100]; /* les paramètres du module recherché */
	char buffer[TAILLE_BUF]; /* contiendra les résultats de la commande */
};

/************************************************************************
 * Commande meminfo : Structure qui contiendra les infos sur la mémoire
 ************************************************************************/
struct info_mem {
	int id;	/* L'ID de la tache si envoyée en asynchrone pour informer l'utilisateur si il veut faire un fg après */
	int pid; /* pid du processus qui exécute la commande */
	char asyn; /* si elle doit etre en mode asynchrone ou pas */
	int finish; /* si elle est finie pour pouvoir afficher le résultat, elle sert surtout pour les fonctions asynchrones */
	char buffer[TAILLE_BUF]; /* contiendra les résultats de la commande*/
};

/********************************************************************
 * Commande Kill : Structure qui contiendra les infos à envoyer
 ********************************************************************/
struct envoie_signal{
	int id; /* L'ID de la tache si envoyée en asynchrone pour informer l'utilisateur si il veut faire un fg après */
	int pid; /* pid du processus à tuer */
	int sig; /* contindra le numéro de signal */
	char asyn; /* si elle doit etre en mode asynchrone ou pas */
	int finish; /* si elle est finie pour pouvoir afficher le résultat, elle sert surtout pour les fonctions asynchrones */
	int pid_return; /* pid du processus qui exécute la commande */
	char buffer[TAILLE_BUF]; /* contiendra les résultats de la commande*/
};

/**********************************************************************
 * Commande List : Structure qui contiendra la liste des tâches en cours
 **********************************************************************/
struct task_run{
	int id;	/* L'ID de la tache si envoyée en asynchrone pour informer l'utilisateur si il veut faire un fg après */
	int pid; /* pid du processus qui exécute la commande */
	char asyn; /* si elle doit etre en mode asynchrone ou pas */
	int finish; /* si elle est finie pour pouvoir afficher le résultat, elle sert surtout pour les fonctions asynchrones */
	char task_Running[TAILLE_BUF]; /* contiendra les résultats de la commande */
};


/***********************************************************************
 * Commande Wait : Structure qui contiendra la liste des PID a attendre
 ***********************************************************************/
struct wait_task{
	int pid; /* pid de la tache terminée */
	int val_retour; /* valeur de retour du processus terminé correspondant */
	int wait_size; /* la taille du tableau des pid à attendre */
	int tabPid[SIZE]; /* la tableau qui contindra les pid */
};

/*****************************************************************************
 * Commande Fg : Structure qui contiendra la réponse des commandes asynchrones
 *****************************************************************************/
struct answer_fg{
	int id; /* l'ID de la commande à mettre en avant plan */
	char buffer[TAILLE_BUF]; /* contindra le résultat de la commande mise en avant plan */
};

/**************************************************
 *          Prototype des fonctions
 **************************************************/
void print_tabCmd(char **tabCmd, int size);
void clear_tabCmd(char **tabCmd, int size);

#endif
