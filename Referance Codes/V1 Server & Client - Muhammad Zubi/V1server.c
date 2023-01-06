#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
struct client_data {
    int client_fd;
    struct sockaddr_in client_addr;
};

void *handle_client(void *arg) {
    int bytes_received;
    char buffer[BUFFER_SIZE];

    struct client_data *data = (struct client_data *)arg;
    int client_fd = data->client_fd;
    struct sockaddr_in client_addr = data->client_addr;

    extern int num_clients;
    extern int clients[MAX_CLIENTS];

    while (1) {
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Error receiving data");
            break;
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Received message from client %s:%d: %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        // Broadcast the message to all clients
        for (int i = 0; i < num_clients; i++) {
            // Don't send the message back to the same client
            if (clients[i] != client_fd) {
                send(clients[i], buffer, bytes_received, 0);
            }
        }
    }

    // Remove client from the list of clients
    for (int i = 0; i < num_clients; i++) {
        if (clients[i] == client_fd) {
            clients[i] = clients[num_clients - 1];
            num_clients--;
            break;
        }
    }

    close(client_fd);
    free(arg);

    pthread_exit(NULL);
}


int clients[MAX_CLIENTS];
int num_clients = 0;

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    pthread_t thread_id;
    struct client_data *data;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
if (listen(server_fd, 10) < 0) {
perror("Error listening for connections");
exit(EXIT_FAILURE);
}
printf("Waiting for client connections on port %d...\n", PORT);
while (1) {
sin_size = sizeof(client_addr);
client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
if (client_fd < 0) {
perror("Error accepting connection");
continue;
}
printf("Connected to client: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));  
// Add client to the list of clients
if (num_clients < MAX_CLIENTS) {
    clients[num_clients] = client_fd;
    num_clients++;
} else {
    printf("Error: Too many clients\n");
    close(client_fd);
    continue;
}

data = (struct client_data *)malloc(sizeof(struct client_data));
data->client_fd = client_fd;
data->client_addr = client_addr;

if (pthread_create(&thread_id, NULL, handle_client, data) < 0) {
    perror("Error creating thread");
    continue;
}

}
close(server_fd);
return 0;
}
