#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include<iomanip>
using namespace std;
#define SERVERPORT 9000
#define BUFSIZE    512

CRITICAL_SECTION cs;
int g_clientCnt = 0;
HANDLE ClientEvent;
string g_pers[BUFSIZE];
// 소켓 함수 오류 출력 후 종료
void gotoxy(int x, int y) {
	COORD pos = { x,y }; //x, y 좌표 설정
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos); //커서 설정
}
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

// 소켓 함수 오류 출력
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
	return(len - left);
}


//파일 받기
int recvf(SOCKET s,char* fname, char* buf, int len, int flags)
{
	int index = g_clientCnt;
	int received;
	char* ptr = buf;
	int left = len;
	int remain = 0;
	int l = len / 10;
	int percent = len;
	g_pers[index] = ' ';
	while (left > 0) {
		received = recv(s, ptr, 10, flags);

		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
		remain += received;
		int result = (float)remain * 100 / (float)percent;

		EnterCriticalSection(&cs);
		g_pers[index] = ' ';
		g_pers[index] = fname;
		g_pers[index] +="진행률: ";
		g_pers[index] += to_string(result);
		g_pers[index] += "% \t";
		string r = g_pers[1] + g_pers[2];
		
		cout << r << '\r';
		LeaveCriticalSection(&cs);

	}
	return(len - left);
}
// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	InitializeCriticalSection(&cs);
	g_clientCnt++;
	SOCKET client_sock = (SOCKET)arg;
	int retval;
	SOCKADDR_IN clientaddr;
	int addrlen;

	char fname[256]; //파일 이름
	ZeroMemory(fname, 256);
	int flen; //파일 크기
	char* buf = NULL; //파일 받을 버퍼
	int len; //파일 길이
	
	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	g_pers[1] = ' ';
	g_pers[2] = ' ';
	while (1) {
		// 데이터 받기
		retval = recvn(client_sock, (char*)&flen, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		//이름 받기
		retval = recvn(client_sock, fname, flen, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		//데이터 받기(고정길이)
		retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		buf = new char[len];
		//데이터 받기(가변길이)
		retval = recvf(client_sock,fname, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
	}

	// closesocket()
	closesocket(client_sock);
	printf("\n[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	ofstream out(fname, ios::binary);
	out.write(buf, len);
	g_clientCnt--;
	return 0;
}

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	DeleteCriticalSection(&cs);
	return 0;
}