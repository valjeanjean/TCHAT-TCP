#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 3000
#define TAB_SIZE 255
#define MAX_CLIENTS 15
#define MAX_SALONS 10
#define EQUAL 0

int clients_id[MAX_CLIENTS];
int salons_ID[MAX_SALONS] = {1, 2, 3, 4, 5}; //exemple 
char username[TAB_SIZE] = "Gianluigi";
int compteur = -1;

void checkSalon(){
    
    
    
}

/* Mettre côté serveur create_salon */
void create_salon(char *user_msg, int user_fd){

    char create_salonCommand[] = "/salon create\n";
    // puis if %d après /salon create == salonID_stock[i], printf("Salon déjà utilisé, faites /salon join %d utilisé pour rejoindre le salon");

    // Utiliser strncmp
    int create_salon = strncmp(user_msg, create_salonCommand, 13);
    int i = 0;

    if(create_salon == EQUAL){
        
        for(i = 0; i < MAX_CLIENTS; i++){
            
            // Finir la condition avec && pour vérifier que l'ID alloué n'a pas déjà été attribué, et faire en sorte que l'ID du salon soit supprimé lorsque 
            // plus aucun client n'est dans ce salon.
            
            if(salons_ID[i] == 0){ 
                
                printf("Itération de i numéro : %d\n\n", i);
                sscanf(user_msg, "/salon create %d\n", &salons_ID[i]);
                compteur = i;
                printf("Salon créé! Numéro de salon : %d\n", salons_ID[i]);
                break;
            }
        }
    }
}

/* Modèle commande pour quitter le tchat */
/* FONCTIONS POUR LE CLIENT */
/* MODIFIER CAR J'AI FAIT UN TABLEAU EN DUR POUR TESTER AVEC UN PSEUDO & UN TABLEAU AVEC DE FAKES ID */
void command_list(char *user_msg, int user_fd){

    char list_command[] = "/list users\n";
    //printf("Commande list:\n");
    char list_salons[] = "/list salons\n";

    int is_cmd_list = strcmp(user_msg, list_command);
    int is_cmd_salon = strcmp(user_msg, list_salons);

    if(is_cmd_list == EQUAL){

        printf("\n\033[0;34muser_id\t\033[0m\033[0;31mNom d'utilisateur\033[0m\n\n");

        for(int i = 0; i < MAX_CLIENTS; i++){

            clients_id[i] = i + 1;

            if(clients_id[i] != user_fd && clients_id[i] != 0){

                printf("\033[0;34m%d\033[0m\t\033[0;31m%s\033[0m\n", clients_id[i], username);
            }
        }
    }
    
    if(is_cmd_salon == EQUAL){

        for(int i = 0; i < MAX_SALONS; i++){

            if(salons_ID[i] != 0){

                printf("\nSalon \033[0;34m%d\033[0m ouvert\n", salons_ID[i]); //cmt faire l'affichage des salons
            }
        }

        printf("\nUtilisez la commande /join salon [numéro_du_salon] pour rejoindre un des salons!\n");
    }
}

int leave_tchat(char *user_msg){
    
    char quit_command[] = "/quit\n";
    
    //printf("Commande leave:\n");

    int value_strcmp = strcmp(user_msg, quit_command);

    if(value_strcmp == EQUAL){

        return 1;
    }

    return 0;
}



int main(){

    int isLeaving;

    int own_fd = 3;

    char client_msg[TAB_SIZE];
    int value = sizeof(client_msg);

    printf("Veuillez entrer une phrase :\n");
    fgets(client_msg, sizeof(client_msg), stdin);

    if((leave_tchat(client_msg)) == 1){

        printf("Vous avez choisi de vous déco\n");
        return EXIT_FAILURE;
    }

    char tab2[TAB_SIZE];

    command_list(client_msg, own_fd);

    create_salon(client_msg, own_fd);
    


    return 0;
}
