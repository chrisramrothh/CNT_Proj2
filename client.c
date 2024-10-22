#include <stdio.h>        /* for printf() and fprintf() */
#include <sys/socket.h>   /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>       /* supports all sorts of functionality */
#include <unistd.h>       /* for close() */
#include <string.h>       /* support any string ops */
#include <zlib.h>         // For CRC32
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>     /* for file information like size */


void handle_diff(int sock, char missing_files[][256], int *num_files);
void request_pull(int sock, char missing_files[][256], int num_files);

int main() {
    char *ip = "127.0.0.1";
    int port = 5566;
    int sock;
    struct sockaddr_in addr;
    char buffer[1024];

    // array to store missing/different files and file count
    char missing_files[100][256];
    int num_files = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP client socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);

    // CONNECT
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    printf("Connected to the server\n");

    // Receive welcome message from server
    bzero(buffer, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    // Send welcome message to server
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "Hello from the client.");
    send(sock, buffer, strlen(buffer), 0);


    while (1) {
        // INPUT MENU
        printf("\nTYPE INPUT 1-4\n");
        printf("1. LIST\n");
        printf("2. DIFF\n");
        printf("3. PULL\n");
        printf("4. LEAVE\n");

        // INPUT 1-4
        char input[2];
        scanf("%s", input);

        bzero(buffer, sizeof(buffer));
        strcpy(buffer, input);
        printf("Client Input: %s\n", buffer);  
        send(sock, buffer, strlen(buffer), 0);

        // LIST
        if (strcmp(input, "1") == 0) {
            bzero(buffer, sizeof(buffer));
            recv(sock, buffer, sizeof(buffer), 0);
            printf("Server: \n%s", buffer);
            printf("Client: Received File List Confirmation.\n");
        }

        // DIFF (store missing/different files)
        else if (strcmp(input, "2") == 0) {
            printf("Requesting diff...\n");
            handle_diff(sock, missing_files, &num_files);
            printf("Diff complete. %d file(s) missing or different.\n", num_files);
        }

        // PULL (pull the files identified by DIFF)
        else if (strcmp(input, "3") == 0) {
            if (num_files > 0) {
                printf("Requesting pull for missing/different files...\n");
                request_pull(sock, missing_files, num_files);
                printf("Pulled missing file(s).\n");
            } else {
                printf("No files to pull.\n");
            }
        }

        // LEAVE
        else if (strcmp(input, "4") == 0) {
            printf("Client is leaving.\n");
            break;
        }

        // invalid input
        else {
            printf("Invalid input. Please try again.\n");
        }
    }

    close(sock);
    printf("Disconnected from the server.\n");
    return 0;
}

void trim_whitespace(char *str) {
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
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
void handle_diff(int sock, char missing_files[][256], int *num_files) {
    char buffer[1024];
    char client_filename[256];
    struct stat file_stat;
    *num_files = 0;

    while (1) {
        
        bzero(buffer, sizeof(buffer));
        int recv_bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (recv_bytes <= 0) {
            printf("Connection closed or error receiving data.\n");
            break;
        }

        // Trim any trailing whitespace
        trim_whitespace(buffer);

        // Check if the server sent "DONE"
        if (strstr(buffer, "DONE") != NULL) {
            printf("Diff operation complete, received DONE signal.\n");
            break;
        }

        // Parse the server file name and checksum
        char server_filename[256];
        unsigned long server_checksum;
        sscanf(buffer, "%s %lu", server_filename, &server_checksum);
        printf("Received file info from server: %s (checksum: %lu)\n", server_filename, server_checksum);

        // Construct the path for the local file in client_files directory
        snprintf(client_filename, sizeof(client_filename), "client_files/%.240s", server_filename);

        // Compute checksum of the local file if it exists
        if (stat(client_filename, &file_stat) == 0) {
            unsigned long local_checksum = compute_crc32(client_filename);

            // Compare checksums
            if (local_checksum != server_checksum) {
                printf("File different (based on checksum): %s\n", server_filename);
                strcpy(missing_files[*num_files], server_filename);
                (*num_files)++;
            } else {
                printf("File matches: %s\n", server_filename);
            }
        } else {
            // File doesn't exist locally
            printf("File missing: %s\n", server_filename);
            strcpy(missing_files[*num_files], server_filename);
            (*num_files)++;
        }
    }
}


// PULL
void request_pull(int sock, char missing_files[][256], int num_files) {
    char buffer[1024];
    int bytes_received;

    if (num_files == 0) {
        printf("No missing files to pull.\n");
        return;
    }

    for (int i = 0; i < num_files; i++) {
        // Send file request to server for each missing/different file
        printf("Requesting file: %s\n", missing_files[i]);
        send(sock, missing_files[i], strlen(missing_files[i]), 0);

        // Receive and save the file
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "client_files/%.240s", missing_files[i]);
        FILE *file = fopen(filepath, "wb");

        if (file) {
            // Receive the file data in chunks
            while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
                fwrite(buffer, 1, bytes_received, file);
                // Break when done
                if (bytes_received < sizeof(buffer)) {
                    break;
                }
            }
            fclose(file);
            printf("Pulled missing file: %s\n", missing_files[i]);
        }
    }
}