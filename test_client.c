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
#define CLIENT_PORT 4000
#define USERNAME_LENGTH 50
#define TAB_SIZE 255

int client_id = -1;

struct info_clients{

    int client_fd;
    char username[USERNAME_LENGTH];
};

void askUsername(char *tab, int user_fd){

    printf("Veuillez saisir votre pseudo :\n");
    fgets(tab, USERNAME_LENGTH, stdin);
    printf("Vous avez choisi le pseudo %s\n\n", tab);
    send(user_fd, tab, USERNAME_LENGTH, 0);
}

int create_client(int port){

    int client_fd = socket(AF_INET, SOCK_STREAM, 0); // Mettre 0 définit automatiquement le protocole en TCP ?
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

    char buffer[TAB_SIZE] = {0};
    int client_fd = *(int *)arg;
    
    while(1){

        int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        //perror("recv");
        
        if(bytes_received > 0){
            
            printf("Message client : %s\n", buffer);
        }

        if(bytes_received <= 0){
            
            printf("La connexion avec le serveur a été soudainement interrompue.\n");
            close(client_fd);
            break;
        }
        
        //printf("client_fd = %d\n\n", client_fd);
        //printf("bytes_received = %d\n", bytes_received);
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
    struct info_clients client;
    client.client_fd = client_fd;
    strcpy(client.username, username);

    /* Déclaration et instanciation struct relatives au réseau */
                
    struct sockaddr_in socket_addr = {

        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(server_port) // Équivalent de atoi
    };

    int error = connect(client_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
    //perror("connect");

    askUsername(username, client_fd);

    if(error == -1){

        printf("Erreur de connexion au serveur\n");
        return EXIT_FAILURE;
    }

    pthread_create(&recv_thread, NULL, handle_recv, &client_fd);

    while(1){

        char packets[BUFSIZ] = {0};
        printf("\n[%d]> :", client_fd);
        fgets(packets, sizeof(packets), stdin);

        int bytes_sent = send(client_fd, packets, sizeof(packets), 0);
        //perror("send");
    
        if(bytes_sent == -1){
    
            printf("Erreur\n");
            break;
        }
    }       
        
    
                
    return EXIT_SUCCESS;
}