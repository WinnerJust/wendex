#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <iostream>
#include <thread>
#include "json.hpp"

#include "server.h"

//For convenience
using json = nlohmann::json;

//Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"



void Server::setOnData(std::function<json(json)> onData) {
    this->onData = onData;
}

Server::Server() {
    //Doing this because we cannot call constructor to run in other thread
    //Or we can, but I do not know how...
    std::thread(&Server::start, this).detach();
}

void Server::start() {
    //This should be initialized in order to use sockets
    WSADATA wsaData;
    int iResult;

    //There we will assign socket objects
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    //Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        throw 1;
    }

    ZeroMemory(&hints, sizeof(hints));

    //Specifying the address family
    hints.ai_family = AF_INET;

    //Specifying type of the socket
    hints.ai_socktype = SOCK_STREAM;

    //Specifying type of the protocol
    hints.ai_protocol = IPPROTO_TCP;

    //Specifying that socket will be used as a server
    hints.ai_flags = AI_PASSIVE;

    //Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        throw 1;
    }

    //Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        throw 1;
    }

    //Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        throw 1;
    }

    //We don't need addrinfo objects anymore, so we can free it
    freeaddrinfo(result);

    while (1) {
        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            throw 1;
        }

        //Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            throw 1;
        }
        
        //Passing the connection to the other thread
        //Thereby letting the current thread to continue accepting other connections using ListenSocket
        std::thread(&Server::handleConnection, this, std::ref(ClientSocket)).detach();
    }

    //No longer need server socket
    closesocket(ListenSocket);

    WSACleanup();
}

int Server::handleConnection(SOCKET& ClientSocket) {
    //There will be stored results of operations with sockets
    int iResult;

    //There will be stored the result of the send operation
    int iSendResult;

    //The following is the buffer and its length
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    //The variable connecting the data from buffers
    std::string fullData = "";

    //Receive until the peer shuts down the connection
    do {
        //Cleaning the buffer after before every read
        //Otherwise there will be troubles with the last part of the message transferred by parts
        ZeroMemory(recvbuf, recvbuflen);

        //Filling the buffer up to 1 symbol left
        //So that we won't lose the terminating null
        iResult = recv(ClientSocket, recvbuf, recvbuflen - 1, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            fullData += recvbuf;
            json j = json::parse(fullData, nullptr, false);

            if (!j.is_discarded())
            {
                std::cout << "the input is valid JSON" << std::endl;

                //Json won't be valid unless we send a full json object that we wanted
                //It cannot be mistakenly treated as a full object as the middle of parsing
                //So at that point we received the whole message
                //And we can clean the variable for a new message
                fullData = "";

                //Calling the handler
                json ans = onData(j);

                //Passing the result of the handler to the client
                iSendResult = send(ClientSocket, ans.dump().c_str(), ans.dump().size(), 0);
                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }

                printf("Bytes sent: %d\n", iSendResult);
            }
            else
            {
                std::cout << "the input is invalid JSON: " << j << std::endl;
                std::cout << fullData << std::endl;
            }
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);


    //There we will get only when the infinite loop above will return
    //Meaning that connection was closed by client

    //So we will perform cleaning operations

    //Shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    //Cleanup
    closesocket(ClientSocket);

    return 0;
}

