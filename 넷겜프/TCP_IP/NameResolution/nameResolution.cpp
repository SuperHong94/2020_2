#pragma comment(lib,"ws2_32")
#include <winsock2.h>
#include <stdio.h>	
#include <iostream>
#include <string>


using namespace std;

//�����Լ� ���� ���

void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

//������ �̸�->IPv4�ּ�
BOOL GetIPAddr(char* name, IN_ADDR* addr)
{
	HOSTENT* ptr = gethostbyname(name);
	if (ptr == NULL)
	{
		err_display("gethostbyname()");
		return FALSE;
	}
	if (ptr->h_addrtype != AF_INET)
		return FALSE;
	std::cout << "������ �ּ��̸�: " << ptr->h_name << endl;
	std::cout << "Aliases\n";
	for (int i =0; ptr->h_aliases[i]; ++i)
	{
		/*if ( ptr->h_aliases[i] == nullptr)
			break;*/
		std::cout << '\t' << ptr->h_aliases[i] << std::endl;
	}

	std::cout << endl;

	std::cout << "IP �ּ�\n";
	for (auto i = ptr->h_addr_list; i != NULL; ++i)
	{
		if (i[0] == nullptr)
			break;
		memcpy(addr, i[0], ptr->h_length);
		char* s = inet_ntoa(*addr);
		std::cout << '\t' << s << endl;
	}
	std::cout << endl;

	return TRUE;

}


//IPv4 �ּ�-> ������ �̸�

BOOL GetDomainName( char* name, int namelen)
{
	IN_ADDR addr;
	addr.s_addr = inet_addr(name);
	HOSTENT* ptr = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	if (ptr == NULL) {
		err_display("gethostbyaddr()");
		return FALSE;
	}
	char buffer[256];
	strncpy(buffer, ptr->h_name, sizeof(buffer));
	std::cout << "IPv4�� ������ �ּ��̸�: " << buffer << endl;
	std::cout << "Aliases\n";
	for (auto i = ptr->h_aliases; i != NULL; ++i)
	{
		if (i[0] == nullptr)
			break;
		std::cout << '\t' << i[0] << std::endl;
	}

	std::cout << endl;

	std::cout << "IP �ּ�\n";
	for (auto i = ptr->h_addr_list; i != NULL; ++i)
	{
		if (i[0] == nullptr)
			break;

		memcpy(&addr, i[0], ptr->h_length);
		char* s = inet_ntoa(addr);
		std::cout << '\t' << s << endl;
	}
	std::cout << endl;
	return TRUE;

}

int main(int arc, char* argv[])
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	
	if (argv[1][0] > '9') {
		char* ip_name = argv[1];
		cout << "ã������ ������ �̸�: " << argv[1] << endl;
		//�������̸� �̸����� ã��
		IN_ADDR addr;
		GetIPAddr(argv[1], &addr);
	}
	else {
		//IP�ּҷ� ã��
		char* ip_num = argv[1];
		cout << "ã������ ipv4�ּ�: " << ip_num << endl;
		GetDomainName( ip_num, sizeof(ip_num));


	}
	WSACleanup();
	return 0;
}