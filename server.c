#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct Client {
    int socket;
    char nickname[50];
    char ip[INET_ADDRSTRLEN];
};

void broadcast(struct Client* clients, int client_count, char* message, int sender_socket) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

void log_message(const char* sender, const char* message) {
    printf("%s: %s\n", sender, message);
}

int main() {
    int server_socket, client_count = 0;
    struct sockaddr_in server_address, client_address;
    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    listen(server_socket, MAX_CLIENTS);
    printf("Server listening on port %d...\n", PORT);

    struct Client clients[MAX_CLIENTS];

    signal(SIGCHLD, SIG_IGN);

    while (1) {
        socklen_t client_address_len = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        recv(client_socket, clients[client_count].nickname, sizeof(clients[client_count].nickname), 0);
        inet_ntop(AF_INET, &client_address.sin_addr, clients[client_count].ip, INET_ADDRSTRLEN);

        printf("%s joined. IP: %s\n", clients[client_count].nickname, clients[client_count].ip);

        clients[client_count].socket = client_socket;
        client_count++;

        pid_t pid = fork();

        if (pid == 0) {
            close(server_socket);

            while (1) {
                memset(buffer, 0, sizeof(buffer));
                ssize_t received = recv(client_socket, buffer, BUFFER_SIZE, 0);

                if (received <= 0) {
                    printf("%s left the chat.\n", clients[client_count - 1].nickname);
                    close(client_socket);

                    for (int i = 0; i < client_count; i++) {
                        if (clients[i].socket == client_socket) {
                            for (int j = i; j < client_count - 1; j++) {
                                clients[j] = clients[j + 1];
                            }
                            client_count--;
                            break;
                        }
                    }

                    exit(EXIT_SUCCESS);
                }

                if (buffer[0] == '/') {
                    char* command = strtok(buffer, " ");
                    if (strcmp(command, "/kick") == 0) {
                        char* target_nickname = strtok(NULL, " ");
                        for (int i = 0; i < client_count; i++) {
                            if (strcmp(clients[i].nickname, target_nickname) == 0) {
                                close(clients[i].socket);
                                printf("%s has been kicked by %s\n", target_nickname, clients[client_count - 1].nickname);

                                // ... (Previous code to remove client)

                                break;
                            }
                            
                        }
                    }
                } else {
                    char formatted_message[BUFFER_SIZE + 50];
                    sprintf(formatted_message, "%s: %s", clients[client_count - 1].nickname, buffer);

                    broadcast(clients, client_count, formatted_message, client_socket);

                    log_message(clients[client_count - 1].nickname, buffer);
                }
            }
        } else if (pid > 0) {
            close(client_socket);
        } else {
            perror("Error forking");
            exit(EXIT_FAILURE);
        }
    }

    close(server_socket);

    return 0;
}
