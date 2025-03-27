#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Création du socket client
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Erreur de socket");
        exit(EXIT_FAILURE);
    }

    // Configuration du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion");
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur ! Tapez un message :\n");

    // Boucle d’envoi/réception
    while (1) {
        // Lire l’entrée utilisateur
        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);

        // Lire la réponse
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Connexion au serveur perdue.\n");
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("Message reçu : %s", buffer);
    }

    close(sock);
    return 0;
}
