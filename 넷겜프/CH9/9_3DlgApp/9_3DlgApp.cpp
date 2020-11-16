#include <Windows.h>
#include <stdio.h>

//������ ���ν���
LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

//���� ��Ʈ�� ����Լ�
void DisplayText(char* fmt, ...);

HINSTANCE hInst; //�ν��Ͻ� �ڵ�
HWND hEdit1;//���� ��Ʈ��
HWND hEdit2;//���� ��Ʈ��


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	DialogBox(handle_t, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;

}

//������ ���ν���

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


//���� ��Ʈ�� ����Լ�
void DisplayText(char* fmt, ...)
{

	//���� ���� ���ڸ� ó���ϱ����� va_list, va_start(), va_end()�Լ��� ���
	va_list arg;
	va_start(arg, fmt);

	char cbuf[256];
	vsprintf(cbuf, fmt, arg);
	int nLength = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}