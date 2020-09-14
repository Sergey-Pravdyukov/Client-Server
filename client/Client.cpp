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

int main() {
	initWinsock();

	Sleep(1000);

	return 0;
}
