/*
To run a new client you need to open a new terminal and type: ./client 4444


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>

#define SIZE 1024

// Global variables
volatile sig_atomic_t control_bit = 0;
int sockfd = 0;
char name[32];

void str_overwrite_stdout()
{
  printf("%s", "> ");
  fflush(stdout);
}

// !!Remove this if not effecting the code
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

void convert_to_asci(char *arr, int length)
{
  //
}

void parity_check(char *arr, int length)
{
  //
}

void cyclic_redundancy_check(char *arr, int length)
{
  //
}

void catch_ctrl_c_and_exit(int sig)
{
  control_bit = 1;
}

void send_msg()
{
  char message[SIZE] = {};
  char buffer[SIZE + 32] = {};

  while (1)
  {
    str_overwrite_stdout();
    fgets(message, SIZE, stdin);
    str_trim_lf(message, SIZE);

    if (strcmp(message, "exit") == 0)
    {
      break;
    }
    else
    {
      // file logs
      int result2 = write_to_file(name, message);
      if (result2 == 0)
      {
        // printf("Data successfully written to file.\n");
      }
      else
      {
        printf("Error writing to file.\n");
      }

      sprintf(buffer, "%s\n", message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

    bzero(message, SIZE);
    bzero(buffer, SIZE + 32);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_msg()
{
  char message[SIZE] = {};
  while (1)
  {
    int receive = recv(sockfd, message, SIZE, 0);

    if (receive > 0)
    {
      str_trim_lf(message, 3000); // hata verdirtme ihtimali olan bir satır..
      printf("%s\n", message);

      str_overwrite_stdout();
    }
    else if (receive == 0)
    {
      break;
    }
    else
    {
      printf("Handle Error!");
    }
    memset(message, 0, sizeof(message));
  }
}

int write_to_file(char *file_name, char *data)
{
  FILE *fp;
  char file_path[100];
  sprintf(file_path, "%s%s", "messages/", file_name);
  fp = fopen(file_path, "a"); // "a" modu ile dosya açılır ve veriler dosyanın sonuna eklenir
  if (fp == NULL)
  {
    // Dosya açılamadı, hata döndür
    return -1;
  }

  // Dosya açıldı, verileri yaz
  fputs(data, fp);
  fputs("\n", fp);

  // Dosyayı kapat ve başarıyla tamamlanan işlemi belirtmek için 0 döndür
  fclose(fp);
  return 0;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Port No: %s <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *ip = "127.0.0.1";
  int port = atoi(argv[1]);

  signal(SIGINT, catch_ctrl_c_and_exit);

  printf("Enter your chat name: ");
  fgets(name, 22, stdin);
  str_trim_lf(name, strlen(name));

  if (strlen(name) > 22 || strlen(name) < 2)
  {
    printf("Name must be between 2-20 characters!\n");
    return EXIT_FAILURE;
  }

  struct sockaddr_in server_addr;

  /* Socket settings */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);

  // Connect to Server
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1)
  {
    printf("ERROR: connect\n");
    return EXIT_FAILURE;
  }

  // Send name
  send(sockfd, name, 32, 0);

  printf("Entered to The Chat Room.\n");

  pthread_t sent_thread;
  pthread_create(&sent_thread, NULL, (void *)send_msg, NULL);
  /* OLD CODE. REMOVE BEFROE SEND THE CODE!!
  if (pthread_create(&sent_thread, NULL, (void *)send_msg, NULL) != 0)
  {
    printf("ERROR: pthread\n");
    return EXIT_FAILURE;
  }
  */

  pthread_t recved_thread;
  pthread_create(&recved_thread, NULL, (void *)recv_msg, NULL);
  /* OLD CODE. REMOVE BEFROE SEND THE CODE!!
  if (pthread_create(&recved_thread, NULL, (void *)recv_msg, NULL) != 0)
  {
    printf("ERROR: pthread\n");
    return EXIT_FAILURE;
  }
  */

  while (1)
  {
    if (control_bit)
    {
      printf("\nExited!\n");
      break;
    }
  }

  close(sockfd);

  return EXIT_SUCCESS;
}
