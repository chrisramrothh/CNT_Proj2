# CNT4007 Project 2 - Christopher Ramroth, Connor Devaney

This project implements a server that hosts a directory of files and allows clients to connect to the server to:
- Request a list of the files the server has.
- Request a diff between the server's files and the client's files based on content (using CRC32 checksums).
- Pull the files listed in the diff to the client.
- The server uses TCP connections and allows concurrent clients using `pthreads`.

### Language Used: C

## Installation

1. **Update the system and install dependencies**:
   Run the following commands to ensure your system is up-to-date and has the required libraries for compiling the project:
   
   ```bash
   sudo apt update
   sudo apt install build-essential zlib1g-dev
   ```

2. **Download the project files**:
   Download `server.c`, `client.c`, `makefile`, `server_files`, and `client_files`.

3. **Compile the project**:
   Run `make` to create the executables for the server and client:
   
   ```bash
   make
   ```

4. **Clean up (optional)**:
   To remove the executables, you can run:
   
   ```bash
   make clean
   ```

## Usage

1. **Start the server**: 
   ```bash
   ./server
   ```

2. **Start a client**: 
   ```bash
   ./client
   ```

Once the connection is established and confirmation messages are exchanged between the server and client, the client can select an option by inputting a number from 1-4 in the command-line interface.

### Client Command Options:

- **1 - LIST**: Request a list of the server's files.
- **2 - DIFF**: Request a comparison of files between the server and client using CRC32 checksums (missing or different files will be listed).
- **3 - PULL**: Pull the files listed in the diff from the server to the client.
- **4 - LEAVE**: Close the connection between the client and server.

### Closing the Server:

When you're done, you can close the server by using `CTRL+C` in the command-line interface.

### Features Implemented:

- **LIST**: Clients can request a list of the files stored on the server.
- **DIFF**: Clients can request a diff between their local files and the server's files, based on content comparison using CRC32 checksums.
- **PULL**: Clients can request to pull missing or different files from the server based on the result of the diff operation.

---

This should cover both the dependencies installation and the usage of the project. Let me know if this looks good or if you want any other modifications!