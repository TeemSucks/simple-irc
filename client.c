#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h> // Include the pthread library

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to handle receiving and printing messages from the server
void *receive_messages(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (received <= 0) {
            // Connection closed or error occurred
            printf("Connection to server closed.\n");
            break;
        }

        printf("%s\n", buffer);
    }

    // Exit the thread
    pthread_exit(NULL);
}

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    char server_host[256];

    printf("Enter the server's IP: ");
    fgets(server_host, sizeof(server_host), stdin);
    server_host[strcspn(server_host, "\n")] = '\0';

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(server_host, NULL, &hints, &result) != 0) {
        perror("Error resolving server's IP");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *server_info = (struct sockaddr_in *)result->ai_addr;
    server_address.sin_family = AF_INET;
    server_address.sin_addr = server_info->sin_addr;
    server_address.sin_port = htons(PORT);

    freeaddrinfo(result);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    char nickname[50];
    printf("Enter your nickname: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = '\0';

    send(client_socket, nickname, strlen(nickname), 0);

    // Create a thread for receiving messages
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void *)&client_socket) != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    // Main loop for sending messages
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        printf("Enter your message: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        send(client_socket, buffer, strlen(buffer), 0);
    }

    // Close the client socket
    close(client_socket);

    return 0;
}
