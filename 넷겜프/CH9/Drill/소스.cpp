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

//������ ���ν���
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//���� ��Ʈ�� ����Լ�
void DisplayText(char* fmt, ...);

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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
HINSTANCE hInst; //�ν��Ͻ� �ڵ�
HWND hEdit;//���� ��Ʈ��
HWND hProgress1;//�����׷�����
HWND hProgress2;//�����׷�����

CRITICAL_SECTION cs; //�Ӱ迵��


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	InitializeCriticalSection(&cs);

	//������Ŭ�������
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

	//������ ����
	HWND hWnd = CreateWindow("MYWndClass", "WinApp", WS_OVERLAPPEDWINDOW, 0, 0, 600, 200, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)return 1;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	//���� ��� ������ ����
	CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);

	//�޽�������
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DeleteCriticalSection(&cs);
	return msg.wParam;

}

//������ ���ν���

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	gHwnd = hWnd;
	switch (uMsg)
	{
	case WM_CREATE:
		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, hWnd, (HMENU)100, hInst, NULL); //ES_READONLY�� ������Ʈ�ѻ����� �ϱ⸸ �����ϴٴ� �ǹ��̴�. �̴� ������Ʈ���� ����������� ����Ѵٴ� �ǹ̴�.
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
	case WM_SETFOCUS: //Ű���� ��Ŀ���� ������ �߻��Ѵ�. �̴� ���� �����찡 Ű���� �޽����� ���� �� ������ �ǹ��Ѵ�.
		SetFocus(hEdit); //Ű������Ŀ���� �ٸ�������� ��ȯ
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//���� ��Ʈ�� ����Լ�
void DisplayText(char* fmt, ...)
{

	//���� ���� ���ڸ� ó���ϱ����� va_list, va_start(), va_end()�Լ��� ���
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

	//���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
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

		// ������ Ŭ���̾�Ʈ ���� ���
		//DisplayText("\r\n[TCP����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ��ȣ=%d\r\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
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

//����� ���� ������ ���� �Լ�
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

	char fname[256]; //���� �̸�
	ZeroMemory(fname, 256);
	int flen; //���� ũ��
	char* buf = NULL; //���� ���� ����
	int len; //���� ����

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	g_pers[1] = ' ';
	g_pers[2] = ' ';
	while (1) {
		// ������ �ޱ�
		retval = recvn(client_sock, (char*)&flen, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		//�̸� �ޱ�
		retval = recvn(client_sock, fname, flen, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		//������ �ޱ�(��������)
		retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;
		buf = new char[len];
		//������ �ޱ�(��������)
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
	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	ofstream out(fname, ios::binary);
	out.write(buf, len);
	g_clientCnt--;
	return 0;
}