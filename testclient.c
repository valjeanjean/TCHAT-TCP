#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 3000

int create_client(int port)
{

    // socket
    // Créer un socket TCP
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    perror("socket");
    if (client_fd == -1)
        return EXIT_FAILURE;

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    // bind

    return client_fd;
}

/**
 * Le CLIENT TCP
 */

int main(int argc, char **argv){

    
    if (argc < 3)
    {
        printf("Error : Missing ports param try \n\t./client 5000 4000\n");
        return EXIT_FAILURE;
    }
    
    int client_port = atoi(argv[1]);
    int server_port = atoi(argv[2]);
    if (client_port == 0)
    {
        printf("Error : wrong port try\n\t./client 5000\n");
        return EXIT_FAILURE;
    }
    
    int client_fd = create_client(client_port); 
    if(client_fd == EXIT_FAILURE) return EXIT_FAILURE;

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(server_port),
        .sin_addr.s_addr = inet_addr("127.0.0.1")
    };
    
    int error = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    perror("connect");
    if (error == -1) return EXIT_FAILURE;

    while(1){
        
        
        char buf[BUFSIZ]; memset(buf,0,BUFSIZ);
        printf("[%d]>", client_fd);
        fgets(buf,BUFSIZ,stdin);
        int nb_bytes = send(client_fd,buf,BUFSIZ,0);perror("send");
        
        if(nb_bytes == -1){
            return EXIT_FAILURE;
        }
        
        char paquet[BUFSIZ];
        memset(paquet, 0, BUFSIZ);
        nb_bytes = recv(client_fd,paquet,BUFSIZ,0);
        perror("recv");
        printf("Reçu : %d octets\t%s\n",nb_bytes,paquet);
        
        close(client_fd);
        
    }

    return EXIT_SUCCESS;
}