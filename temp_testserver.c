#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS]; // Tableau des sockets clients

void broadcast_message(int sender_fd, char *message, int message_len) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0 && clients[i] != sender_fd) { // Ne pas envoyer au client expéditeur
            send(clients[i], message, message_len, 0);
        }
    }
}

int main() {
    int server_fd, new_socket, max_sd, activity;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set readfds; // Ensemble de descripteurs pour `select()`
    
    // Création du socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Liaison du socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erreur de bind");
        exit(EXIT_FAILURE);
    }

    // Écoute des connexions entrantes
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Erreur de listen");
        exit(EXIT_FAILURE);
    }

    // Initialiser le tableau des clients à zéro
    memset(clients, 0, sizeof(clients));

    printf("Serveur en attente de connexions sur le port %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);  // Réinitialiser l'ensemble de lecture
        FD_SET(server_fd, &readfds); // Ajouter le socket du serveur
        max_sd = server_fd;

        // Ajouter les clients à l'ensemble
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        // Attente d'activité sur l'un des sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Erreur de select");
            continue;
        }

        // Nouvelle connexion entrante
        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, &addr_len);
            if (new_socket < 0) {
                perror("Erreur d'acceptation");
                continue;
            }

            printf("Nouvelle connexion : socket %d, IP %s, Port %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Ajouter le nouveau client au tableau
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == 0) {
                    clients[i] = new_socket;
                    break;
                }
            }
        }

        // Vérifier l'activité des clients existants
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i];

            if (FD_ISSET(sd, &readfds)) {
                int bytes_received = recv(sd, buffer, BUFFER_SIZE, 0);
                if (bytes_received <= 0) {
                    // Déconnexion du client
                    printf("Client socket %d déconnecté\n", sd);
                    close(sd);
                    clients[i] = 0;
                } else {
                    // Diffuser le message aux autres clients
                    buffer[bytes_received] = '\0'; // Assurer la terminaison de la chaîne
                    printf("Message reçu de %d : %s\n", sd, buffer);
                    broadcast_message(sd, buffer, bytes_received);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
