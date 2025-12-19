#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Cache coherence states
enum CacheState { INVALID, SHARED, MODIFIED };

// Structure to represent a cache block
struct CacheBlock {
    char data[BUFFER_SIZE];
    enum CacheState state;
};

// Shared memory cache
struct CacheBlock cache[BUFFER_SIZE];

// Function to send acknowledgment
void sendAcknowledgment(SOCKET clientSocket, int blockIndex) {
    char acknowledgment[BUFFER_SIZE];
    int messageLength = snprintf(acknowledgment, sizeof(acknowledgment), "acknowledgment %d", blockIndex);
    
    if (messageLength >= sizeof(acknowledgment)) {
        fprintf(stderr, "Acknowledgment message is too long\n");
        // Handle the error, e.g., by truncating the message or notifying the client.
    } else {
        int bytesSent = send(clientSocket, acknowledgment, strlen(acknowledgment), 0);
        if (bytesSent == SOCKET_ERROR) {
            fprintf(stderr, "Error sending acknowledgment\n");
        } else {
            printf("Acknowledgment sent: %s\n", acknowledgment);
        }
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

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesRead <= 0) {
            fprintf(stderr, "Client disconnected\n");
            break;
        }

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Received exit command from Node B\n");
            break;
        } else if (strncmp(buffer, "read", 4) == 0) {
            printf("Received read request from Node B\n");
            int blockIndex = atoi(buffer + 4);
            if (blockIndex >= 0 && blockIndex < BUFFER_SIZE) {
                if (cache[blockIndex].state == INVALID) {
                    // Handle cache miss - In this example, return a "Not found" message.
                    printf("Cache Miss: Block %d is INVALID\n", blockIndex);
                    send(clientSocket, "Not found", 9, 0);
                    } else if (cache[blockIndex].state == SHARED || cache[blockIndex].state == MODIFIED) {
    // Cache hit
    printf("Cache Hit: Block %d is %s\n", blockIndex,
        (cache[blockIndex].state == SHARED) ? "SHARED" : "MODIFIED");

    // Cache hit - Send the data from the cache
    send(clientSocket, cache[blockIndex].data, strlen(cache[blockIndex].data), 0);
                
                
                
                
                } else {
                    // Cache hit - Send the data from the cache
                    send(clientSocket, cache[blockIndex].data, strlen(cache[blockIndex].data), 0);
                }
            } else {
                send(clientSocket, "Invalid cache block index", 24, 0);
            }
        } else if (strncmp(buffer, "write", 5) == 0) {
            printf("Received write request from Node B\n");
            int blockIndex = strtol(buffer + 5, NULL, 10);
            // Skip "write " and store the data
            strncpy(cache[blockIndex].data, buffer + 12, BUFFER_SIZE);
            cache[blockIndex].state = MODIFIED;
            printf("Writing to cache (Block %d): %s\n", blockIndex, buffer + 6);

            // Send acknowledgment to Node B
            sendAcknowledgment(clientSocket, blockIndex);
            fflush(stdout);
            

            // Notify other nodes of the invalidation
            for (int i = 0; i < BUFFER_SIZE; i++) {
                if (i != blockIndex && cache[i].state == SHARED) {
                    char invalidateMessage[BUFFER_SIZE];
                    snprintf(invalidateMessage, sizeof(invalidateMessage), "invalidate %d", i);
                    int bytesSent = send(clientSocket, invalidateMessage, strlen(invalidateMessage), 0);
            if (bytesSent == SOCKET_ERROR) {
                fprintf(stderr, "Error sending invalidation message\n");
            } else {
                printf("Invalidation message sent: %s\n", invalidateMessage);
            }
                }
            }
        } else if (strncmp(buffer, "invalidate", 10) == 0) {
            printf("Received invalidation request from Node B\n");
            int blockIndex = atoi(buffer + 10);
            if (blockIndex >= 0 && blockIndex < BUFFER_SIZE) {
                // Handle the invalidation request
                cache[blockIndex].state = INVALID;
                printf("Cache block %d invalidated\n", blockIndex);
            }
            sendAcknowledgment(clientSocket, blockIndex);
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
