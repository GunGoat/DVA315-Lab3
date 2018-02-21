#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "resource.h"

#define MAINWINDOW 0
#define ADDWINDOW 1

HWND *dialog;
MSG msg;
BOOL ret;

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
			if (dialog[MAINWINDOW] == hDlg)
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						SendMessage(hDlg, WM_CLOSE, 0, 0);
						return TRUE;
					case IDADD:
						ShowWindow(dialog[ADDWINDOW], 5);
						return TRUE;
					case IDSAVE:
						switch (MessageBox(hDlg, TEXT("Say we pee-lanets"), TEXT("Say we!"), MB_ICONINFORMATION | MB_OK)) {
							case IDOK:
								break;
						}
						return TRUE;
					case IDLOAD:
						switch (MessageBox(hDlg, TEXT("Low ad pee-lanets"), TEXT("Low ad!"), MB_ICONINFORMATION | MB_OK)) {
							case IDOK:
								break;
						}
						return TRUE;
					case IDSEND:
						switch (MessageBox(hDlg, TEXT("See end pee-lanets"), TEXT("See end!"), MB_ICONINFORMATION | MB_OK)) {
							case IDOK:
								break;
						}
						return TRUE;
				}
			else if (dialog[ADDWINDOW] == hDlg)
				switch (LOWORD(wParam))
				{
					case IDCANCEL:
						EndDialog(hDlg, 0);
						return TRUE;
					case IDOK:
						switch (MessageBox(hDlg, TEXT("Add pee-lanet"), TEXT("In from a tion"), MB_ICONINFORMATION | MB_OK)) {
							case IDOK:
								EndDialog(hDlg, 0);
								break;
						}
						return TRUE;
				}
			break;

		case WM_CLOSE:
			if (dialog[MAINWINDOW] == hDlg) {
				switch (MessageBox(hDlg, TEXT("Don't go man!"), TEXT("Heading towards the door..."), MB_ICONSTOP | MB_ABORTRETRYIGNORE)) {
					case IDIGNORE:
						DestroyWindow(hDlg);
						break;
					case IDRETRY:
						return FALSE;
					default:
						return TRUE;
				}
			}
			else {
				EndDialog(hDlg, 0);
				//EMPTY ALL INPUT?
			}
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;

	}

	return FALSE;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	//MessageBox(NULL, "It works man?\n", "A cool mbop", 0);

	dialog = malloc(sizeof(HWND) * 2);
	dialog[MAINWINDOW] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
	dialog[ADDWINDOW] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), 0, DialogProc, 0);
	//A man, a plan, a canal, Panama
	ShowWindow(dialog[MAINWINDOW], nCmdShow);



	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(dialog[MAINWINDOW], &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	return 0;

}