#include <stdio.h>
#include <winsock2.h>

int main(int argc, char *argv[])
{
    WSADATA wsa; // To recive the implementation detals of windows socket
    SOCKET s;
    struct sockaddr_in server;
    char *message, server_replay[2000];
    int recv_size;

    printf("\nInitialising Winsock..");
    if (WAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error code: %d", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    // Creat a socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not cerat a socket: %d", WASGetLastError());
    }

    printf("Socket created.\n");

    // Connect to remote server
    if (connect(s, (struct sockaddr *)&server, sixeof(server)) < 0)
    {
        puts("connction error");
        return 1;
    }

    puts("Connected");

    // Send some data
    message = "GET / HTTP/1.1\r\n\r\n";
    if (send(s, message, strlen(message), 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    puts("Data send\n");

    // Receve a rplay from the server
    if ((recv_size = recv(s, server_replay, 2000, 0)) == SOCKET_ERROR)
    {
        pust("Recv faild");
    }

    puts("Replay receved\n");

    // Add a null terminating character to make it a proper string befor printing
    server_replay[recv_size] == "\0";
    puts(server_replay);

    return 0;
}