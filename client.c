#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>
#include "messages.h"
#include <sys/socket.h>  /* Pour socket */
#include <arpa/inet.h>   /* Pour sockaddr_in, inet_pton */
#include <string.h>      /* Pour memset */
#include <unistd.h>      /* Pour close */

#define NB_LIGNES_SIM		40				/* Dimensions des fenetres du programme */
#define NB_COL_SIM			80
#define NB_LIGNES_MSG		27
#define NB_COL_MSG			49
#define NB_LIGNES_OUTILS	6
#define NB_COL_OUTILS		49

#define HAUTEUR 6
#define LONGUEUR 7


WINDOW *fen_sim;							/* Fenetre de simulation partagee par les lems*/
WINDOW *fen_msg;							/* Fenetre de messages partagee par les lems*/


void ncurses_initialiser() {
	initscr();								/* Demarre le mode ncurses */
	cbreak();								/* Pour les saisies clavier (desac. mise en buffer) */
	noecho();								/* Desactive l'affichage des caracteres saisis */
	keypad(stdscr, TRUE);					/* Active les touches specifiques */
	refresh();								/* Met a jour l'affichage */
	curs_set(FALSE);						/* Masque le curseur */
	mousemask(BUTTON1_CLICKED, NULL);		/* Active le clic gauche de la souris*/
}

void ncurses_stopper() {
	endwin();
}




WINDOW *creer_fenetre_box_sim() {
/*Creation de la fenetre de contour de la fenetre de simulation */

	WINDOW *fen_box_sim;
	
	fen_box_sim = newwin(NB_LIGNES_SIM + 2, NB_COL_SIM + 2, 0, 0);
	box(fen_box_sim, 0, 0);
	mvwprintw(fen_box_sim, 0, (NB_COL_SIM + 2) / 2 - 5, "SIMULATION");	
	wrefresh(fen_box_sim);
	
	return fen_box_sim;
}

WINDOW *creer_fenetre_sim() {
/* Creation de la fenetre de simulation dans la fenetre de contour */
/* La simulation est affichee dans cette fenetre */

	WINDOW *fen_sim;
	
	fen_sim = newwin(NB_LIGNES_SIM, NB_COL_SIM, 1, 1);
	
	return fen_sim;
}

WINDOW *creer_fenetre_box_msg() {
/* Creation de la fenetre de contour de la fenetre de messages */

	WINDOW *fen_box_msg;
	
	fen_box_msg = newwin(NB_LIGNES_MSG + 2, NB_COL_MSG + 2, 0, NB_COL_SIM + 2);
	box(fen_box_msg, 0, 0);
	mvwprintw(fen_box_msg, 0, (NB_COL_MSG + 2) / 2 - 4, "MESSAGES");
	wrefresh(fen_box_msg);
	
	return fen_box_msg;
}

WINDOW *creer_fenetre_msg() {
/* Creation de la fenetre de messages dans la fenetre de contour */
/* Les messages indicatifs des evenements de la simulation et de l'interface */
/* utilisateur sont affiches dans cete fenetre */

	WINDOW *fen_msg;
	
	fen_msg = newwin(NB_LIGNES_MSG, NB_COL_MSG, 1, NB_COL_SIM + 3);
	scrollok(fen_msg, TRUE);
	
	return fen_msg;
}


void afficherGrille(unsigned char** grille){
	for(int i = 0; i < HAUTEUR; i++){
		for(int j = 0; j < LONGUEUR; j++){
			printf("%d",grille[i][j]);
			//printf("%d %d \n", i,j);
		}
		printf("\n");
	}

}
//Depot d'une piece dans la grille
int ajouterPiece(unsigned char*** grille, unsigned char ligne, unsigned char joueur){
	for(int i = HAUTEUR -1; i > 0; i--){
		printf("%d %d\n", i,ligne);
		
		if((*grille)[i][ligne] == 0){
			(*grille)[i][ligne] = joueur;
			return 1;
		}
		
	}
	return 0;
	
}

