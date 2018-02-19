#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "../Shared files/wrapper.h"
#include "postman.h"

#define MAILSLOT "\\\\.\\mailslot\\blert";
#define MAX_MSG_SIZE 100

/*
	... I'm goin' down to the house of sausage, I got my barbecue shoes on...

	Welcome to barbecue pit boys dot cooooooom!
	Today we're cooking up some mailslots here at the pit, and it's real easy to do..

	Now if you haven't had mailslots before, you DEFINITELY gotta check THIS OUT!
*/

HANDLE mutexHandle;

int readThreadFunc() {
	LPTSTR Slot = TEXT(MAILSLOT);
	HANDLE h;
	Mail* mail;
	DWORD dwWaitResult; // Mutex wait result

	mail = malloc(sizeof(Mail));
	mail->msg = malloc(MAX_MSG_SIZE);
	mail->msg[0] = '\0';

	h = mailslotCreate(Slot);

	mutexHandle = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (mutexHandle == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	// Whoa, this is gunna be good
	HANDLE writeThread = threadCreate(writeThreadFunc, NULL);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	WORD saved_attributes;

	/* Save current attributes */
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	saved_attributes = consoleInfo.wAttributes;

	do {
		// Here we're using a DWORD unsigned long to store the size of the message in mailslot, and of course use you favorite unsigned long...
		DWORD msgSize;

		// Wait for mutex
		dwWaitResult = WaitForSingleObject(
			mutexHandle,    // handle to mutex
			INFINITE);  // no time-out interval

		switch (dwWaitResult) {
			case WAIT_OBJECT_0:
				__try {
					int success = GetMailslotInfo(h,
						(LPDWORD)NULL,
						&msgSize,
						(DWORD)NULL,
						(LPDWORD)NULL);

					if (success == 1 && msgSize != MAILSLOT_NO_MESSAGE) {
						mailslotRead(h, mail->msg, msgSize);
						printf_s("Message: ");
						SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
						printf_s("%s\n", mail->msg);
						SetConsoleTextAttribute(hConsole, saved_attributes);
					}
				}
				__finally {
					ReleaseMutex(mutexHandle);
				}
				break;
		}


		Sleep(100);

		for (int i = 0; i < strlen(mail->msg); i++) {
			mail->msg[i] = toupper(mail->msg[i]);
		}
	} while (strcmp(mail->msg, "END") != 0);

	TerminateThread(writeThread, 123123);
	free(mail->msg);
	free(mail);
	mailslotClose(h);
	printf("Reader says: Farewell!");
	Sleep(2000);
}

// Oh man looks good enough to eat already
int writeThreadFunc() {
	LPTSTR Slot = TEXT(MAILSLOT);
	HANDLE h;
	DWORD dwWaitResult; // Mutex wait result
	Mail* mail;
	mail = malloc(sizeof(Mail));
	mail->msg = malloc(MAX_MSG_SIZE);

	h = mailslotConnect(Slot);

	// We're eatin' good tonight Martha!
	do {
		Sleep(500); // You can't put Spam on the BBQ

		printf("Message to send: ");
		fgets(mail->msg, MAX_MSG_SIZE, stdin);
		if ((strlen(mail->msg) > 0) && (mail->msg[strlen(mail->msg) - 1] == '\n'))
			mail->msg[strlen(mail->msg) - 1] = '\0';

		// Wait for mutex
		dwWaitResult = WaitForSingleObject(
			mutexHandle,    // handle to mutex
			INFINITE);		// no time-out interval

		switch (dwWaitResult) {
			case WAIT_OBJECT_0:
				__try {
					mailslotWrite(h, mail->msg, strlen(mail->msg) + 1);
				}
				__finally {
					ReleaseMutex(mutexHandle);
				}
		}

		for (int i = 0; i < strlen(mail->msg); i++) {
			mail->msg[i] = toupper(mail->msg[i]);
		}
	} while (strcmp(mail->msg, "END") != 0);

	free(mail->msg);
	free(mail);
	printf("Writer says: Farewell!\n");
	Sleep(2000);

	// And in the miracle of time, these mailslots are DONE!
}

// As always, we do apoligize for eatin' in front of you like this, 
// but we call this PIT MASTER PRIVILEGE!
