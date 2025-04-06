#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

/* Utiliser commande server_connect pour lancer le serveur */

#define SERVER_PORT 3000
#define MAX_CLIENTS 100
#define MAX_MSG_LENGTH 300
#define SALON_WORLD -1
#define EQUAL 0
#define CLIENT_COMMAND 20
#define TAB_SIZE 255
#define LIST_USERS 1
#define LIST_SALONS 2
#define INFOS_TAB 300
#define ONLINE 1
#define JOIN 1
#define BIG_SIZE 600
#define SEND 1

/* Utiliser struct pour y stocker infos : FD, salon id &  */
struct clients_infos{

    int client_fd;
    int salon_id;
    int status;
};

struct clients_infos clients[MAX_CLIENTS] = {0};

/* Mettre côté serveur create_salon */
void create_salon(char *user_msg, int user_fd){

    // puis if %d après /salon create == salonID_stock[i], printf("Salon déjà utilisé, faites /salon join %d utilisé pour rejoindre le salon");

    // Utiliser strncmp
    int create_salon = strncmp(user_msg, "/salon create", 13);

    //printf("message client : %s\n\n", user_msg);
    //printf("commande salon create : %s\n\n", create_salonCommand);
    //printf("Egal = %d\n", create_salon);

    int i = 0;

    if(create_salon == EQUAL){
        
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

/* Vérifie si la commande /list a été entrée, return -1 pour faux */
int check_list_cmd(char *message){

    int is_list_users = strncmp(message, "/list users", 11);
    int is_list_salons = strncmp(message, "/list salons", 12);

    if(is_list_users == EQUAL){

        return LIST_USERS;

    }else if(is_list_salons == EQUAL){

        return LIST_SALONS;
    
    }else{

        return -1;
    }

}

/* Vérifie si la commande /msg a été entrée */
int check_privateMessage(char *message){

    char send_MP[] = "/msg";
    int isEqual = strncmp(send_MP, message, 4);

    if(isEqual == EQUAL){

        return 1;
   
    }else{

        return 0;
    }

}

/* Vérifie que la commande /join salon a été entrée */
int check_joinSalon(char *message){

    char joinSalon_cmd[] = "/join salon";
    int isEqual = strncmp(joinSalon_cmd, message, 10);

    if(isEqual == EQUAL){

        return 1;
   
    }else{

        return 0;
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
        int is_list = check_list_cmd(packets);
        int is_join = check_joinSalon(packets);
        int send_MP = check_privateMessage(packets);
        char online_users[INFOS_TAB] = {0};
        //char user_online_IDs[BIG_SIZE] = {0};
        printf("taille tab = %ld\n", sizeof(online_users));

        /* Marche pas, affiche seulement un seul joueur connecté alors qu'il y en a plusieurs */
        /* Corrigé mais l'affichage se fait 1 par 1, donc si un autre client envoie un message au même moment, il s'intercalera entre 2 printf de clients connectés */

        if(is_list == LIST_USERS){

            for(int i = 0; i < MAX_CLIENTS; i++){

                if(clients[i].status == ONLINE && clients[i].client_fd != client_fd && clients[i].client_fd > 3){

                    sprintf(online_users, "\nClient %d connecté",clients[i].client_fd);
                    printf("%s\n", online_users);
                    send(client_fd, online_users, INFOS_TAB, 0);
                }
            }
            
        }else if(is_list == LIST_SALONS){

            char available_salon[INFOS_TAB] = {0};

            for(int i = 0; i < MAX_CLIENTS; i++){

                if(clients[i].status == ONLINE && clients[i].salon_id > 0){

                    sprintf(available_salon, "\nSalon %d ouvert",clients[i].salon_id);
                    printf("%s\n", online_users);
                    send(client_fd, available_salon, INFOS_TAB, 0);
                }
            }
        }

        for(int i = 0; i < MAX_CLIENTS; i++){
        
            if(is_join == JOIN && clients[i].client_fd == client_fd){

                int temp_salon_id = 0;
                char tab[TAB_SIZE];

                sscanf(packets, "/join salon %d", &temp_salon_id); // Mettre variable de l'utilisateur envoyant le truc
                clients[i].salon_id = temp_salon_id;
                sprintf(tab, "Salon %d rejoint!\n", clients[i].salon_id);
                send(client_fd, tab, TAB_SIZE, 0);

                break;            
            }
        }

        if(send_MP == SEND){

            int chars_read = 0;
            int send_mp_to = 0;
            sscanf(packets, "/msg %d %n", &send_mp_to, &chars_read);
            char *mp_sent = packets + chars_read;
            printf("MP qui sera reçu : %s\n", mp_sent);

            for(int i = 0; i < MAX_CLIENTS; i++){

                if(clients[i].client_fd == send_mp_to){

                    int length = strlen(mp_sent);
                    send(clients[i].client_fd, mp_sent, length, 0);
                    break;
                }
            }
        }

        printf("send_MP = %d\t is_list = %d\t is_join = %d\n", send_MP, is_list, is_join);
        if(send_MP == 0 && is_list == -1 && is_join == 0){

            send_broadcast(packets, client_fd);
        }else{

            printf("Commande privée non généralement notifiée!\n");
        }
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
                clients[i].status = ONLINE;
                //printf("salon_id = %d\n", clients[i].salon_id);

                break;
            }
        }

        pthread_create(&server_accept, NULL, handle_client, &client_fd);
        pthread_detach(server_accept);
        
                
    }
                
    return EXIT_SUCCESS;
}