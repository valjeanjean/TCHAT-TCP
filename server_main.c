#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define SERVER_PORT 3000
#define MAX_CLIENTS 100
#define MAX_MSG_LENGTH 300
#define SALON_WORLD -1
#define EQUAL 0
#define CLIENT_COMMAND 20
#define TAB_SIZE 255
#define LIST_USERS 1
#define LIST_SALONS 2

/* Utiliser struct pour y stocker infos : FD, salon id &  */
struct clients_infos{

    int client_fd;
    int salon_id;
};

struct clients_infos clients[MAX_CLIENTS] = {0};

/* Mettre côté serveur create_salon */
void create_salon(char *user_msg, int user_fd){

    char create_salonCommand[] = "/salon create";
    // puis if %d après /salon create == salonID_stock[i], printf("Salon déjà utilisé, faites /salon join %d utilisé pour rejoindre le salon");

    // Utiliser strncmp
    int create_salon = strncmp(user_msg, create_salonCommand, 13);

    //printf("message client : %s\n\n", user_msg);
    //printf("commande salon create : %s\n\n", create_salonCommand);
    //printf("Egal = %d\n", create_salon);

    int i = 0;

    if(create_salon == 0){
        
        for(i = 0; i < MAX_CLIENTS; i++){
            
            // Finir la condition avec && pour vérifier que l'ID alloué n'a pas déjà été attribué, et faire en sorte que l'ID du salon soit supprimé lorsque 
            // plus aucun client n'est dans ce salon.
            
            if(clients[i].client_fd == user_fd){ 
                
                //printf("Itération de i numéro : %d\n\n", i);
                sscanf(user_msg, "/salon create %d", &clients[i].salon_id);
                int compteur = i;
                char tab[TAB_SIZE] = {0};
                sprintf(tab, "Salon créé! Numéro de salon : %d\n", clients[compteur].salon_id);
                //printf("client fd de l'envoyeur : %d\n", clients[compteur].client_fd);
                send(clients[compteur].client_fd, tab, TAB_SIZE, 0);
                perror("send");
                break;
            }
        }
    }
}

/* Fonction de renvoi des messages clients */
void send_broadcast(char *message, int user_fd){

    printf("expéditeur_fd = %d\n\n", user_fd);

    int user_salon_id = SALON_WORLD;

    for(int i = 0; i < MAX_CLIENTS; i++){

        if(clients[i].client_fd == user_fd){

            user_salon_id = clients[i].salon_id;
            break;
        }
    }

    for(int i = 0; i < MAX_CLIENTS; i++){

        if(clients[i].client_fd > 0 && clients[i].client_fd != user_fd && clients[i].salon_id == user_salon_id){
            
            int error = send(clients[i].client_fd, message, MAX_MSG_LENGTH, 0);
            printf("[SEND] Octets envoyés : %d\n", error);

            if(error <= 0){

                printf("Erreur send sur la fonction send_broadcast\n");
                break;
            }
        }

    }
}

int check_list_cmd(char *message){

    char list_users[] = "/list users";
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

void *handle_client(void *arg){

    int client_fd = *(int *)arg;
    //printf("client_fd = %d\n", client_fd);

    while(1){

        char packets[MAX_MSG_LENGTH] = {0};
        int bytes_received = recv(client_fd, packets, MAX_MSG_LENGTH, 0);
        perror("recv");
        printf("Message reçu : %s\n", packets);

        printf("[RECV] Nombre d'octets du message reçu : %d\n\n", bytes_received);

        if(bytes_received <= 0){
            
            printf("Client %d vient de se déconnecter.\n", client_fd);
            
            for(int i = 0; i < MAX_CLIENTS; i++){
                
                if(clients[i].client_fd == client_fd){
                    
                    clients[i].client_fd = 0;
                    break;
                }
            }
            
            close(client_fd);
            break;
        }

        create_salon(packets, client_fd);
        check_list_cmd(packets);
        send_broadcast(packets, client_fd);
    }
    
    return NULL;
}

int create_server(int port){

    int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    
    if(server_fd == -1){
        return EXIT_FAILURE;
    } 

    struct sockaddr_in socket_addr = {

        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(port) 
    };

    int error = bind(server_fd, (struct sockaddr*) &socket_addr, sizeof socket_addr);
    perror("bind");
    
    if(error == -1){

        close(server_fd);
        return EXIT_FAILURE;
    }

    error = listen(server_fd, 100);
    perror("listen");

    if(error == -1){

        close(server_fd);
        return EXIT_FAILURE;
    }

    return server_fd;
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


int main(int argc, char**argv){

    pthread_t server_accept;

    int server_port = atoi(argv[1]);
    
    int server_fd = create_server(server_port);

    while(1){
        
        printf("Server listen on port : %d\n", server_port);
        
        struct sockaddr_in client_addr;
        socklen_t len;
        
        int client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &len);
        perror("accept");
    
        if(client_fd < 3){ // if client_fd < 4 ?
            
            close(client_fd);
            // continue ?
            close(server_fd);
            return EXIT_FAILURE;
        }

        for(int i = 0; i < MAX_CLIENTS; i++){

            if(client_fd == clients[i].client_fd){

                break;
            }

            if(clients[i].client_fd == 0){
                
                clients[i].client_fd = client_fd;
                //printf("client_fd du client %d = %d\n", i, clients[i].client_fd);
                clients[i].salon_id = -1;
                //printf("salon_id = %d\n", clients[i].salon_id);

                break;
            }
        }

        pthread_create(&server_accept, NULL, handle_client, &client_fd);
        pthread_detach(server_accept);
        
                
    }
                
    return EXIT_SUCCESS;
}