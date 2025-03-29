#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 3000

int create_server(int port)
{

    // socket
    // Créer un socket TCP
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    perror("socket");
    if (server_fd == -1)
        return EXIT_FAILURE;

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)};

    // bind
    int error = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    perror("bind");
    if (error == -1)
        return EXIT_FAILURE;

    // listen
    error = listen(server_fd, BUFSIZ);
    perror("listen");
    if (error == -1)
        return EXIT_FAILURE;

   
    return server_fd;
}


/**
 * LE SERVER TCP
 */
int main(int argc, char** argv)
{

    if (argc < 2)
    {
        printf("Error : Missing port param try \n\t./client 5000\n");
        return EXIT_FAILURE;
    }

    int server_port = atoi(argv[1]);
    if (server_port == 0)
    {
        printf("Error : wrong port try\n\t./client 5000\n");
        return EXIT_FAILURE;
    }


    int server_fd = create_server(server_port);
    if(server_fd == EXIT_FAILURE) return EXIT_FAILURE;

    while (1)
    {
        struct sockaddr addr_client;
        socklen_t addr_client_len;
        // J'attend qu'un client arrive...
        printf("Server waiting for client connection...\n");
        int client_fd = accept(server_fd, &addr_client, &addr_client_len);
        perror("accept");
        if (client_fd == -1){
            close(client_fd);
            continue;
        }
        printf("Client %d à été accepté !\n", client_fd);

        // J'attend de reçevoir sa requête
        // recv
        printf("Server waiting for client request...\n");
        char paquet[BUFSIZ];
        memset(paquet, 0, BUFSIZ);
        int nb_bytes = recv(client_fd,paquet,BUFSIZ,0);
        perror("recv");

        if (nb_bytes == -1)
        {
            close(client_fd);
            continue;
        }
        
        printf("Nombre d'octets reçu %d !\n",nb_bytes);
        printf("DATA : %s\n", paquet);
        if(nb_bytes == 0){
            printf("Client %d à été déconnecté de façon innopiné !\n", client_fd);
        }
        memset(paquet,0,BUFSIZ);
        int nb_sent = send(client_fd,paquet,BUFSIZ,0);
        perror("send");
        if(nb_sent == -1){
            return EXIT_FAILURE;
        }
        // Ferme la connexion avec le client
        //close(client_fd);
    }

    return EXIT_SUCCESS;
}