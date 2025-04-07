#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

/* POUR LANCER LE CLIENT, UTILISER L'ALIAS : client_connect */

#define SERVER_PORT 3000
#define CLIENT_PORT 4000
#define USERNAME_LENGTH 50
#define TAB_SIZE 300
#define MAX_MSG_LENGTH 255
#define BUFFER_SIZE 600
#define EQUAL 0
#define LIST_USERS 1
#define LIST_SALONS 2
#define CREATE_SALON 1
#define JOIN_SALON 1
#define SEND_MP 1

int client_id = -1;

struct info_clients{

    int client_fd;
    char username[USERNAME_LENGTH];
};

struct info_clients client;

void askUsername(char *tab){

    printf("Veuillez saisir votre pseudo :\n\n");
    fgets(tab, USERNAME_LENGTH, stdin);
    tab[strcspn(tab, "\n")] = '\0';
    printf("Vous avez choisi le pseudo %s\n\n", tab);
    printf("Bienvenue dans TCHAT TCP!\n\n");
    //printf("Vous pouvez utiliser les commandes suivantes :\n\n");
    //printf("/salon create [numéro_de_salon] pour créer un salon. & /join salon [numéro_de_salon] pour en rejoindre un\n");
    //printf("/list users & /list salons\n");

}

int create_client(int port){

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    client_id = client_fd;
    
    if(client_fd == -1){
        return EXIT_FAILURE;
    } 

    struct sockaddr_in socket_addr = {

        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(port) // Équivalent de atoi
    };

    return client_fd;
}

void *handle_recv(void *arg){

    int client_fd = *(int *)arg;
    
    while(1){

        char buffer[TAB_SIZE] = {0};

        int bytes_received = recv(client_fd, buffer, TAB_SIZE, 0);
        //perror("\nrecv");
        
        if(bytes_received <= 0){
            
            printf("La connexion avec le serveur a été soudainement interrompue.\n");
            close(client_fd);
            break;
        }

        printf("\n%s\n\n", buffer);
    }
}

/* Mettre côté serveur create_salon */
int check_create_salon(char *user_msg){

    char create_salonCommand[] = "/salon create";
    // puis if %d après /salon create == salonID_stock[i], printf("Salon déjà utilisé, faites /salon join %d utilisé pour rejoindre le salon");

    // Utiliser strncmp
    int create_salon = strncmp(user_msg, create_salonCommand, 13);

    if(create_salon == EQUAL){

        return 1;
    
    }else{

        return -1;
    }


    //printf("message client : %s\n\n", user_msg);
    //printf("commande salon create : %s\n\n", create_salonCommand);
    //printf("Egal = %d\n", create_salon);

}

int check_list_cmd(char *message){

    char list_users[] = "/list users\n";
    //printf("Commande list:\n");
    char list_salons[] = "/list salons\n";

    int is_list_users = strncmp(message, list_users, 11);
    int is_list_salons = strncmp(message, list_salons, 12);

    if(is_list_users == EQUAL){

        return LIST_USERS;

    }else if(is_list_salons == EQUAL){

        return LIST_SALONS;
    
    }else{

        return -1;
    }
}

int check_privateMessage(char *message){

    char send_MP[] = "/msg";
    int isEqual = strncmp(send_MP, message, 4);

    if(isEqual == EQUAL){

        return 1;
   
    }else{

        return 0;
    }

}

int check_joinSalon(char *message){

    char joinSalon_cmd[] = "/join salon";
    int isEqual = strncmp(joinSalon_cmd, message, 10);

    if(isEqual == EQUAL){

        return 1;
   
    }else{

        return 0;
    }

}

int main(int argc, char **argv){

    pthread_t recv_thread, send_thread;
    
    char username[USERNAME_LENGTH];
    
    int server_port = atoi(argv[2]);
    
    int client_fd = create_client(server_port);
    
    if(client_fd == -1){
        
        return EXIT_FAILURE;
    }
    
    printf("Server listen on port : %d\n", server_port);
    
    /* Instanciation des éléments de la struct sur les infos du joueur */
    
    /* Déclaration et instanciation struct relatives au réseau */
    
    struct sockaddr_in socket_addr = {
        
        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(server_port) // Équivalent de atoi
    };
    
    int error = connect(client_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
    //perror("connect");
    
    askUsername(username);
   
    /* Mettre ça après le askUsername sinon le pseudo stocké est aléatoire */
    client.client_fd = client_fd;
    strcpy(client.username, username);

    if(error == -1){

        printf("Erreur de connexion au serveur\n");
        return EXIT_FAILURE;
    }

    pthread_create(&recv_thread, NULL, handle_recv, &client_fd);
    pthread_detach(recv_thread);

    while(1){

        char message[MAX_MSG_LENGTH] = {0}, buffer[BUFFER_SIZE] = {0};
        printf("[%s] > : ", username);
        fgets(message, MAX_MSG_LENGTH, stdin);
        message[strcspn(message, "\n")] = '\0';
        client.username[strcspn(client.username, "\n")] = '\0';

        int create_salon = check_create_salon(message);
        
        int list_command = check_list_cmd(message);
        
        int join_salon_command = check_joinSalon(message);

        int send_mp_to = check_privateMessage(message);

        if(create_salon != CREATE_SALON && list_command != LIST_USERS && list_command != LIST_SALONS && join_salon_command != JOIN_SALON && send_mp_to != SEND_MP){
              
            snprintf(buffer, BUFFER_SIZE, "[%s] > : %s", client.username, message); 
            int bytes_sent = send(client_fd, buffer, MAX_MSG_LENGTH, 0);

            if(bytes_sent == -1){
        
                printf("Erreur\n");
                break;
            }
        }else{

            int bytes_sent = send(client_fd, message, MAX_MSG_LENGTH, 0);
            //printf("Message : %s\n", message);

            if(bytes_sent == -1){
        
                printf("Erreur\n");
                break;
            }
        }

    }       
        
    
                
    return EXIT_SUCCESS;
}