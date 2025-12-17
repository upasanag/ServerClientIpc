// Server file
#include "config.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <map>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::map<int, time_t> numToTsMap;


void* processClientMsg(void* arg) {
    int clientSocket = *(int*)arg;
    delete (int*)arg; 

    Command clientMsg;
    ServerResponse response;
    ssize_t readBytes;
    
    readBytes = read(clientSocket, &clientMsg, sizeof(Command));
    if (readBytes != sizeof(Command)) {
        close(clientSocket);
        return NULL;
    }

    pthread_mutex_lock(&mutex);
    
    try {
        switch (clientMsg.type) {
            case INSERT: {
                if (clientMsg.number <= 0) {
                    response.status = ERROR_GENERAL;
                } else if (numToTsMap.count(clientMsg.number)) {
                    response.status = ERROR_DUPLICATE;
                } else {
                    numToTsMap[clientMsg.number] = time(NULL);
                    response.status = OK;
                    response.number = clientMsg.number;
                }
                write(clientSocket, &response, sizeof(ServerResponse));
                break;
            }
            case DELETE: {
                if (clientMsg.number <= 0) {
                    response.status = ERROR_GENERAL;
                } else if (numToTsMap.erase(clientMsg.number)) {
                    response.status = OK;
                    response.number = clientMsg.number;
                } else {
                    response.status = ERROR_NOT_FOUND;
                }
                write(clientSocket, &response, sizeof(ServerResponse));
                break;
            }
            case PRINT_ALL: {
                if (numToTsMap.empty()) {
                    response.status = NO_DATA;
                    write(clientSocket, &response, sizeof(ServerResponse));
                } else {
                    std::vector<NumTsPair> entries;
                    for (const auto& pair : numToTsMap) {
                        entries.push_back({pair.first, pair.second});
                    }

                    response.status = OK;
                    write(clientSocket, &response, sizeof(ServerResponse));
                    write(clientSocket, entries.data(), entries.size() * sizeof(NumTsPair));
                }
                break;
            }
            case DELETE_ALL: {
                numToTsMap.clear();
                response.status = OK;
                write(clientSocket, &response, sizeof(ServerResponse));
                break;
            }
            case FIND: {
                if (clientMsg.number <= 0) {
                    response.status = ERROR_GENERAL;
                } else if (numToTsMap.count(clientMsg.number)) {
                    response.status = OK;
                    response.number = clientMsg.number;
                } else {
                    response.status = ERROR_NOT_FOUND;
                }
                write(clientSocket, &response, sizeof(ServerResponse));
                break;
            }
            case EXIT: {
                response.status = OK;
                write(clientSocket, &response, sizeof(ServerResponse));
                break;
            }
            default: {
                response.status = ERROR_INVALID_CMD;
                write(clientSocket, &response, sizeof(ServerResponse));
            }
        }
    } catch (...) {
        response.status = ERROR_GENERAL;
        write(clientSocket, &response, sizeof(ServerResponse));
    }

    pthread_mutex_unlock(&mutex);
    
    close(clientSocket);
    return NULL;
}

int main() {
    int serverSock, clientSock;
    socklen_t clientLength;
    struct sockaddr_un serverAddress, clientAddress;

    serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("Error creating socket");
        return 1;
    }

    unlink(PATH);
    
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sun_family = AF_UNIX;
    strncpy(serverAddress.sun_path, PATH, sizeof(serverAddress.sun_path) - 1);

    if (bind(serverSock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error binding socket");
        close(serverSock);
        return 1;
    }

    if (listen(serverSock, 5) < 0) {
        perror("Error listening on socket");
        close(serverSock);
        return 1;
    }

    std::cout << "Server listening on path: " << PATH << "..." << std::endl;

    while (true) {
        clientLength = sizeof(clientAddress);
        clientSock = accept(serverSock, (struct sockaddr*)&clientAddress, &clientLength);
        if (clientSock < 0) {
            perror("Error accepting connection");
            continue;
        }

        int* newSocket = new int(clientSock); 
        pthread_t thread;
        
        if (pthread_create(&thread, NULL, processClientMsg, (void*)newSocket) != 0) {
            perror("Error creating thread");
            close(clientSock);
            delete newSocket;
        } else {
            pthread_detach(thread);
        }
    }

    close(serverSock);
    unlink(PATH);
    pthread_mutex_destroy(&mutex);
    return 0;
}