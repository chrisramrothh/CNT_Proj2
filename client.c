#include <stdio.h>        /* for printf() and fprintf() */
#include <sys/socket.h>   /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>       /* supports all sorts of functionality */
#include <unistd.h>       /* for close() */
#include <string.h>       /* support any string ops */
#include <openssl/evp.h>  /* for OpenSSL EVP digest libraries/SHA256 */

int main(){
        char *ip = "127.0.0.1";
        int port = 5566;

        int sock;
        struct sockaddr_in addr;
        socklen_t addr_size;
        char buffer[1024];
        int n;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
                perror("[-]Socket error");
                exit(1);
        }
        printf("[+]TCP server socket created.\n");

        memset(&addr, '\0', sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = port;
        addr.sin_addr.s_addr = inet_addr(ip);

        // CONNECT
        connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        printf("Connected to the server");

        // SEND FIRST SOCK
        bzero(buffer, 1024);
        strcpy(buffer, "HELLO, THIS IS CLIENT.\n");
        printf("Client: %s\n", buffer);
        send(sock, buffer, strlen(buffer), 0);

        // RECIEVE CONFIRMATION SOCK
        bzero(buffer, 1024);
        recv(sock, buffer, sizeof(buffer), 0);
        printf("Server: %s\n", buffer);

        // LOOP WHILE INPUT IS NOT 4
        // CLIENT WILL SEND "LEAVE" WHEN INPUT IS 4
        while (strcmp(buffer, "LEAVE") !=0)
        {
                char input[2];
                scanf("%s", input);


                bzero(buffer, 1024);
                strcpy(buffer, input);
                printf("ClientTEST: %s\n", buffer);
                send(sock, buffer, strlen(buffer), 0);

                bzero(buffer, 1024);
                recv(sock, buffer, sizeof(buffer), 0);
                printf("Server: %s\n", buffer);
        }
        
        // CLIENT HAS SENT "LEAVE", DISCONNECT
        close(sock);
        printf("Disconnected from the server.\n");

        return 0;
}