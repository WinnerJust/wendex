#pragma once

#define WIN32_LEAN_AND_MEAN
//This prevents from double inclusion

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "json.hpp"

//For convenience
using json = nlohmann::json;

class Client {
    //These should be initialized in order to use sockets
    WSADATA wsaData;

    //There we will assign socket objects
    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    int iResult;

public:
    Client(char** argv);

    ~Client();

    json request(json j);
};