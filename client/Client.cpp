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
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "27015"

struct addrinfo *addrinfoListPtr = NULL, *ptr = NULL, hints;

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

SOCKET connectSocket(char*& hostname) {
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int getaddrinfoStatus = getaddrinfo(hostname, 
		DEFAULT_PORT, 
		&hints, 
		&addrinfoListPtr);
	if (getaddrinfoStatus != 0) {
		printf("getaddrinfo failed: %d\n", getaddrinfoStatus);
		WSACleanup();
		exit(1);
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	
	// Attempt to connect to an address until one succeeds
	for (ptr = addrinfoListPtr; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			exit(1);
		}

		// Connect to server.
		int connectStatus = connect(ConnectSocket, 
			ptr->ai_addr, 
			(int)ptr->ai_addrlen);
		if (connectStatus == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(addrinfoListPtr);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		exit(1);
	}

	printf("Socket connected.\n");
	return ConnectSocket;
}

int main(int argc, char *argv[]) {

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	initWinsock();
	SOCKET ConnectSocket = connectSocket(argv[1]);


	Sleep(1000);

	return 0;
}
