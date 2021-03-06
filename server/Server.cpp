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

#include <algorithm>

/*
The #pragma comment indicates to the linker that the Ws2_32.lib file is needed.
*/
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

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

void bindSocket(SOCKET& ListenSocket) {
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

void listenSocket(SOCKET& ListenSocket) {
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	printf("Socket is listening.\n");
}

SOCKET acceptConnection(SOCKET& ListenSocket) {
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

int sendData(SOCKET& ClientSocket, char* recvbuf, int& bytesReceived) {
	int bytesSent = send(ClientSocket, recvbuf, bytesReceived, 0);
	if (bytesSent == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		exit(1);
	}
	return bytesSent;
}

int receiveData(SOCKET ClientSocket, char* recvbuf, int recvbuflen) {
	int bytesReceived = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (bytesReceived < 0) {
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		exit(1);
	}
	return bytesReceived;
}

void receiveAndSendData(SOCKET ClientSocket) {
	char* recvbuf = new char[DEFAULT_BUFLEN];
	std::fill(recvbuf, recvbuf + DEFAULT_BUFLEN, '\0');
	int recvbuflen = DEFAULT_BUFLEN;
	int bytesReceived = 0;

	// Receive until the peer shuts down the connection
	do {

		bytesReceived = receiveData(ClientSocket, recvbuf, recvbuflen);
		if (bytesReceived > 0) {
			printf("\nBytes received: %d\nMessage received: %s\n\n", 
				bytesReceived, 
				recvbuf);

			recvbuf[bytesReceived - 1] = '\n';
			// Echo the buffer back to the sender
			int bytesSent = sendData(ClientSocket, recvbuf, bytesReceived);
			printf("Bytes sent: %d\n", bytesSent);
		}
		else if (bytesReceived == 0)
			printf("Connection closing...\n");

	} while (bytesReceived > 0);

	delete[] recvbuf;
}

void disconnectAndShutdown(SOCKET clientSocket) {
	// shutdown the send half of the connection since no more data will be sent
	int shutdownStatus = shutdown(clientSocket, SD_SEND);
	if (shutdownStatus == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		exit(1);
	}
	// cleanup
	closesocket(clientSocket);
}

int main(int argc, char* argv[]) {

	initWinsock();
	SOCKET ListenSocket = createSocket();
	bindSocket(ListenSocket);
	listenSocket(ListenSocket);
	SOCKET ClientSocket = INVALID_SOCKET;
	while(true) {
		ClientSocket = acceptConnection(ListenSocket);
		receiveAndSendData(ClientSocket);
		disconnectAndShutdown(ClientSocket);
	}
	
	WSACleanup();

	return 0;
}

