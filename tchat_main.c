#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PORT 3000



int main(){

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0); // Mettre 0 définit automatiquement le protocole en TCP ?
    
    struct sockaddr_in socket_addr = {

        .sin_addr.s_addr = INADDR_ANY,
        .sin_family = AF_INET,
        .sin_port = htons(SOCKET_PORT) // Équivalent de atoi
    };

    int error = bind(socket_fd, (struct sockaddr*) &socket_addr, sizeof socket_addr);
    perror("bind");

    if(error == -1){

        close(socket_fd);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}