int main(int argc, char *argv[]){

	//TODO faire la connexion au serveur
	//TODO Faire un handler pour les signaux
	//TODO Faire un broadcast pour trouver l'adresse du serveur

	unsigned char type,idPartie,idJoueur;
	unsigned char** grille;
	int sockfd;
  	struct sockaddr_in adresseServeur;
  	unsigned char connexion_master[sizeof(unsigned char)];
  	unsigned char bufferMsg[sizeof(unsigned char) * 500];
  	socklen_t adresseSlaveLen = sizeof(struct sockaddr_in);


	if(argc != 2) {
        fprintf(stderr, "Usage: %s portServeur \n", argv[0]);
        fprintf(stderr, "\tOu:\n");
        fprintf(stderr, "\tport : port UDP du serveur\n");

        exit(EXIT_FAILURE);
    }

    /*****************************************************
	Broadcast UDP
    ******************************************************/
    struct sockaddr_in s;



    memset(&s, '\0', sizeof(struct sockaddr_in));
    s.sin_family = AF_INET;
    s.sin_port = (in_port_t)htons(atoi(argv[1]));
    s.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    type = 1;

	memcpy(&connexion_master,&type,sizeof(type));

	
    // Creation de la socket 
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("Erreur lors de la creation de la socket ");
		exit(EXIT_FAILURE);
	}
	int broadcastEnable=1;
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
	/*

	// Creation de l'adresse du serveur 
	memset(&adresseServeur, 0, sizeof(struct sockaddr_in));
	adresseServeur.sin_family = AF_INET;
	adresseServeur.sin_port = htons(atoi(argv[2]));
	if(inet_pton(AF_INET, argv[1], &adresseServeur.sin_addr) != 1) {
		perror("Erreur lors de la conversion de l'adresse ");
		exit(EXIT_FAILURE);
	}

	// connexion_master_udp.port = atoi(argv[3]); 
	type = 1;
	memcpy(&connexion_master,&type,sizeof(type));
	*/

	if(sendto(sockfd, connexion_master, sizeof(connexion_master), 0, (struct sockaddr *)&s, sizeof(struct sockaddr_in)) ==-1 ){
		perror("Erreur lors de l'envoi de l'envoi de la demande de connexion ");
    	exit(EXIT_FAILURE);
	}


	//TODO Mettre a jour l'adresse pour ne plus broadcast

	if(recvfrom(sockfd, bufferMsg, sizeof(bufferMsg), 0, (struct sockaddr*)&adresseServeur, &adresseSlaveLen) == -1) {
		perror("Erreur lors de la reception de la reception du message ");
		exit(EXIT_FAILURE);
	}

	memcpy(&type,&bufferMsg,sizeof(unsigned char));
	printf("Type : %u\n",type);

	switch(type){
		case 2: 
			//Connexion reussi
			memcpy(&idPartie,&bufferMsg[sizeof(unsigned char)],sizeof(unsigned char));
			memcpy(&idJoueur,&bufferMsg[sizeof(unsigned char) * 2],sizeof(unsigned char));

			grille = (unsigned char**) malloc(sizeof(unsigned char*) * HAUTEUR);
			for(int i = 0; i < HAUTEUR; i++){
				grille[i] = (unsigned char*) malloc(sizeof(unsigned char) * LONGUEUR);
				for(int j = 0; j < LONGUEUR;j++){
					grille[i][j] = 1;
				}
			}


			


		break;
	}


	while(1){
		if(recvfrom(sockfd, bufferMsg, sizeof(bufferMsg), 0, (struct sockaddr*)&adresseServeur, &adresseSlaveLen) == -1) {
			perror("Erreur lors de la reception de la reception du message ");
			exit(EXIT_FAILURE);
		}
		memcpy(&type,&bufferMsg,sizeof(unsigned char));
		printf("Type : %u\n",type);

		//On attends l'état de la partie, on joue, puis on envoi à nouveau

		switch(type){
			case 4:
				grille = (unsigned char**) malloc(sizeof(unsigned char *)* HAUTEUR);
				
				for(int i = 0; i< HAUTEUR; i++){
					grille[i] = (unsigned char *) malloc(sizeof(unsigned char) * LONGUEUR);
					for(int j = 0; j < LONGUEUR; j++){
						memcpy(&grille[i][j],&bufferMsg[sizeof(unsigned char) + ((i * LONGUEUR  + j ) * sizeof(unsigned char))],sizeof(unsigned char));
					}
					
				}

				afficherGrille(grille);
				printf("-------------------------------------\n");
				ajouterPiece(&grille, 3,1);
				afficherGrille(grille);
				ajouterPiece(&grille, 3,1);
				afficherGrille(grille);
				ajouterPiece(&grille, 3,1);
				afficherGrille(grille);

				//Reception de l'état de la partie

				//TODO Affichage de la grille

				//TODO recuperation du clique

				//TODO changement du visuel

				//TODO envoi de l'action


		}


	}

	//Type 2 = connexion OK
	//Type 3 == refus


	return 1;
}