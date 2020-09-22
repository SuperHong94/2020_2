#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <iostream>
#include <stdio.h>


int main(int argc, char* argv[])
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	hostent 

	WSACleanup();

	return 0;
}