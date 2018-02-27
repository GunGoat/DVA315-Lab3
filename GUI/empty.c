#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "resource.h"
#include "../Shared files/wrapper.h"

#define MAINWINDOW 0
#define ADDWINDOW 1

HWND *dialog;
HWND msgBox;
MSG msg;
BOOL ret;
planet_type* localPlanets;
int counter = 0;

HANDLE writeslot;

void responseThread(char* pid);
void sendPlanetsToServer(planet_type* p);
int setupMailboxesAndThreads();
int planetsToFile(planet_type* p, char* filename);
planet_type* planetsFromFile(char* filename);

planet_type* addPlanet() {
	char pid[30];
	sprintf(pid, "%ul", GetCurrentProcessId());
	char buffer[1024];
	planet_type* tempPlanet = malloc(sizeof(planet_type));
	GetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_NAME, buffer, 1024);
	strcpy(tempPlanet->name, buffer);
	GetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_MASS, buffer, 1024);
	tempPlanet->mass = atoi(buffer);
	GetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_XPOS, buffer, 1024);
	tempPlanet->sx = atoi(buffer);
	GetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_YPOS, buffer, 1024);
	tempPlanet->sy = atoi(buffer);
	GetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_XVEL, buffer, 1024);
	tempPlanet->vx = atof(buffer);
	GetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_YVEL, buffer, 1024);
	tempPlanet->vy = atof(buffer);
	GetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_LIFE, buffer, 1024);
	tempPlanet->life = atoi(buffer);
	tempPlanet->next = localPlanets;
	strcpy(tempPlanet->pid, pid);
	return tempPlanet;
}

void resetAddWindow()
{
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_NAME), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_MASS), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_XPOS), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_YPOS), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_XVEL), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_YVEL), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_LIFE), "");
}

void resetCounter() {
	counter = 0;
	SetWindowText(GetDlgItem(dialog[MAINWINDOW], IDC_EDIT_COUNTER), "Number of local planets: 0");
}
void updateCounter() {
	counter += 1;
	SetWindowText(GetDlgItem(dialog[MAINWINDOW], IDC_EDIT_COUNTER), "Number of Local Planets: OVER 9000!!!!!");
}

INT_PTR CALLBACK AddDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, 0);
			resetAddWindow();
			return TRUE;
		case IDOK:
			localPlanets = addPlanet();
			msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_LOCAL);
			SendMessage(msgBox, LB_ADDSTRING, 0, localPlanets->name);
			EndDialog(hDlg, 0);
			updateCounter();
			resetAddWindow();
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		resetAddWindow(hDlg);
		return TRUE;

	}

	return FALSE;
}

INT_PTR CALLBACK MainDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char filename[100];
	OPENFILENAME ofn;

	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_COUNTER), "Number of local planets: 0");
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		case IDADD:
			ShowWindow(dialog[ADDWINDOW], 5);
			return TRUE;
		case IDSAVE:
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = dialog[MAINWINDOW];
			ofn.lpstrFilter = "Planet data files (.dat)\0*.dat\0All Files (*.*)\0*.*\0";
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = "Save planets";
			ofn.lpstrDefExt = ".dat";
			ofn.Flags = OFN_DONTADDTORECENT;

			if (GetSaveFileNameA(&ofn)) {
				if (planetsToFile(localPlanets, filename) == 0) {
					MessageBox(NULL, "Planets saved!", "Success!", 0);
				}
				else {
					MessageBox(NULL, "Failed to open file!", "Failure!", 0);
				}
			}

			return TRUE;
		case IDLOAD:
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = dialog[MAINWINDOW];
			ofn.lpstrFilter = "Planet data files (.dat)\0*.dat\0All Files (*.*)\0*.*\0";
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = "Load planets";
			ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;


			if (GetOpenFileNameA(&ofn))
			{
				planet_type* loaded = planetsFromFile(filename);
				planet_type* temp = malloc(sizeof(planet_type));
				memcpy(temp, loaded, sizeof(planet_type));
				if (loaded == NULL) {
					MessageBox(NULL, "Failed to load planets!", "Failure!", 0);
					break;
				}

				msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_LOCAL);

				planet_type* prev = NULL;
				while (loaded != NULL) {
					SendMessage(msgBox, LB_ADDSTRING, 0, loaded->name);

					prev = loaded;
					loaded = loaded->next;

				}

				prev->next = localPlanets;
				localPlanets = temp;
				MessageBox(NULL, "Successfully loaded planets!", "Success!", 0);
			}

			return TRUE;
		case IDSEND:
			msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_LOCAL);
			SendMessage(msgBox, LB_RESETCONTENT, 0, 0);
			sendPlanetsToServer(localPlanets);
			localPlanets = NULL;
			resetCounter();
			return TRUE;
		}
		break;

	case WM_CLOSE:
		switch (MessageBox(hDlg, TEXT("Don't go man!"), TEXT("Heading towards the door..."), MB_ICONSTOP | MB_ABORTRETRYIGNORE)) {
		case IDIGNORE:
			DestroyWindow(dialog[MAINWINDOW]);
			DestroyWindow(dialog[ADDWINDOW]);
			break;
		case IDRETRY:
			return FALSE;
		default:
			return TRUE;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	//MessageBox(NULL, "It works man?\n", "A cool mbop", 0);

	localPlanets = NULL;

	dialog = malloc(sizeof(HWND) * 2);
	dialog[MAINWINDOW] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, MainDialogProc, 0);
	dialog[ADDWINDOW] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), 0, AddDialogProc, 0);
	//A man, a plan, a canal, Panama
	ShowWindow(dialog[MAINWINDOW], nCmdShow);

	if (setupMailboxesAndThreads() == -1)
		return 0;

	// Needs to be created in main, otherwise dies with the setup function
	char pid[30];
	sprintf(pid, "%ul", GetCurrentProcessId());
	if (threadCreate(responseThread, pid) == 0)
		MessageBox(NULL, "Could not setup mailbox to recieve messages from server", "Error", 0);


	while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(GetActiveWindow(), &msg)) {
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
		MessageBox(NULL, "Failed to get a handle to the mailslot!\nPlease check that the server is online.", "Error!", 0);
		return -1;
	}

	return 0;
}

