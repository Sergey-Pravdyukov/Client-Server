/*
The Winsock2.h header file internally includes core elements from the Windows.h
header file, so there is not  usually an #include line for the Windows.h header
file in Winsock applications. If an #include line is needed for the Windows.h
header file, this should be preceded with the #define WIN32_LEAN_AND_MEAN
macro. For historical reasons, the Windows.h header defaults to including the
Winsock.h header file for Windows Sockets 1.1. The declarations in the
Winsock.h header file will conflict with the declarations in the Winsock2.h
header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN macro
prevents the Winsock.h from being included by the Windows.h header.
*/
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>

/*
The #pragma comment indicates to the linker that the Ws2_32.lib file is needed.
*/
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"

struct addrinfo *addrinfoListPtr = NULL, hints;

void initWinsock() {
	WSADATA wsaData;

	// Initialize Winsock
	int initStatus = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (initStatus != 0) {
		printf("WSAStartup failed: %d\n", initStatus);
		exit(1);
	}
	printf("Winsock initialized.\n");
}

SOCKET createSocket() {
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int getaddrinfoStatus = getaddrinfo(NULL, 
		DEFAULT_PORT, 
		&hints, 
		&addrinfoListPtr);
	if (getaddrinfoStatus != 0) {
		printf("getaddrinfo failed: %d\n", getaddrinfoStatus);
		WSACleanup();
		exit(1);
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(addrinfoListPtr->ai_family, 
		addrinfoListPtr->ai_socktype, 
		addrinfoListPtr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(addrinfoListPtr);
		WSACleanup();
		exit(1);
	}

	printf("Listen socket created.\n");;
	return ListenSocket;
}

void bindSocket(SOCKET ListenSocket) {
	// Setup the TCP listening socket
	int bindStatus = bind(ListenSocket, 
		addrinfoListPtr->ai_addr, 
		(int)addrinfoListPtr->ai_addrlen);
	if (bindStatus == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(addrinfoListPtr);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	freeaddrinfo(addrinfoListPtr);
	printf("Listen socket binded.\n");
}

void listenSocket(SOCKET ListenSocket) {
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	printf("Socket is listening.\n");
}

SOCKET acceptConnection(SOCKET ListenSocket) {
	SOCKET ClientSocket = INVALID_SOCKET;

	// Accept a client socket
	printf("Waiting for client socket.\n");
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	printf("Client Socket was accepted.\n");
	return ClientSocket;
}

int main(int argc, char* argv[]) {

	initWinsock();
	SOCKET ListenSocket = createSocket();
	bindSocket(ListenSocket);
	listenSocket(ListenSocket);
	SOCKET ClientSocket = acceptConnection(ListenSocket);


	Sleep(1000);

	return 0;
}

