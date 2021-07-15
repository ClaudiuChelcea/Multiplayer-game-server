#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SEND_DATA_SOCKET_INITIALISE_ERROR 9999
#define DIE(assertion, message)                                                         \
if(assertion)                                                                           \
{                                                                                       \
    std::cerr << "Error at line " << __LINE__ << " in file " << __FILE__ << "!\n;";     \
    std::cerr << message;                                                               \
    std::exit(-1);                                                                      \
}

// Libraries
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <iostream>

// Global variables declaration
int iResult;
WSADATA wsaData;
SOCKET SendSocket = INVALID_SOCKET;
sockaddr_in RecvAddr;
unsigned short Port = 27015;
char SendBuf[1024];
int BufLen = 1024;

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// Initialise global variables, try starting the server
int initialiseGlobal()
{
    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %d\n", iResult);
        return SEND_DATA_SOCKET_INITIALISE_ERROR;
    }

    //---------------------------------------------
    // Create a socket for sending data
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SendSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return SEND_DATA_SOCKET_INITIALISE_ERROR;
    }
    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // the receiver (in this example case "127.0.0.1")
    // and the specified port number.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
}

// Send to
int sendTo()
{
    //---------------------------------------------
    // Send a datagram to the receiver
    wprintf(L"Sending a datagram to the receiver...\n");
    iResult = sendto(SendSocket,
        SendBuf, BufLen, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }
}

// Limit fps
void sleepClient()
{
    Sleep(2000);
}

// Update game ( send messages to server )
void update()
{
    DIE(sendTo() == 1, "Couldn`t send message!");
}

// Close the client
int closeClient()
{
    //---------------------------------------------
    // When the application is finished sending, close the socket.
    wprintf(L"Finished sending. Closing socket.\n");
    iResult = closesocket(SendSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
}

// Exit
void cleanUp()
{
    //---------------------------------------------
    // Clean up and quit.
    wprintf(L"Exiting.\n");
    WSACleanup();
}

int main()
{
    // Initialisation
    DIE(initialiseGlobal() == SEND_DATA_SOCKET_INITIALISE_ERROR, "Couldn't create client's socket!");

    // Main loop
    while (true) {
        update();
        sleepClient();
    }

    // Close
    DIE(closeClient() == 1, "Couldn't close client properly!");

    // Close
    cleanUp();

    return 0;
}