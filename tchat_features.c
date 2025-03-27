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

int clients_id[MAX_CLIENTS];

/* Modèle commande pour quitter le tchat */
/* FONCTIONS POUR LE CLIENT */
void list_users(char *user_msg, int user_fd){

    char list_command[] = "/list\n";
    
    printf("Commande list:\n");
    fgets(user_msg, TAB_SIZE, stdin);

    int strcmp_value = strcmp(user_msg, list_command);

    if(strcmp_value == 0){

        for(int i = 0; i < MAX_CLIENTS; i++){

            clients_id[i] = i + 1;

            if(clients_id[i] != user_fd && clients_id[i] != 0){

                printf("Client %d connecté\n", clients_id[i]);
            }
        }
    }    
}

int leave_tchat(char *user_msg){
    
    char quit_command[] = "/quit\n";
    
    printf("Commande leave:\n");
    fgets(user_msg, TAB_SIZE, stdin);

    int value_strcmp = strcmp(user_msg, quit_command);

    if(value_strcmp == 0){

        return 1;
    }

    return 0;
}

int main(){

    int isLeaving;

    int own_fd = 3;

    char tab[TAB_SIZE];
    int value = sizeof(tab);
    printf("Valeur sizeof(tab) = %d\n\n", value);

    if((leave_tchat(tab)) == 1){

        printf("Vous avez choisi de vous déco\n");
        return EXIT_FAILURE;
    }

    char tab2[TAB_SIZE];

    list_users(tab2, own_fd);



    return 0;
}
