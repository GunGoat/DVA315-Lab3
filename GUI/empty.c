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

HANDLE srvListMutex;

void responseThread(char* pid);
int sendPlanetsToServer(planet_type* p);
int planetsToFile(planet_type* p, char* filename);
planet_type* planetsFromFile(char* filename);
char* randomize_name();

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
	//SetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_NAME, "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_NAME), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_MASS), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_XPOS), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_YPOS), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_XVEL), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_YVEL), "");
	SetWindowText(GetDlgItem(dialog[ADDWINDOW], IDC_EDIT_LIFE), "");
}

//void resetCounter() {
//	counter = 0;
//	char buffer[50];
//	sprintf(buffer, "Number of Local Planets: %d", counter);
//	SetWindowText(GetDlgItem(dialog[MAINWINDOW], IDC_EDIT_COUNTER), buffer);
//}
void updateCounter() {
	char buffer[50];
	sprintf(buffer, "Number of Local Planets: %d", SendMessage(msgBox, LB_GETCOUNT, 0, 0));
	SetWindowText(GetDlgItem(dialog[MAINWINDOW], IDC_EDIT_COUNTER), buffer);
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
		case IDC_RANDOM_NAME:
			SetDlgItemText(dialog[ADDWINDOW], IDC_EDIT_NAME, randomize_name());
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
					//MessageBox(NULL, "Planets saved!", "Success!", 0);
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
				planet_type* head = malloc(sizeof(planet_type));
				head = loaded;
				if (loaded == NULL) {
					MessageBox(NULL, "Failed to load planets!", "Failure!", 0);
					break;
				}

				msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_LOCAL);

				while (loaded != NULL) {
					SendMessage(msgBox, LB_INSERTSTRING, 0, loaded->name);
					if (loaded->next == NULL) {
						loaded->next = localPlanets;
						localPlanets = head;
						break;
					}
					loaded = loaded->next;
				}
				updateCounter();

			}

			return TRUE;
		case IDSEND:
			if (sendPlanetsToServer(localPlanets) == 0) {
				msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_LOCAL);
				SendMessage(msgBox, LB_RESETCONTENT, 0, 0);
				localPlanets = NULL;
				updateCounter();
			}

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

	srand(time(NULL));

	localPlanets = NULL;

	srvListMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex
	if (srvListMutex == NULL)
	{
		char error[100];
		sprintf(error, "CreateMutex error: %d\n", GetLastError());
		MessageBox(NULL, error, "Error", 0);
	}

	dialog = malloc(sizeof(HWND) * 2);
	dialog[MAINWINDOW] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, MainDialogProc, 0);
	dialog[ADDWINDOW] = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), 0, AddDialogProc, 0);
	//A man, a plan, a canal, Panama
	ShowWindow(dialog[MAINWINDOW], nCmdShow);

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

	return 0;
}


int sendPlanetsToServer(planet_type* p) {
	// TODO: This is pretty much a copy paste of old client code, needs to be updated to work maybe!
	//printf("Adding planet \"%s\" with position (%lf, %lf), velocity (%lf, %lf), mass %lf and life %d\n", p->name, p->sx, p->sy, p->vx, p->vy, p->mass, p->life);
	
	// Connect to the mailbox setup by the server
	HANDLE writeslot = mailslotConnect("mailbox");
	if (writeslot == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, "Failed to get a handle to the mailslot!\nPlease check that the server is online.", "Error!", 0);
		return 1;
	}
	
	DWORD bytesWritten = 0, mutexWaitResult;
	char formattedMessage[200];
	planet_type* old;
	HWND msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_MESSAGE);
	HWND srvBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_SERVER);

	mutexWaitResult = WaitForSingleObject(
		srvListMutex,    // handle to mutex
		INFINITE);  // no time-out interval

	switch (mutexWaitResult) {
	case WAIT_OBJECT_0:
		__try {
			while (p != NULL) {
				SendMessage(srvBox, LB_INSERTSTRING, 0, p->name);
				bytesWritten += mailslotWrite(writeslot, p, sizeof(planet_type));
				sprintf(formattedMessage, "%s was sent to the server", p->name);
				SendMessage(msgBox, LB_INSERTSTRING, 0, formattedMessage);
				
				old = p;
				p = p->next;
				free(old);
			}
			Sleep(100);
		}
		__finally {
			// Always release the mutex
			ReleaseMutex(srvListMutex);
		}
		break;
	case WAIT_ABANDONED:
		// Mutex was abandoned
		MessageBox(NULL, "Critical section error: Server list mutex was abandoned", "Error!", 0);
		return;
	}
	if (bytesWritten == -1)
		MessageBox(NULL, "Error: Failed to send data to the server!", "Error!", 0);

	mailslotClose(writeslot);
	return 0;

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

	char pid[30];
	sprintf(pid, "%ul", GetCurrentProcessId());

	while (1) {

		fread(buff, sizeof(planet_type), 1, file);

		if (feof(file)) {
			break;
		}

		read = calloc(1, sizeof(planet_type));
		read = memcpy(read, buff, sizeof(planet_type));
		strcpy(read->pid, pid);
		read->next = prev;
		prev = read;
	}

	free(buff);
	fclose(file);
	return read;
}


void responseThread(char* pid) {
	DWORD bytesRead, mutexWaitResult;
	HANDLE readslot = mailslotCreate(pid);
	char formattedMessage[256];
	int index;

	HWND msgBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_MESSAGE);
	HWND srvBox = GetDlgItem(dialog[MAINWINDOW], IDC_LIST_SERVER);

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

			mutexWaitResult = WaitForSingleObject(
				srvListMutex,    // handle to mutex
				INFINITE);  // no time-out interval

			switch (mutexWaitResult) {
			case WAIT_OBJECT_0:
				__try {
					if ((index = SendMessage(srvBox, LB_FINDSTRINGEXACT, -1, buffer->name)) == LB_ERR)
						MessageBox(NULL, "Planet not found...", "ERROR", 0);
					else {
						SendMessage(srvBox, LB_DELETESTRING, index, 0);
					}
				}
				__finally {
					// Always release the mutex
					ReleaseMutex(srvListMutex);
				}
				break;
			case WAIT_ABANDONED:
				// Mutex was abandoned
				MessageBox(NULL, "Critical section error: Server list mutex was abandoned", "Error!", 0);
				return;
			}
		}
		Sleep(100);
	}
}

char* randomize_name() {
	char blocks[][3] = { "n\0", "wa\0", "ra\0", "ma\0", "ta\0", "sa\0", "ka\0", "mi\0", "hi\0", "ki\0", "ku\0", "su\0", "fu\0", "mu\0", "nu\0", "ne\0", "te\0", "se\0", "ke\0", "re\0", "ro\0", "yo\0", "ho\0", "mo\0", "no\0", "to\0", "so\0", "ko\0", "ll\0", "m\0", "nn\0", "b\0", "tt\0", "si\0", "ck\0", "a\0", "o\0", "u\0", "i\0", "e\0", "m\0", "s\0", "pp\0" };
	int nblocks = rand() % 5 + 3;
	int i;
	char output[25] = { '\0' }; //8*3+1
	for (i = 0; i < nblocks; i++) {
		strcat(output, blocks[rand() % 43]);
	}
	output[0] = toupper(output[0]);
	return output;
}