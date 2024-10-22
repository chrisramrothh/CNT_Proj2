all: client server

client: client.c
	gcc client.c -o client -lssl -lcrypto -lz

server: server.c
	gcc server.c -o server -lssl -lcrypto -lz

clean:
	rm client server