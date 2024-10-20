# CNT4007 Project2 - Christopher Ramroth, Connor Lavado Devaney

Server that contains a directory of files, and clients can connect to the server, request a list of the files that server has, request a diff of the server files compared to the client's files, and pull all files listed in the diff to the client. Uses a TCP connection and allows for concurrent connecting clients using pthreads.

Language Used: C

## Installation

- Download server.c, client.c, makefile, server_files, and client_files
- Run "make" to make the executables for server and client
- Can run "make clean" to remove the exeucutables for server and client

## Usage

- Use ./server to start the server
- Use ./client to start a client

##### Once connected and the confirmation messages are sent between the server and client, client can select an input 1-4 in the command-line interface.

- 1 - LIST (request list of server's files)
- 2 - DIFF (request list of files differing between server and client) // Not functional yet
- 3 - PULL (pulls the files listed in diff to the client) // Not functional yet
- 4 - LEAVE (closes connection between client and server)

##### If you are done using the server, close by using CTRL+C in the command-line interface.

