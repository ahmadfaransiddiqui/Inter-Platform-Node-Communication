#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Shared memory variables
char sharedMemory[BUFFER_SIZE] = "";  // Shared memory for data
int sharedMemoryReady = 0;  // Flag to indicate if shared memory is ready

// Function to write to shared memory
void writeToSharedMemory(char* data) {
    if (sharedMemoryReady) {
        strcpy(sharedMemory, data);
        printf("Writing to shared memory: %s\n", data);
    }
}

// Function to read from shared memory
char* readFromSharedMemory() {
    if (sharedMemoryReady) {
        if (strlen(sharedMemory) > 0) {
            printf("Reading from shared memory: %s\n", sharedMemory);
            return sharedMemory;
        } else {
            return "The cache line is invalid. Data is not present in the cache.";
        }
    } else {
        return "Shared memory not ready.";
    }
}



int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "Failed to initialize Winsock\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        fprintf(stderr, "Binding failed\n");
        return 1;
    }

    listen(serverSocket, 1);
    printf("Node A is listening on Port %d\n", PORT);

    int clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Accepting connection failed\n");
        return 1;
    }

    printf("Node B connected\n");

    // Set shared memory as ready
    sharedMemoryReady = 1;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesRead <= 0) {
            fprintf(stderr, "Connection closed\n");
            break;
        }

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Received exit command from Node B\n");
            break;
        } else if (strncmp(buffer, "read", 4) == 0) {
            printf("Received read request from Node B\n");
            // Simulate reading from shared memory
            char* data = readFromSharedMemory();
            if(strlen(data)>0)
            send(clientSocket, data, strlen(data), 0);
            else
            send(clientSocket, "NULL", 4, 0);
        } else if (strncmp(buffer, "write", 5) == 0) {
            printf("Received write request from Node B\n");
            // Receive data from Node B
            memset(buffer, 0, BUFFER_SIZE);
            int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            // if (bytesRead > 0) {
            //     writeToSharedMemory(buffer);
            //     send(clientSocket, "Write operation completed", 26, 0);
            // } else {
                send(clientSocket, "Error during write operation", 28, 0);
            // }
        } else {
            printf("Received invalid command from Node B\n");
            send(clientSocket, "Invalid command", 16, 0);
        }
    }

    // Close the connection and cleanup
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    printf("Node A disconnected\n");

    return 0;
}
