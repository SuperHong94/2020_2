#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 1200

using namespace std;
//소켓 함 수 오류 출력 후 종료
void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}


//소켓함수 오류출력
void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


//사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int main(int argc, char* argv[])
{
	int retval;

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");


	//connet()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)err_quit("connect()");

	//데이터 통신에 사용할 변수

	//파일읽기

	string fname = argv[1];
	int flen = fname.length();
	ifstream in(fname, ios::binary);
	if (!in) {
		std::cout << "그런 파일없습니다." << std::endl;
		return 0;
	}
	in.seekg(0, ios::end);
	int len = in.tellg();  //전체크기

	in.seekg(0, ios::beg);
	char* buf = new char[len];
	in.read(buf, len);



	char cfname[256];
	ZeroMemory(cfname, 256);
	strcpy(cfname, fname.data());
	flen = fname.length();
	//서버와 데이터 통신

	//데이터 보내기(고정길이)
	retval = send(sock, (char*)&flen, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 0;
	}

	retval = send(sock, cfname, flen, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 0;
	}
	cout << flen << endl;

	retval = send(sock, (char*)&len, sizeof(int), 0);
	std::cout << len << std::endl;
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 0;
	}

	std::cout << "고정길이는 보냄";
	//데이터 보내기(가변길이)
	retval = send(sock, buf, len, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 0;
	}
	printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);

	//closesoket()
	closesocket(sock);

	//윈속 종료
	WSACleanup();
	delete[] buf;
	return 0;
}