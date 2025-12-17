// Config file
#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>

#define PATH "/tmp/log.tmp"

enum Status {
    OK,
    ERROR_DUPLICATE,
    ERROR_NOT_FOUND,
    ERROR_INVALID_CMD,
    ERROR_GENERAL,
    NO_DATA
};

enum Type {
    INSERT,
    DELETE,
    PRINT_ALL,
    DELETE_ALL,
    FIND,
    EXIT,
};

struct Command {
    Type type;
    int number; 
};

struct NumTsPair {
    int number;
    time_t timestamp;
};

struct ServerResponse {
    Status status;
    int number;
};