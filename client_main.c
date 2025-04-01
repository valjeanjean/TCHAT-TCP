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
#define TAB_SIZE 255
#define MAX_MSG_LENGTH 255
#define BUFFER_SIZE 600

int client_id = -1;

struct info_clients{

    int client_fd;
    char username[USERNAME_LENGTH];
};

void askUsername(char *tab){

    printf("Veuillez saisir votre pseudo :\n\n");
    fgets(tab, USERNAME_LENGTH, stdin);
    tab[strcspn(tab, "\n")] = '\0';
    printf("Vous avez choisi le pseudo %s\n\n", tab);
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

    char buffer[MAX_MSG_LENGTH] = {0};
    int client_fd = *(int *)arg;
    
    while(1){

        int bytes_received = recv(client_fd, buffer, MAX_MSG_LENGTH, 0);
        

        if(bytes_received <= 0){
            
            printf("La connexion avec le serveur a été soudainement interrompue.\n");
            close(client_fd);
            break;
        }

        printf("\n%s\n\n", buffer);
        
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
    struct info_clients client;
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

        snprintf(buffer, BUFFER_SIZE, "[%s] > : %s", client.username, message); 
        
        int bytes_sent = send(client_fd, buffer, MAX_MSG_LENGTH, 0);
        //printf("[SEND] Octets envoyés au serveur : %d\n\n", bytes_sent);
        //perror("send");
    
        if(bytes_sent == -1){
    
            printf("Erreur\n");
            break;
        }
    }       
        
    
                
    return EXIT_SUCCESS;
}