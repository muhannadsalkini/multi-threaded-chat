/*
Group 10:
1910205563 - MUHANNAD SALKINI
2010205590 - MOHAMAD ZUBI
1810205559 - Ousman Abdoulaye Ahmat
1810205053 - Ismail Acar
*/

/*
compile:
  gcc -Wall -g3 -fsanitize=address -pthread client.c -o client
*/

// The program is runable on a machine tha have gcc installed on it.
// To open a new client from the terminal you need to type: ./client after compile the file

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
#define MAX_BINARY_LEN 9               // 8 bits per character + 1 for the null terminator
#define POLYNOMIAL 0b10001000000100001 // CRC-16-CCITT polynomial
#define POLYNOMIAL_LEN 17              // Length of the polynomial, in bits

// Global variables
volatile sig_atomic_t control_bit = 0; // Atomic variable for signaling
int sockfd = 0;                        // Socket descriptor
char name[32];                         // User's name

// Function to overwrite the current line in stdout with the "> " prompt
void str_overwrite_stdout()
{
  printf("%s", "> ");
  fflush(stdout);
}

// Trims the '\n' character from a string
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

// Converts a string to its binary ASCII representation
void convert_to_asci(char *str, int length)
{
  // Check for proper usage
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <string>\n", argv[0]);
    return 1;
  }

  // Get input string
  // char *str = argv[1];

  // Create array of strings to store binary ASCII representation
  char binary[strlen(str)][MAX_BINARY_LEN];

  // Convert string to binary ASCII representation
  for (int i = 0; i < strlen(str); i++)
  {
    char c = str[i];
    int idx = 0;
    for (int j = 7; j >= 0; j--)
    {
      binary[i][idx++] = (c >> j) & 1 ? '1' : '0';
    }
    binary[i][idx] = '\0';
  }

  // Print binary ASCII representation with '|' separator
  // for (int i = 0; i < strlen(str); i++)
  //{
  //  printf("%s|", binary[i]);
  //}

  // return binary;
  return 0;
}

// Applies parity check to an array of binary strings
void parity_check(char *binary_strings, num_strings)
{
  // Check for proper usage
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <binary strings>\n", argv[0]);
    return 1;
  }

  // Parse input binary strings
  // int num_strings = argc - 1;
  // char *binary_strings[num_strings];
  for (int i = 0; i < num_strings; i++)
  {
    binary_strings[i] = argv[i + 1];
  }

  // Apply parity check to each binary string
  for (int i = 0; i < num_strings; i++)
  {
    int num_ones = 0;
    for (int j = 0; j < strlen(binary_strings[i]); j++)
    {
      if (binary_strings[i][j] == '1')
      {
        num_ones++;
      }
    }

    // if (num_ones % 2 == 0)
    //{
    //   printf("%s has even parity\n", binary_strings[i]);
    // }
    // else
    //{
    //   printf("%s has odd parity\n", binary_strings[i]);
    // }
  }

  return 0;
}

// Applies cyclic redundancy check to an array of binary strings
void cyclic_redundancy_check(char *binary_strings, num_strings)
{
  // Check for proper usage
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <binary strings>\n", argv[0]);
    return 1;
  }

  // Parse input binary strings
  int num_strings = argc - 1;
  char *binary_strings[num_strings];
  for (int i = 0; i < num_strings; i++)
  {
    binary_strings[i] = argv[i + 1];
  }

  // Apply cyclic redundancy check to each binary string
  for (int i = 0; i < num_strings; i++)
  {
    char crc[POLYNOMIAL_LEN];        // Stores the calculated CRC
    strcpy(crc, "0000000000000000"); // Initialize the CRC to all zeros

    // Calculate the CRC
    for (int j = 0; j < strlen(binary_strings[i]); j++)
    {
      // Shift the CRC left by 1 bit
      for (int k = POLYNOMIAL_LEN - 1; k > 0; k--)
      {
        crc[k] = crc[k - 1];
      }

      // XOR the leftmost bit of the CRC with the next bit of the input string, if necessary
      if (crc[0] == '1')
      {
        for (int k = 0; k < POLYNOMIAL_LEN; k++)
        {
          crc[k] = crc[k] == polynomial[k] ? '0' : '1';
        }
      }

      // Shift the input string left by 1 bit
      binary_strings[i][j] = binary_strings[i][j + 1];
    }

    // Print the calculated CRC
    // printf("CRC for %s is %s\n", binary_strings[i], crc);
  }

  return 0;
}

// Function to catch the SIGINT signal (ctrl-c) and set the control_bit variable to 1
void catch_ctrl_c_and_exit(int sig)
{
  control_bit = 1;
}

// Function to write data to a file with the given file name
int write_to_file(char *file_name, char *data)
{
  FILE *fp; // Open the file in append mode
  char file_path[100];
  sprintf(file_path, "%s%s", "messages/", file_name); // create the file path by concatenating
  fp = fopen(file_path, "a");
  if (fp == NULL)
  {
    return -1;
  }

  // Write the data to the file and add a newline character
  fputs(data, fp);
  fputs("\n", fp);

  fclose(fp);
  return 0;
}

// Function to send a message
void send_msg()
{
  char message[SIZE] = {};     // create a buffer to store the message
  char buffer[SIZE + 32] = {}; // create a buffer to store the message along with additional

  // Loop until the user types "exit"
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

      // add the message to the buffer along with additional information and send it to the server
      sprintf(buffer, "%s\n", message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

    // clear the buffers
    bzero(message, SIZE);
    bzero(buffer, SIZE + 32);
  }
  catch_ctrl_c_and_exit(2); // catch the SIGINT signal (ctrl-c) and set the control_bit variable to
}

// Function to receive a message
void recv_msg()
{
  char message[SIZE] = {}; // create a buffer to store the received message
  while (1)
  {
    int receive = recv(sockfd, message, SIZE, 0); // receive a message from the socket

    // if the message was received successfully
    if (receive > 0)
    {
      str_trim_lf(message, 3000); // remove the '\n' character from the end of
      printf("%s\n", message);    // print the message to stdout

      str_overwrite_stdout(); // overwrite the current line in stdout with the "> " prompt
    }
    // if the connection was closed by the other side
    else if (receive == 0)
    {
      break;
    }
    // if there was an error in receiving the message
    else
    {
      printf("Handle Error!");
    }
    memset(message, 0, sizeof(message)); // reset the message buffer
  }
}

int main(int argc, char **argv)
{
  char *ip = "127.0.0.1"; // the IP address of the server
  int port = 1234;

  signal(SIGINT, catch_ctrl_c_and_exit); // register signal handler for Ctrl-C

  printf("Enter your chat name: ");
  fgets(name, 22, stdin);
  str_trim_lf(name, strlen(name));

  if (strlen(name) > 22 || strlen(name) < 2)
  {
    printf("Name must be between 2-20 characters!\n");
    return EXIT_FAILURE;
  }

  struct sockaddr_in server_addr;

  // Socket settings
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
  pthread_create(&sent_thread, NULL, (void *)send_msg, NULL); // create a new thread to send messages

  pthread_t recved_thread;
  pthread_create(&recved_thread, NULL, (void *)recv_msg, NULL); // create a new thread to receive messages

  while (1)
  {
    if (control_bit == 1)
    {
      printf("\nExited!\n");
      break;
    }
  }

  close(sockfd); // close

  return 0;
}