void sendPlanetsToServer(planet_type* p) {
	// TODO: This is pretty much a copy paste of old client code, needs to be updated to work maybe!
	//printf("Adding planet \"%s\" with position (%lf, %lf), velocity (%lf, %lf), mass %lf and life %d\n", p->name, p->sx, p->sy, p->vx, p->vy, p->mass, p->life);
	DWORD bytesWritten = 0;
	char formattedMessage[200];
	planet_type* old;
	HWND msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_MESSAGE);
	
	while (p != NULL) {
		bytesWritten += mailslotWrite(writeslot, p, sizeof(planet_type));
		sprintf(formattedMessage, "%s was sent to the server", p->name);
		SendMessage(msgBox, LB_INSERTSTRING, 0, formattedMessage);
		old = p;
		p = p->next;
		free(old);
	}
	if (bytesWritten != -1)
		MessageBox(NULL, "All local planets sent!", "Happy day!", 0);
	else
		MessageBox(NULL, "Error: Failed to send data to the server!", "Error!", 0);

}

int planetsToFile(planet_type* p, char* filename) {
	FILE* file;
	planet_type* buff = calloc(1, sizeof(planet_type));

	file = fopen(filename, "wb+");
	if (!file) {
		// Failed to open file
		return 1;
	}
	
	// Successfully opened file
	while (p != NULL) {
		buff = memcpy(buff, p, sizeof(planet_type));
		buff->next = NULL;

		fwrite(buff, sizeof(planet_type), 1, file);

		p = p->next;
	}
	fclose(file);
	free(buff);

	return 0;

}

planet_type* planetsFromFile(char* filename) {
	FILE* file;
	planet_type* read = NULL;
	planet_type* prev = NULL;
	planet_type* buff = calloc(1, sizeof(planet_type));
	file = fopen(filename, "rb");
	if (!file) {
		// Failed to open file
		return NULL;
	}

	while (1) {

		fread(buff, sizeof(planet_type), 1, file);

		if (feof(file)) {
			break;
		}

		read = calloc(1, sizeof(planet_type));
		read = memcpy(read, buff, sizeof(planet_type));

		read->next = prev;
		prev = read;
	}

	free(buff);
	fclose(file);
	return read;
}


void responseThread(char* pid) {
	DWORD bytesRead;
	HANDLE readslot = mailslotCreate(pid);
	char formattedMessage[256];

	HWND msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_MESSAGE);

	//char buffer[1024];
	planet_type* buffer = malloc(sizeof(planet_type));
	while (1) {

		bytesRead = mailslotRead(readslot, buffer, sizeof(planet_type));
		if (bytesRead != 0) {
			switch (buffer->life) {
			case 1:
				sprintf(formattedMessage, "%s has died of natural causes.", buffer->name);
				break;
			case 2:
				sprintf(formattedMessage, "%s went into the light at the top of the universe.", buffer->name);
				break;
			case 3:
				sprintf(formattedMessage, "%s went down to a scary place.", buffer->name);
				break;
			case 4:
				sprintf(formattedMessage, "%s went to the new world to the west.", buffer->name);
				break;
			case 5:
				sprintf(formattedMessage, "%s went east for fame and riches.", buffer->name);
				break;
			default:
				sprintf(formattedMessage, "%s has died, because of death.", buffer->name);
				break;
			}
			SendMessage(msgBox, LB_INSERTSTRING, 0, formattedMessage);
		}
		Sleep(100);
	}
}