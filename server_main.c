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
#define MAX_MSG_LENGTH 255

int clients_ID[MAX_CLIENTS] = {0};

/* Fonction de renvoi des messages clients */
void send_broadcast(char *message, int user_fd){

    for(int i = 0; i < MAX_CLIENTS; i++){

        if(clients_ID[i] > 0 && clients_ID[i] != user_fd){
            
            //printf("clients id : %d\n", clients_ID[i]);
            int error = send(clients_ID[i], message, MAX_MSG_LENGTH, 0);
            printf("[SEND] Octets envoyés : %d\n", error);
            if(error <= 0){

                printf("Erreur send sur la fonction send_broadcast\n");
                break;
            }
        }
    }
}

void *handle_client(void *arg){

    int client_fd = *(int *)arg;
    printf("client_fd = %d\n", client_fd);

    while(1){

        char packets[MAX_MSG_LENGTH] = {0};
        int bytes_received = recv(client_fd, packets, MAX_MSG_LENGTH, 0);
        perror("recv");

        printf("[RECV] Nombre d'octets du message reçu : %d\n\n", bytes_received);

        if(bytes_received <= 0){
            
            printf("Client %d vient de se déconnecter.\n", client_fd);
            
            for(int i = 0; i < MAX_CLIENTS; i++){
                
                if(clients_ID[i] == client_fd){
                    
                    clients_ID[i] = 0;
                    break;
                }
            }
            
            close(client_fd);
            break;
        }

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

        if(client_fd == -1){
            
            close(client_fd);
            close(server_fd);
            return EXIT_FAILURE;
        }

        for(int i = 0; i < MAX_CLIENTS; i++){

            if(client_fd == clients_ID[i]){

                break;
            }

            if(clients_ID[i] == 0){
                
                clients_ID[i] = client_fd;
                break;
            }
        }

        pthread_create(&server_accept, NULL, handle_client, &client_fd);
        pthread_detach(server_accept);
        
                
    }
                
    return EXIT_SUCCESS;
}