#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "json.hpp"
#include "client.h"

//For convenience
using json = nlohmann::json;

//Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"




Client::Client(char** argv) {
    //Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        exit(1);
    }

    ZeroMemory(&hints, sizeof(hints));

    //Specifying the address family
    hints.ai_family = AF_UNSPEC;

    //Specifying type of the socket
    hints.ai_socktype = SOCK_STREAM;

    //Specifying type of the protocol
    hints.ai_protocol = IPPROTO_TCP;

    //Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        exit(1);
    }

    //Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        //Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            exit(1);
        }

        //Connect to server
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    //We don't need addrinfo objects anymore, so we can free it
    freeaddrinfo(result);

    //Printing the error
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        exit(1);
    }

    std::cout << "Successfully connected to server" << std::endl;
}

Client::~Client() {
    //Shutdown the connection since no more data will be sent

    //iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    //Cleanup
    closesocket(ConnectSocket);
    WSACleanup();
}

json Client::request(json j) {
    //The following is the buffer and its length
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    //The variable connecting the data from buffers
    std::string fullData = "";

    ZeroMemory(recvbuf, recvbuflen);

    std::string s = j.dump();

    iResult = send(ConnectSocket, s.c_str(), (int)strlen(s.c_str()), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    //printf("Bytes Sent: %ld\n", iResult);


    //Receive until the peer closes the connection
    do {
        //Cleaning the buffer after before every read
        //Otherwise there will be troubles with the last part of the message transferred by parts
        ZeroMemory(recvbuf, recvbuflen);

        //Filling the buffer up to 1 symbol left
        //So that we won't lose the terminating null
        iResult = recv(ConnectSocket, recvbuf, recvbuflen - 1, 0);
        if (iResult > 0)
            //printf("Bytes received: %d\n", iResult);
            fullData += recvbuf;
            json j = json::parse(fullData, nullptr, false);

            //std::cout << fullData << std::endl;

            if (!j.is_discarded())
            {
                //std::cout << "the input is valid JSON" << std::endl;
                break;
            }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    return json::parse(fullData);
}

