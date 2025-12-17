// Client file
#include "config.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <iomanip>

int connectToServer() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error in creating socket. Returning.");
        return -1;
    }

    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, PATH, sizeof(remote.sun_path) - 1);
    
    if (connect(sock, (struct sockaddr*)&remote, sizeof(remote)) < 0) {
        std::cerr << "Error: Could not connect to the server at " << PATH << std::endl;
        close(sock);
        return -1;
    }
    return sock;
}

void display() {
    std::cout << "\n***** Please select option *****" << std::endl;
    std::cout << "1. Insert a number" << std::endl;
    std::cout << "2. Delete a number" << std::endl;
    std::cout << "3. Print all" << std::endl;
    std::cout << "4. Delete all" << std::endl;
    std::cout << "5. Find a number" << std::endl;
    std::cout << "6. Exit" << std::endl;
    std::cout << "Option: ";
}

bool validateInteger(const std::string& nextMsg, int& number) {
    std::string line;
    std::cout << nextMsg;
    if (!std::getline(std::cin, line)) {
        return false;
    }
    
    std::stringstream ss(line);
    long long temp;
    
    if (ss >> temp && ss.eof()) { 
        if (temp > 0 && temp <= 2147483647) { 
            number = (int)temp;
            return true;
        }
    }
    
    std::cerr << "Error!!!: Invalid number provided. It should be a positive integer (1 to 2147483647)." << std::endl;
    return false;
}

void handleResponse(ServerResponse response, Command msg, int socket) {
    switch (response.status) {
            case OK:
                if (msg.type == INSERT) {
                    std::cout << "SUCCESS: Number " << response.number << " inserted with timestamp " << time(NULL) << "." << std::endl;
                } else if (msg.type == DELETE) {
                    std::cout << "SUCCESS: Number " << response.number << " deleted." << std::endl;
                } else if (msg.type == DELETE_ALL) {
                    std::cout << "SUCCESS: Deleted all numbers." << std::endl;
                } else if (msg.type == FIND) {
                    std::cout << "SUCCESS: Number " << response.number << " is PRESENT in the store." << std::endl;
                } else if (msg.type == PRINT_ALL) {
                    NumTsPair pair;
                    ssize_t readBytes;
                    
                    std::cout << "\n*** Sorted Stored Numbers ***" << std::endl;
                    std::cout << std::left << std::setw(15) << "Number" << " | " << "Timestamp (Unix)" << std::endl;
                    std::cout << "===================================" << std::endl;

                    while ((readBytes = read(socket, &pair, sizeof(NumTsPair))) > 0) {
                        if (readBytes == sizeof(NumTsPair)) {
                            std::cout << std::left << std::setw(15) << pair.number << " | " << pair.timestamp << std::endl;
                        } else {
                            std::cerr << "!!! Warning: Partial read." << std::endl;
                        }
                    }
                    if (readBytes < 0) perror("Error reading number");
                    std::cout << "===================================" << std::endl;
                }
                break;
            case ERROR_DUPLICATE:
                std::cerr << "Error!: Number: " << msg.number << " is already present." << std::endl;
                break;
            case ERROR_NOT_FOUND:
                std::cerr << "Error!: Number " << msg.number << " not found." << std::endl;
                break;
            case NO_DATA:
                std::cout << "INFO: The store is empty." << std::endl;
                break;
            case ERROR_GENERAL:
                std::cerr << "Error!: An internal error occurred (e.g., invalid number)." << std::endl;
                break;
            default:
                std::cerr << "Error!: Received unknown status code from server." << std::endl;
        }
}

int main() {
    int choice = 0;
    int number;
    
    while (choice != 6) {
        display();
        
        std::string strChoice;
        if (!std::getline(std::cin, strChoice) || strChoice.empty()) {
            std::cerr << "Error!: Invalid choice." << std::endl;
            continue;
        }

        try {
            choice = std::stoi(strChoice);
        } catch (...) {
            std::cerr << "Error!: Invalid choice entered. Please enter number (1-6)." << std::endl;
            continue;
        }
        
        if (choice == 6) break; 
        
        Command msg = {};
        int sock = -1;

        if (choice == 1 || choice == 2 || choice == 5) {
            std::string nextMsg;
            switch (choice)
            {
            case 1:
                nextMsg = "Enter number to insert: ";
                break;
            case 2:
                nextMsg = "Enter number to delete: ";
                break;
            case 5: 
                nextMsg = "Enter number to find: ";
                break;
            }
            
            if (!validateInteger(nextMsg, number)) continue;
            msg.number = number;
        }

        switch (choice) {
            case 1: msg.type = INSERT; break;
            case 2: msg.type = DELETE; break;
            case 3: msg.type = PRINT_ALL; break;
            case 4: msg.type = DELETE_ALL; break;
            case 5: msg.type = FIND; break;
            default: 
                std::cerr << "Error!: Invalid choice (1-6)." << std::endl;
                continue;
        }

        sock = connectToServer();
        if (sock == -1) continue; 
        
        if (write(sock, &msg, sizeof(Command)) < 0) {
            perror("Error sending to server");
            close(sock);
            continue;
        }
        
        ServerResponse response;
        if (read(sock, &response, sizeof(ServerResponse)) <= 0) {
            std::cerr << "Error!: Failed to receive response." << std::endl;
            close(sock);
            continue;
        }
        handleResponse(response, msg, sock);
        
        close(sock);
    }

    std::cout << "\n Exiting." << std::endl;
    return 0;
}