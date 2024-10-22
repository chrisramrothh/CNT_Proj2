#include <stdio.h>        /* for printf() and fprintf() */
#include <sys/socket.h>   /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>       /* supports all sorts of functionality */
#include <unistd.h>       /* for close() */
#include <string.h>       /* support any string ops */
#include <openssl/evp.h>  /* for OpenSSL EVP digest libraries/SHA256 */  
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <zlib.h>  // For CRC32

int compareFile(FILE *file1, FILE *file2, int *line, int *col);
void *handle_connection(void *p_client_sock);
void handle_diff(int client_sock);
void handle_pull(int client_sock);

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
    printf("Listening...\n");

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
    int client_sock = *((int *)p_client_sock);
    free(p_client_sock);
    char buffer[1024];

    // Send welcome message from server
    bzero(buffer, 1024);
    strcpy(buffer, "Hello from the server.");
    send(client_sock, buffer, strlen(buffer), 0);
    printf("Sent welcome message.\n");

    // Receive welcome message from client
    bzero(buffer, sizeof(buffer));
    recv(client_sock, buffer, sizeof(buffer), 0);
    printf("Client: %s\n", buffer);

    while (1) {
        bzero(buffer, 1024);
        int recv_bytes = recv(client_sock, buffer, 1024, 0);

        if (recv_bytes <= 0) {
            printf("Client disconnected or error receiving data.\n");
            break;
        }

        printf("Client: %s\n", buffer); 

        // Handle LIST, DIFF, PULL, LEAVE
        if (strcmp(buffer, "1") == 0) {
            printf("Listing files.\n");
            DIR *dir = opendir("server_files");
            struct dirent *entry;
            char file_list[1024] = "";

            while ((entry = readdir(dir)) != NULL) {
                strcat(file_list, entry->d_name);
                strcat(file_list, "\n");
            }
            closedir(dir);
            send(client_sock, file_list, strlen(file_list), 0);
            printf("Sent file list to client.\n");
        }
        else if (strcmp(buffer, "2") == 0) {
            printf("Performing diff.\n");
            handle_diff(client_sock);
            printf("Diff completed.\n");
        }
        else if (strcmp(buffer, "3") == 0) {
            printf("Performing pull.\n");
            handle_pull(client_sock);
            printf("Pull completed.\n");
        }
        else if (strcmp(buffer, "4") == 0) {
            printf("Client has left.\n");
            break;
        } else {
            printf("Invalid input received: %s\n", buffer);
        }
    }
    close(client_sock);
    return NULL;
}
// Function to compute CRC32 checksum of a file
unsigned long compute_crc32(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return 0;

    unsigned char buffer[1024];
    size_t bytesRead = 0;
    unsigned long crc = crc32(0L, Z_NULL, 0);

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        crc = crc32(crc, buffer, bytesRead);
    }

    fclose(file);
    return crc;
}

// DIFF
void handle_diff(int client_sock) {
    DIR *dir = opendir("server_files");
    struct dirent *entry;
    char buffer[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "server_files/%.240s", entry->d_name);

        // Compute CRC32 checksum of the file
        unsigned long checksum = compute_crc32(filepath);

        // Send file name and checksum to the client
        snprintf(buffer, sizeof(buffer), "%s %lu\n", entry->d_name, checksum);
        printf("Sending: %s (checksum: %lu)\n", entry->d_name, checksum);
        send(client_sock, buffer, strlen(buffer), 0);
    }

    // Send "DONE" to indicate the end of the file list
    send(client_sock, "DONE", 5, 0);
    closedir(dir);
}


// PULL 
void handle_pull(int client_sock) {
    char filename[256];
    char buffer[1024];
    int bytes_read;

    // Receive file requests in a loop
    while (1) {
        bzero(filename, sizeof(filename));
        int recv_bytes = recv(client_sock, filename, sizeof(filename), 0);

        if (recv_bytes <= 0 || strcmp(filename, "DONE") == 0) {
            break;
        }

        printf("Received file request: %s\n", filename);

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "server_files/%.240s", filename);
        FILE *file = fopen(filepath, "rb");

        if (file) {
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                send(client_sock, buffer, bytes_read, 0);
            }
            fclose(file);
            printf("File %s sent successfully.\n", filename);
        } else {
            printf("File %s not found on server.\n", filename);
            send(client_sock, "ERROR: File not found", 21, 0);
        }
    }
}
