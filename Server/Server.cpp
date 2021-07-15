#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define INITIALISATION_ERROR 9999
#define SERVER_CLOSE_ERROR -9999
#define DIE(assertion, message)                                                         \
if(assertion)                                                                           \
{                                                                                       \
    std::cerr << "Error at line " << __LINE__ << " in file " << __FILE__ << "!\n";      \
    std::cerr << message;                                                               \
    std::exit(-1);                                                                      \
}

// Used libraries
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>

// Global variables declaration
int iResult = 0;
WSADATA wsaData;
SOCKET RecvSocket;
struct sockaddr_in RecvAddr;
unsigned short Port = 27015;
char RecvBuf[1024];
int BufLen = 1024;
struct sockaddr_in SenderAddr;
int SenderAddrSize = sizeof(SenderAddr);

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// Initialise global variables, try starting the server
int initialiseGlobal()
{
    //-----------------------------------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error %d\n", iResult);
        return INITIALISATION_ERROR;
    }
    //-----------------------------------------------
    // Create a receiver socket to receive datagrams
    RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (RecvSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error %d\n", WSAGetLastError());
        return INITIALISATION_ERROR;
    }
    //-----------------------------------------------
    // Bind the socket to any address and the specified port.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Give the port to this process
    iResult = bind(RecvSocket, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResult != 0) {
        wprintf(L"bind failed with error %d\n", WSAGetLastError());
        return INITIALISATION_ERROR;
    }
}

// Receive message
void receiveFrom()
{
    //-----------------------------------------------
    // Call the recvfrom function to receive datagrams
    // on the bound socket.
    wprintf(L"Receiving datagrams...\n");
    iResult = recvfrom(RecvSocket,
        RecvBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
    }
}

// Limit fps to not overload the CPU
void sleepServer()
{
    Sleep(2000);
}

// Receive message
void update()
{
    // Unblock socket if we receive no data
    if (WSAGetLastError() == WSAEWOULDBLOCK) {
        wprintf(L"No data received!");
        return;
    }
    receiveFrom();
}

// Close the server
int closeServer()
{
    //-----------------------------------------------
    // Close the socket when finished receiving datagrams
    wprintf(L"Finished receiving. Closing socket.\n");
    iResult = closesocket(RecvSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
        return SERVER_CLOSE_ERROR;
    }
}

// Exit
void cleanUp()
{
    //-----------------------------------------------
    // Clean up and exit.
    wprintf(L"Exiting.\n");
    WSACleanup();
}

int main()
{
    // Initialisation
    DIE(initialiseGlobal() == INITIALISATION_ERROR, "Couldn't start the server!");

    // Main loop
    while (true) {
        update();
        sleepServer();
    }

    // Close
    DIE(closeServer() == SERVER_CLOSE_ERROR, "Couldn't stop the server!");

    // Close
    cleanUp();

    return 0;
}