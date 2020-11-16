#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <CommCtrl.h>
using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512
int 	g_clientCnt = 0;


int g_pers[2];

int recvn(SOCKET s, char* buf, int len, int flags);
int recvf(SOCKET s, char* fname, char* buf, int len, int flags);

//윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//편집 컨트롤 출력함수
void DisplayText(char* fmt, ...);

// 소켓 함수 오류 출력 후 종료
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

DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);

HWND gHwnd;
HINSTANCE hInst; //인스턴스 핸들
HWND hEdit;//편집 컨트롤
HWND hProgress1;//포르그래스바
HWND hProgress2;//포르그래스바

CRITICAL_SECTION cs; //임계영역


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	InitializeCriticalSection(&cs);

	//윈도우클래스등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "MyWndClass";
	if (!RegisterClass(&wndclass))return 1;

	//윈도우 생성
	HWND hWnd = CreateWindow("MYWndClass", "WinApp", WS_OVERLAPPEDWINDOW, 0, 0, 600, 200, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)return 1;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	//소켓 통신 스레드 생성
	CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);

	//메시지루프
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DeleteCriticalSection(&cs);
	return msg.wParam;

}

//윈도우 프로시저

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	gHwnd = hWnd;
	switch (uMsg)
	{
	case WM_CREATE:
		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, hWnd, (HMENU)100, hInst, NULL); //ES_READONLY는 편집컨트롤생성시 일기만 가능하다는 의미이다. 이는 편집컨트롤을 출력전용으로 사용한다는 의미다.
		hProgress1 = CreateWindow(PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 10, 500, 30, gHwnd, NULL, hInst, NULL);
		SendMessage(hProgress1, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(hProgress1, PBM_SETPOS, 0, 0);


		hProgress2 = CreateWindow(PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 50, 500, 30, gHwnd, NULL, hInst, NULL);
		SendMessage(hProgress2, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(hProgress2, PBM_SETPOS, 0, 0);

		return 0;
	case WM_SIZE:
		MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	case WM_SETFOCUS: //키보드 포커스를 얻을떄 발생한다. 이는 메인 윈도우가 키보드 메시지를 받을 수 있음을 의미한다.
		SetFocus(hEdit); //키보드포커스를 다른윈도우로 전환
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//편집 컨트롤 출력함수
void DisplayText(char* fmt, ...)
{

	//가변 길이 인자를 처리하기위해 va_list, va_start(), va_end()함수를 사용
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZ + 256];
	vsprintf(cbuf, fmt, arg);
	EnterCriticalSection(&cs);
	int nLength = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
	LeaveCriticalSection(&cs);

	va_end(arg);
}

DWORD WINAPI ServerMain(LPVOID arg)
{
	int retval;

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//socket()	
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)err_quit("socket()");

	//bind()	
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
		//DisplayText("\r\n[TCP서버] 클라이언트 점속: IP 주소=%s, 포트번호=%d\r\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

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
	return 0;
}




int recvf(SOCKET s, char* fname, char* buf, int len, int flags)
{
	int index = g_clientCnt;
	int received;
	char* ptr = buf;
	int left = len;
	int remain = 0;
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

		g_pers[index] = result;

		/*for (int i = 0; i < g_clientCnt + 1; i++)
			r += g_pers[i];*/

		EnterCriticalSection(&cs);
		if (index == 1) {
			SendMessage(hProgress1, PBM_SETPOS, g_pers[index], 0);
		}
		else if (index == 2) {

			SendMessage(hProgress2, PBM_SETPOS, g_pers[index], 0);
		}
		LeaveCriticalSection(&cs);

	}
	return(len - left);
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

DWORD WINAPI ProcessClient(LPVOID arg)
{



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
		retval = recvf(client_sock, fname, buf, len, 0);
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