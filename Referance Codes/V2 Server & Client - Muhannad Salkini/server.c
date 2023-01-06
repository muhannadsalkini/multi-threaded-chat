#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct Client {
    char name[BUFFER_SIZE];
    int socket;
};

struct Message {
    char sender[BUFFER_SIZE];
    char receiver[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char error_checking_bits[BUFFER_SIZE];
};

// Function to service client connections in a separate thread
void* service_client(void* client_ptr) {
    // Cast void* pointer to struct Client*
    struct Client* client = (struct Client*) client_ptr;
    int client_socket = client->socket;
    char buffer[BUFFER_SIZE];

    // Receive messages from client and process them
    while (1) {
        int received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (received <= 0) {
            break;
        }

        // Parse message
        struct Message message;
        sscanf(buffer, "%s|%s|%s|%s", message.sender, message.receiver, message.message, message.error_checking_bits);

        // Process message based on its type
        if (strcmp(message.sender, "CONN") == 0) {
            // Do nothing, this is just a connection message
        } 
        else if (strcmp(message.sender, "MESG") == 0) {
            if (check_error(message) == 0) {
                send_message(clients, client_socket, message);
            } 
            else {
                send(client_socket, "MERR", 4, 0);
            }
        } 
        else if (strcmp(message.sender, "MERR") == 0) {
            send_message(clients, client_socket, message);
        } 
        else if (strcmp(message.sender, "GONE") == 0) {
            break;
        }
    }
    // Remove client and close their socket
    remove_client(clients, client_socket);
    close(client_socket);
    return NULL;
}

// Function to send a message to a specific client
void send_message(struct Client* clients, int sender_socket, struct Message message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (strcmp(clients[i].name, message.receiver) == 0) {
            int receiver_socket = clients[i].socket;
            send(receiver_socket, &message, sizeof(message), 0);
            break;
        }
    }
}

// Function to broadcast a message to all clients except the sender
void broadcast(struct Client* clients, int sender_socket, char* message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

// This function adds a new client to the list of clients.
void add_client(struct Client* clients, struct Client client) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Iterate through the list of clients.
        if (clients[i].socket == 0) {
            // If this spot in the list is empty, add the new client here.
            clients[i] = client;
            break;
        }
    }
}

void remove_client(struct Client* clients, int socket) {
    // This function removes a client from the list of clients.
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Iterate through the list of clients.
        if (clients[i].socket == socket) {
            // If the socket of the current client matches the socket of the client to be removed, set this spot in the list to empty.
            clients[i] = (struct Client) {0};
            break;
        }
    }
}

int check_error(struct Message message) {
    // Implement error checking here
    return 0;
}

int main() {
    // Set up server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    // Create a new socket for the server.
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    // Bind the socket to a port and address.
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    // Set the socket to listen for incoming connections.
    listen(server_socket, MAX_CLIENTS);

    // Set up client list
    struct Client clients[MAX_CLIENTS];
    int num_clients = 0;
    // Initialize an empty list of clients.

    // Accept client connections and start service threads
    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        struct Client client;
        recv(client_socket, client.name, BUFFER_SIZE, 0);
        client.socket = client_socket;
        add_client(clients, client);
        broadcast(clients, client_socket, "has joined the chat.");
        pthread_t thread;
        pthread_create(&thread, NULL, service_client, (void*) &client);
    }

    return 0;
}