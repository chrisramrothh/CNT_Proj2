all: client server

client: client.c
        gcc client.c -o client -lssl -lcrypto
        
server: server.c
        gcc server.c -o server -lssl -lcrypto
        
clean:
        rm client server
