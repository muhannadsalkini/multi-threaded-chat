/*
Group 10:
1910205563 - MUHANNAD SALKINI
2010205590 - MOHAMAD ZUBI
1810205559 - Ousman Abdoulaye Ahmat
1810205053 - Ismail Acar
*/

/*
compile:
	gcc -Wall -g3 -fsanitize=address -pthread server.c -o server
*/

// The program is runable on a machine tha have gcc installed on it.
// To run the server side from the terminal you need to type: ./server after compile the file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#define FORMAT "-> %s hello"
#define MAX_CLIENTS 20
#define BUFFER_SIZE 1024

static _Atomic unsigned int total_clients = 0; // total_clients is an atomic variable that can be modified concurrently
static int uid = 10;						   // uid is a global variable that stores the next available client ID
char global_name[32];						   // global_name is a global character array that stores the name of the client

// client_t is a structure that stores the client's address, socket descriptor, ID, and name
typedef struct
{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS]; // clients is an array of pointers to client_t structures

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// str_overwrite_stdout overwrites the current line in stdout with "> "
void str_overwrite_stdout()
{
	printf("\r%s", "> ");
	fflush(stdout);
}

// str_trim_lf trims the '\n' character from the end of a character array
void str_trim_lf(char *arr, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{ // trim \n
		if (arr[i] == '\n')
		{
			arr[i] = '\0';
			break;
		}
	}
}

// CONN_add_client adds a client to the clients array
void CONN_add_client(struct sockaddr_in addr)
{
	// CONN
	printf("%d.%d.%d.%d",
		   addr.sin_addr.s_addr & 0xff,
		   (addr.sin_addr.s_addr & 0xff00) >> 8,
		   (addr.sin_addr.s_addr & 0xff0000) >> 16,
		   (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

// CONN_add_client_que adds a client to the clients array in a thread-safe manner using a mutex
void CONN_add_client_que(client_t *cl)
{
	// CONN
	pthread_mutex_lock(&clients_mutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!clients[i])
		{
			clients[i] = cl;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

// CONN_send_join_message sends a message to all clients except the sender
void CONN_send_join_message(char *s, int uid)
{
	// CONN
	pthread_mutex_lock(&clients_mutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i])
		{
			if (clients[i]->uid != uid)
			{
				if (write(clients[i]->sockfd, s, strlen(s)) < 0)
				{
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

// CONN_print_clients_list prints the list of active clients
void CONN_print_clients_list(struct sockaddr_in *head)
{
	// CONN
	int i = 0;
	while (head != NULL)
	{
		printf("%d => %d ", i, head->data);
		head = head->next;
	}
	printf("\n");
}
// MESG_send_message sends a message to the specified client
void MESG_send_message(char *s, int uid)
{
	// MESG
	pthread_mutex_lock(&clients_mutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i])
		{
			if (clients[i]->uid == uid)
			{
				if (write(clients[i]->sockfd, s, strlen(s)) < 0)
				{
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

// MERR_send_message_without_error sends a message to the specified client without checking for errors
void MERR_send_message_without_error(char *s, int uid)
{
	// MERR
	pthread_mutex_lock(&clients_mutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i])
		{
			if (clients[i]->uid == uid)
			{
				write(clients[i]->sockfd, s, strlen(s));
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

// GONE_client_remove_que removes a client from the clients array in a thread-safe manner using a mutex
void GONE_client_remove_que(int uid)
{
	// GONE
	pthread_mutex_lock(&clients_mutex);

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i])
		{
			if (clients[i]->uid == uid)
			{
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

// find_Client searches the clients array for a client with the specified name and returns the client's ID
int find_Client(char *s)
{
	char output[32];
	char format[20];
	sprintf(format, FORMAT, "%s");
	sscanf(s, format, output);

	strcpy(global_name, output);
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (clients[i])
		{
			if (strcmp(clients[i]->name, output) == 0)
			{
				return clients[i]->uid;
			}
		}
	}
	return -1;
}

// write_to_file writes the given data to a file with the specified name
int write_to_file(char *file_name, char *data)
{
	FILE *fp;
	char file_path[100];
	sprintf(file_path, "%s%s", "messages/", file_name);
	fp = fopen(file_path, "a");
	if (fp == NULL)
	{
		return -1;
	}
	str_trim_lf(data, strlen(data));
	fputs(data, fp);
	fputs("\n", fp);

	fclose(fp);
	return 0;
}

// service_client handles all communication with a client
void *service_client(void *arg)
{
	char buff_out[BUFFER_SIZE];

	char name[32];
	int leave_flag = 0;

	total_clients++;
	client_t *cli = (client_t *)arg;

	// Name
	if (recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) < 2 || strlen(name) >= 32 - 1)
	{
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	}
	else
	{
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		CONN_send_join_message(buff_out, cli->uid);
	}

	bzero(buff_out, BUFFER_SIZE);

	while (1)
	{
		if (leave_flag)
		{
			break;
		}

		int receive = recv(cli->sockfd, buff_out, BUFFER_SIZE, 0);
		int a = find_Client(buff_out);
		if (receive > 0)
		{
			if (strlen(buff_out) > 0)
			{
				str_trim_lf(buff_out, strlen(buff_out));
				if (a == -1)
				{
					// do nothing or continue;
				}
				else
				{
					char result[4444];
					sprintf(result, "%s %s\n", cli->name, buff_out);
					MESG_send_message(result, a);
					int result2 = write_to_file(global_name, result);
					if (result2 == 0)
					{
						printf("Data successfully written to file.\n");
					}
					else
					{
						printf("Error writing to file.\n");
					}

					str_trim_lf(buff_out, strlen(buff_out));
					printf("%s %s\n", cli->name, buff_out);
				}
			}
		}
		else if (receive == 0 || strcmp(buff_out, "exit") == 0)
		{
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			CONN_send_join_message(buff_out, cli->uid);
			leave_flag = 1;
		}
		else
		{
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SIZE);
	}

	// Delete client from queue and yield thread
	close(cli->sockfd); // close
	GONE_client_remove_que(cli->uid);
	free(cli);
	total_clients--;
	pthread_detach(pthread_self());

	return NULL;
}

int main(int argc, char **argv)
{
	char *ip = "127.0.0.1";
	int port = 1234;
	int option = 1;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	pthread_t tid;

	// Socket settings
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);

	// Ignore pipe signals
	signal(SIGPIPE, SIG_IGN);

	if (setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&option, sizeof(option)) < 0)
	{
		perror("ERROR: setsockopt failed");
		return EXIT_FAILURE;
	}

	// Bind
	if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR: Socket binding failed");
		return EXIT_FAILURE;
	}

	// Listen
	if (listen(listenfd, 10) < 0)
	{
		perror("ERROR: Socket listening failed");
		return EXIT_FAILURE;
	}

	printf("Lunched succsesfuly!\n");

	while (1)
	{
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen);

		// Check if max clients is reached
		if ((total_clients + 1) == MAX_CLIENTS)
		{
			printf("Max clients reached. Rejected: ");
			CONN_add_client(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		// Client settings
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		// Add client to the queue and fork thread
		CONN_add_client_que(cli);
		pthread_create(&tid, NULL, &service_client, (void *)cli);

		// Reduce CPU usage
		sleep(1);
	}

	return EXIT_SUCCESS;
}
