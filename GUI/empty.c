#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "resource.h"

HWND *dialog;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	MessageBox(NULL, "It works man?\n", "A cool Mbox", 0);

	dialog = malloc(sizeof(HWND) * 2);
	dialog[0] = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, 0);
	dialog[1] = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), 0, 0);

	//ShowWindow(dialog[0], SW_SHOW);
	//ShowWindow(dialog[1], SW_HIDE);

	return 1;

}