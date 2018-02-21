#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "resource.h"

HWND *dialog;
MSG msg;
BOOL ret;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		switch (MessageBox(hDlg, TEXT("Don't go man!"), TEXT("Close"), MB_RIGHT | MB_ICONHAND | MB_ABORTRETRYIGNORE)){
			case IDIGNORE:
				DestroyWindow(hDlg);
				break;
			case IDRETRY:
				return FALSE;
			default:
				return TRUE;
		}
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}

	return FALSE;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	MessageBox(NULL, "It works man?\n", "A cool Mbox", 0);

	dialog = malloc(sizeof(HWND) * 1);
	dialog[0] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
	dialog[1] = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), 0, 0);

	ShowWindow(dialog[0], nCmdShow);
	ShowWindow(dialog[1], nCmdShow);

	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(dialog[0], &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	return 0;

}