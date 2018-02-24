#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "resource.h"
#include "../Shared files/wrapper.h"

#define MAINWINDOW 0
#define ADDWINDOW 1

HWND *dialog;
MSG msg;
BOOL ret;

HANDLE writeslot;

void responseThread(char* pid);
void sendPlanetToServer(planet_data* p);
int setupMailboxesAndThreads();

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

	if (setupMailboxesAndThreads() == -1)
		return 0;


	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(dialog[MAINWINDOW], &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}

	mailslotClose(writeslot);

	return 0;
}

int setupMailboxesAndThreads() {

	// Connect to the mailbox setup by the server
	writeslot = mailslotConnect("mailbox");
	if (writeslot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return -1;
	}

	char pid[30];
	sprintf(pid, "%ul", GetCurrentProcessId());

	// Create server response listener thread
	if (threadCreate(responseThread, pid) == 0)
		return -1;

	return 0;
}

void sendPlanetToServer(planet_data* p) {
	// TODO: This is pretty much a copy paste of old client code, needs to be updated to work maybe!
	printf("Adding planet \"%s\" with position (%lf, %lf), velocity (%lf, %lf), mass %lf and life %d\n",
		p->name, p->sx, p->sy, p->vx, p->vy, p->mass, p->life);
	DWORD bytesWritten = mailslotWrite(writeslot, p, sizeof(planet_type));

	if (bytesWritten != -1)
		printf("Data sent to server (bytes = %d)\n\n", bytesWritten);
	else
		printf("Failed sending data to server\n\n");

	/*if (strlen(unread) > 0) {
		unread[strlen(unread) - 2] = '\0';
		printf("The planet(s): %s died while you were away.\n\n", unread);
		unread[0] = '\0';
	}*/
}

void responseThread(char* pid) {
	DWORD bytesRead;
	HANDLE readslot = mailslotCreate(pid);

	HWND msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_MESSAGE);
	SendMessage(msgBox, LB_ADDSTRING, 0, "Hej");
	SendMessage(msgBox, LB_ADDSTRING, 0, "Hej2");
	SendMessage(msgBox, LB_ADDSTRING, 0, "Hej3");
	SendMessage(msgBox, LB_ADDSTRING, 0, "Hej4");

	//char buffer[1024];
	planet_type* buffer = malloc(sizeof(planet_type));
	while (1) {

		bytesRead = mailslotRead(readslot, buffer, sizeof(planet_type));
		if (bytesRead != 0) {
			planet_type *pt = (planet_type *)malloc(sizeof(planet_type));
			memcpy(pt, buffer, sizeof(planet_type));
			/*if (lock == 1) {
				strcat(unread, strcat(pt->name, ", "));
			}
			else {
				printf("\n\"%s\" is dead! Long live \"%s\"\n", pt->name, pt->name);
				printf("\nPress 1 to create planet\nPress 2 to randomize planet\n");
			}*/
			free(pt);
		}
		Sleep(1000);
	}
}