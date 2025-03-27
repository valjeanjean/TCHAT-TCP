#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define MAX_ROOMS 5
#define BUFFER_SIZE 1024
#define SERVER_PORT 3000

typedef struct {
    int socket;
    char username[50];
    int room_id;
} Client;

typedef struct {
    char name[50];
    int clients[MAX_CLIENTS];
} Room;

Client clients[MAX_CLIENTS];
Room rooms[MAX_ROOMS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_message_to_room(int sender_fd, int room_id, char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != 0 && clients[i].room_id == room_id && clients[i].socket != sender_fd) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];

    recv(client_fd, buffer, 50, 0);
    pthread_mutex_lock(&clients_mutex);
    int index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket == 0) {
            clients[i].socket = client_fd;
            strcpy(clients[i].username, buffer);
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (index == -1) {
        char *msg = "Server full!\n";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
        return NULL;
    }

    sprintf(buffer, "%s  joined. Use /room <name> to join a room.\n", clients[index].username);
    send(client_fd, buffer, strlen(buffer), 0);

    while (1) {
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) break;
        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "/room ", 6) == 0) {
            char room_name[50];
            sscanf(buffer + 6, "%s", room_name);

            pthread_mutex_lock(&clients_mutex);
            int room_id = -1;
            for (int i = 0; i < MAX_ROOMS; i++) {
                if (strcmp(rooms[i].name, room_name) == 0) {
                    room_id = i;
                    break;
                }
            }

            if (room_id == -1) {
                for (int i = 0; i < MAX_ROOMS; i++) {
                    if (rooms[i].name[0] == '\0') {
                        strcpy(rooms[i].name, room_name);
                        room_id = i;
                        break;
                    }
                }
            }

            if (room_id != -1) {
                clients[index].room_id = room_id;
                sprintf(buffer, "Joined room: %s\n", rooms[room_id].name);
            } else {
                sprintf(buffer, "Room limit reached!\n");
            }
            pthread_mutex_unlock(&clients_mutex);
            send(client_fd, buffer, strlen(buffer), 0);
        } else {
            sprintf(buffer, "%s: %s", clients[index].username, buffer);
            send_message_to_room(client_fd, clients[index].room_id, buffer);
        }
    }

    close(client_fd);
    pthread_mutex_lock(&clients_mutex);
    clients[index].socket = 0;
    pthread_mutex_unlock(&clients_mutex);
    return NULL;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {AF_INET, htons(server_port), INADDR_ANY};

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);

    printf("Server started on port %d\n", server_port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_fd);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
