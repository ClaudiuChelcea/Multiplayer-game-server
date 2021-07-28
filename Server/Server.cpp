#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define INITIALISATION_ERROR 9999
#define SERVER_CLOSE_ERROR -9999
#define DIE(assertion, message)                                                         \
if(assertion)                                                                           \
{                                                                                       \
    std::cerr << "Error at line " << __LINE__ << " in file " << __FILE__ << "!\n;";     \
    std::cerr << message;                                                               \
    std::exit(-1);                                                                      \
}

// Used libraries
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <vector>
#include <string>

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

// Send answer variables
sockaddr_in vector_sock[100];
char vector_adrese[100][100];
int numar_adrese = 0;
int flag = 0;
char SendBuf[1024];
char* ip = NULL;
int aux = 0;
std::vector<std::pair<char*, int>> clients;
bool established_connection = false;
bool waiting_for_answer = true;

// Forward declarations
int send_to();

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

    u_long mode = 1;  // 1 to enable non-blocking socket
    ioctlsocket(RecvSocket, FIONBIO, &mode);

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

    return -1;
}

// Create clients list
void ClientsList()
{
    std::pair<char*, int> data;

    data.first = ip;
    data.second = SenderAddr.sin_port;

    // Count if the client exists
    int counter = 0;
    for (int i = 0; i < clients.size(); ++i)
    {
        if (clients[i].first == ip)
        {
            counter = 1;
        }
    }

    if (counter == 1)
    {
        printf("\nAlready connected client!\n");
    }
    else
    {
        clients.push_back(data);
        printf("\nAdded client!\n");
    }
}

// Receive message
void receiveFrom()
{
    //-----------------------------------------------
    // Call the recvfrom function to receive datagrams
    // on the bound socket.
    if (waiting_for_answer == true)
    {
        wprintf(L"\nWaiting for datagrams...\n");
        waiting_for_answer = false;
    }

    iResult = recvfrom(RecvSocket,
        RecvBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);

    if (iResult > 0)
    {
        std::cout << "\nMessage received: " << RecvBuf << "\n";
        ip = inet_ntoa(SenderAddr.sin_addr);
        std::cout << "From the client with IP " << ip << " port " << SenderAddr.sin_port << ".";
        ClientsList();
        aux++;
        waiting_for_answer = true;
    }

    if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            return;
        }
        else if (WSAGetLastError() == 10054)
        {
            wprintf(L"Client not open!\n");
        }
        else
        {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
        }
    }
}

// Limit fps to not overload the CPU
void sleepServer()
{
    Sleep(2000);
}

// Send to
int send_to()
{
    if (established_connection == false)
    {
        wprintf(L"Connection successfully established!\n");
        established_connection = true;
    }
    strcpy(SendBuf, "Hello from the server!");

    //---------------------------------------------
    // Send a datagram to the receiver
    iResult = sendto(RecvSocket,
        SendBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, sizeof(SenderAddr));

    if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return 1;
        }
        else
        {
            wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
            closesocket(RecvSocket);
            WSACleanup();
            return 1;
        }
    }

    return 0;
}

// Receive message
void update()
{
    

    // Send or receive alternately
    if (aux == 0)
    {
        receiveFrom();
    }
    if (aux == 1)
    {
        int a = send_to();
        aux = 0;
    }
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
