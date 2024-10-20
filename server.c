#include <stdio.h>        /* for printf() and fprintf() */
#include <sys/socket.h>   /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>       /* supports all sorts of functionality */
#include <unistd.h>       /* for close() */
#include <string.h>       /* support any string ops */
#include <openssl/evp.h>  /* for OpenSSL EVP digest libraries/SHA256 */
#include <dirent.h>
#include <pthread.h>

int compareFile(FILE *file1, FILE *file2, int *line, int*col);
void *handle_connection(void *p_client_sock);

int main()
{
        char *ip = "127.0.0.1";
        int port = 5566;

        int server_sock, client_sock;
        struct sockaddr_in server_addr, client_addr;
        socklen_t addr_size;
        int n;

        server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0)
        {
                perror("[-]Socket error");
                exit(1);
        }
        printf("[+]TCP server socket created.\n");

        memset(&server_addr, '\0', sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = port;
        server_addr.sin_addr.s_addr = inet_addr(ip);

        n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (n < 0)
        {
                perror("[-]Bind error");
                exit(1);
        }
        printf("[+]Bind to the port number: %d\n", port);

        listen(server_sock, 100);
        printf("Listening...");

        while(1)
        {
                // CONNECT
                addr_size = sizeof(client_addr);
                client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
                printf("[+]Client connected.\n");

                // create pthread to handle multiple connections efficiently
                pthread_t t;
                int *pclient = malloc(sizeof(int));
                *pclient = client_sock;
                pthread_create(&t, NULL, handle_connection, pclient);
                // pthread sent to handle_connection function
        }



        return 0;
}





void *handle_connection(void *p_client_sock)
{
        // turn pointer into int and free the pointer
        int client_sock = *((int*)p_client_sock);
        free(p_client_sock);

        // used for server file directory
        DIR *d;
        struct dirent *dir;

        char buffer[1024];
        // RECIEVE FIRST SOCK
        bzero(buffer, 1024);
        recv(client_sock, buffer, sizeof(buffer), 0);
        printf("Client: %s\n", buffer);

        // SEND CONFIRMATION SOCK
        bzero(buffer, 1024);
        strcpy(buffer, "HI, THIS IS SERVER\n\nTYPE INPUT 1-4\n1. LIST\n2. DIFF\n3. PULL\n4. LEAVE\n");
        printf("Server: %s\n", buffer);
        send(client_sock, buffer, strlen(buffer), 0);

        // RECIEVE FIRST INPUT
        bzero(buffer, 1024);
        recv(client_sock, buffer, sizeof(buffer), 0);
        printf("Client: %s\n\n", buffer);

        // LOOP WHILE INPUT IS NOT 4 FOR LEAVE
        while (strcmp(buffer, "4") != 0)
        {
                if (strcmp(buffer, "1") == 0)
                {
                        // LIST
                        char fileList[1024];
                        strcpy(fileList, "Server File List:");
                        d = opendir("server_files");
                        if (d == NULL)
                        {
                                printf("Missing directory for server files");
                        }
                        else if (d)
                        {
                                while ((dir = readdir(d)) != NULL)
                                {
                                        strcat(fileList, "\n");
                                        strcat(fileList, dir->d_name);
                                }
                                strcat(fileList, "\n\n");
                                // SEND FILE LIST
                                bzero(buffer, 1024);
                                strcpy(buffer, fileList);
                                printf("Server: Sending Files\n\n");
                                send(client_sock, buffer, strlen(buffer), 0);

                                // RECIEVE CONFIRMATION
                                bzero(buffer, 1024);
                                recv(client_sock, buffer, sizeof(buffer), 0);
                                printf("Client: %s\n", buffer);
                         }
                }
                else if (strcmp(buffer, "2") == 0)
                {
                        // DIFF
                }
                else if (strcmp(buffer, "3") == 0)
                {
                        // PULL
                }

                // ASK CLIENT FOR INPUT
                bzero(buffer, 1024);
                strcpy(buffer, "TYPE INPUT 1-4\n1. LIST\n2. DIFF\n3. PULL\n4. LEAVE\n");
                printf("Server: %s\n", buffer);
                send(client_sock, buffer, strlen(buffer), 0);

                // RECIEVE INPUT
                bzero(buffer, 1024);
                recv(client_sock, buffer, sizeof(buffer), 0);
                printf("Client: %s\n\n", buffer);

        }

         // INPUT IS 4, DISCONNECT AND CLOSE DIRECTORY
        closedir(d);

        bzero(buffer, 1024);
        strcpy(buffer, "LEAVE\n");
        printf("Server: %s\n", buffer);
        send(client_sock, buffer, strlen(buffer), 0);
        close(client_sock);
        printf("[+]Client disconnected.\n\n");

        return NULL;
}

// FUNCTION TO COMPARE TWO FILES (RETURNS 0 IF THE SAME, -1 IF DIFFERENT)
// UNSURE WHETHER IT WILL BE IMPLEMENTED IN SERVER OR CLIENT
int compareFile(FILE *file1, FILE *file2, int *line, int *col)
{
        char ch1, ch2;
        *line = 1;
        *col = 0;

        do
        {
                ch1 = fgetc(file1);
                ch2 = fgetc(file2);

                if (ch1 == '\n')
                {
                        *line += 1;
                        *col = 0;
                }

                if (ch1 != ch2)
                {
                        return -1;
                }
                *col += 1;
        } while (ch1 != EOF && ch2 != EOF);

        if (ch1 == EOF && ch2 == EOF)
        {
                return 0;
        }
        else
        {
                return -1;
        }
}
