#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <iostream>
#include <stdio.h>

union EndianCheck
{
	struct {
		BYTE b1;
		BYTE b2;
	} byte;
	struct {
		WORD w;
	} word;
};


int main(int argc, char* argv[])
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	u_short x1 = 0x1234;
	u_long y1 = 0x12345678;
	u_short x2=htons(x1);	// 0x 3412
	u_short x3 = ntohs(x1); // 0x 1234
	u_long y2;


	printf("0x%x\n", x1);
	printf("0x%x\n", x2);
	printf("0x%x\n", x3);

	//ȣ��Ʈ ����Ʈ-> ��Ʈ��ũ ����Ʈ

	//printf("[ȣ��Ʈ ����Ʈ-> ��Ʈ��ũ ����Ʈ]\n");
	//printf("0x%x -> 0x%x\n", x1, x2 = htons(x1));
	//printf("0x%x -> 0x%x\n", y1, y2 = htonl(y1));


	////��Ʈ��ũ ����Ʈ -> ȣ��Ʈ ����Ʈ
	//printf("\n[��Ʈ��ũ ����Ʈ -> ȣ��Ʈ ����Ʈ]\n");
	//printf("0x%x -> 0x%x\n", x2, ntohs(x2));
	//printf("0x%x -> 0x%x\n", y2, ntohl(y2));


	//printf("\n[�߸��� ��� ��	]\n");
	//printf("0x%x -> 0x%x\n", x1, htonl(x1));

	WSACleanup();




	return 0;
}