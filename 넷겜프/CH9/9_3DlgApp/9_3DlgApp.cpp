#include <Windows.h>
#include <stdio.h>

//윈도우 프로시저
LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

//편집 컨트롤 출력함수
void DisplayText(char* fmt, ...);

HINSTANCE hInst; //인스턴스 핸들
HWND hEdit1;//편집 컨트롤
HWND hEdit2;//편집 컨트롤


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	DialogBox(handle_t, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;

}

//윈도우 프로시저

LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZ, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			DisplayText("%s\r\n", buf);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;
		case IDCANCEL:

		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//편집 컨트롤 출력함수
void DisplayText(char* fmt, ...)
{

	//가변 길이 인자를 처리하기위해 va_list, va_start(), va_end()함수를 사용
	va_list arg;
	va_start(arg, fmt);

	char cbuf[256];
	vsprintf(cbuf, fmt, arg);
	int nLength = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}