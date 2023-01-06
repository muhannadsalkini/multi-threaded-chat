#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Define a structure to hold message information
struct Message {
    char sender[BUFFER_SIZE];
    char receiver[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char error_checking_bits[BUFFER_SIZE];
};

// Function to receive chat messages in a separate thread
void* receive_chat(void* socket_ptr) {
    // Cast socket_ptr to int type and store in variable socket
    int socket = ((int) socket_ptr);
    // Create a buffer to store received messages
    char buffer[BUFFER_SIZE];

    // Receive messages until the connection is closed
    while (1) {
        // Receive message from the server
        int received = recv(socket, buffer, BUFFER_SIZE, 0);
        // If received <= 0, the connection has been closed
        if (received <= 0) {
            break;
        }

        // Parse message
        struct Message message;
        // Read fields from buffer into message struct using sscanf
        sscanf(buffer, "%s|%s|%s|%s", message.sender, message.receiver, message.message, message.error_checking_bits);

        if (strcmp(message.sender, "CONN") == 0) {
            // Print list of current chat clients
            printf("Current chat clients: %s\n", message.message);
        } else if (strcmp(message.sender, "MESG") == 0) {
            // Print message
            printf("%s: %s\n", message.sender, message.message);
        } else if (strcmp(message.sender, "MERR") == 0) {
            // Print error message
            printf("Error detected in previous message.\n");
        } else if (strcmp(message.sender, "GONE") == 0) {
            // Print message about client leaving chat
            printf("%s has left the chat.\n", message.message);
        }
    }

    // Close socket and return from thread
    close(socket);
    return NULL;
}

// Function to send chat messages
void send_chat(int socket, char* name, char* receiver, char* message, char* error_checking_bits) {
    // Compose message in the correct format
    char buffer[BUFFER_SIZE];
    // Format the message using sprintf and store it in the buffer
    sprintf(buffer, "MESG|%s|%s|%s|%s", name, receiver, message, error_checking_bits);
    // Send message
    send(socket, buffer, strlen(buffer), 0);
}

int main() {
    // Create a buffer to store the user's name
    char name[BUFFER_SIZE];
    // Read user's name
    printf("Enter your name: ");
    scanf("%s", name);

    // Connect to server
    int socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    connect(socket, (struct sockaddr*) &server_address, sizeof(server_address));
    send(socket, name, strlen(name), 0);

    // Start receive thread
    pthread_t thread;
    pthread_create(&thread, NULL, receive_chat, (void*) &socket);

    // Read input and send chat
    char receiver[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char error_checking_bits[BUFFER_SIZE];
    while (1) {
        printf("Enter the receiver name: ");
        scanf("%s", receiver);
        printf("Enter your message: ");
        scanf("%s", message);
        printf("Enter error checking bits: ");
        scanf("%s", error_checking_bits);
        send_chat(socket, name, receiver, message, error_checking_bits);
    }

    return 0;

}
