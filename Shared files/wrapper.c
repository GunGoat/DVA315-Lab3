#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "wrapper.h"

#define TIMERID			100  /* id for timer that is used by the thread that manages the window where graphics is drawn */
#define DEFAULT_STACK_SIZE	1024
#define TIME_OUT			MAILSLOT_WAIT_FOREVER 

/* ATTENTION!!! calls that require a time out, use TIME_OUT constant, specifies that calls are blocked forever */


DWORD threadCreate(LPTHREAD_START_ROUTINE threadFunc, LPVOID threadParams) {

	/* Creates a thread running threadFunc */
	/* optional parameters (NULL otherwise)and returns its id! */

	HANDLE thread = CreateThread(NULL, 0, threadFunc, threadParams, 0, NULL);
	return GetThreadId(thread);
}

HANDLE threadOpen(DWORD threadID) {

	/* Get handle of an open thread */

	return OpenThread(THREAD_ALL_ACCESS, NULL, threadID);
}

HANDLE mailslotCreate (char *name) {

	/* Creates a mailslot with the specified name and returns the handle */
	/* Should be able to handle a messages of any size */

	char dest[30] = "\\\\.\\mailslot\\";
	strcat(dest, name);

	HANDLE mailSlot = CreateMailslot(dest, DEFAULT_STACK_SIZE, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES)NULL);
	
	if (mailSlot == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailslot failed with %d\n", GetLastError());
		return FALSE;
	}
	else
		printf("Mailbox up and running!\n");

	return mailSlot;
}

HANDLE mailslotConnect (char *name) {

	/* Connects to an existing mailslot for writing */
	/* and returns the handle upon success */

	HANDLE hansel;
	char dest[30] = "\\\\.\\mailslot\\";
	strcat(dest, name);

	hansel = CreateFile(dest, GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	/*if (hansel == INVALID_HANDLE_VALUE)
	{
		printf("ConnectMailslot failed with %d\n", GetLastError());
		//return FALSE;
	}
	else
		printf("Mailbox connection established!\n");*/

	return hansel;
}

int mailslotWrite(HANDLE mailSlot, void *msg, int msgSize) {

	/* Write a msg to a mailslot, return nr */
	/* of successful bytes written         */

	unsigned long cbWritten;
	int success = 5;
	success = WriteFile(mailSlot, msg, msgSize, &cbWritten, NULL);
	if (!success)
		printf("WriteFile error: %d\n", GetLastError());

	/* Make sure all the bytes were read */
	else if (msgSize != cbWritten)
		printf("WriteFile did not read the correct number of bytes!\n");

	else
		printf("Message sent!\n");

	return cbWritten;
}

int	mailslotRead (HANDLE mailbox, void *msg, int msgSize) {

	/* Read a msg from a mailslot, return nr */
	/* of successful bytes read              */

	unsigned long cbRead = 0;
	int fResult;
	fResult = ReadFile(mailbox, msg, msgSize, &cbRead, 0);
	/*if (!fResult)
		printf("ReadFile error: %d\n", GetLastError());

	// Make sure all the bytes were read
	else if (msgSize != cbRead)
		printf("ReadFile did not read the correct number of bytes!\n");

	else
		printf("Message received!\n");*/

	return cbRead;
}

int mailslotClose(HANDLE mailSlot){
	
	/* close a mailslot, returning whatever the service call returns */
	return CloseHandle(mailSlot);;
}


/******************** Wrappers for window management, used for lab 2 and 3 ***********************/
/******************** DONT CHANGE!!! JUST FYI ******************************************************/


HWND windowCreate (HINSTANCE hPI, HINSTANCE hI, int ncs, char *title, WNDPROC callbackFunc, int bgcolor) {

  HWND hWnd;
  WNDCLASS wc; 

  /* initialize and create the presentation window        */
  /* NOTE: The only important thing to you is that we     */
  /*       associate the function 'MainWndProc' with this */
  /*       window class. This function will be called by  */
  /*       windows when something happens to the window.  */
  if( !hPI) {
	 wc.lpszClassName = "GenericAppClass";
	 wc.lpfnWndProc = callbackFunc;          /* (this function is called when the window receives an event) */
	 wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	 wc.hInstance = hI;
	 wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	 wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	 wc.hbrBackground = (HBRUSH) bgcolor;
	 wc.lpszMenuName = "GenericAppMenu";

	 wc.cbClsExtra = 0;
	 wc.cbWndExtra = 0;

	 RegisterClass( &wc );
  }

  /* NOTE: This creates a window instance. Don't bother about the    */
  /*       parameters to this function. It is sufficient to know     */
  /*       that this function creates a window in which we can draw. */
  hWnd = CreateWindow( "GenericAppClass",
				 title,
				 WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
				 0,
				 0,
				 CW_USEDEFAULT,
				 CW_USEDEFAULT,
				 NULL,
				 NULL,
				 hI,
				 NULL
				 );

  /* NOTE: This makes our window visible. */
  ShowWindow( hWnd, ncs );
  /* (window creation complete) */

  return hWnd;
}

void windowRefreshTimer (HWND hWnd, int updateFreq) {

  if(SetTimer(hWnd, TIMERID, updateFreq, NULL) == 0) {
	 /* NOTE: Example of how to use MessageBoxes, see the online help for details. */
	 MessageBox(NULL, "Failed setting timer", "Error!!", MB_OK);
	 exit (1);
  }
}


/******************** Wrappers for window management, used for lab  3 ***********************/
/*****  Lab 3: Check in MSDN GetOpenFileName and GetSaveFileName  *********/
/**************  what the parameters mean, and what you must call this function with *********/


HANDLE OpenFileDialog(char* string, DWORD accessMode, DWORD howToCreate)
{

	OPENFILENAME opf;
	char szFileName[_MAX_PATH]="";

	opf.Flags				= OFN_SHOWHELP | OFN_OVERWRITEPROMPT; 
	opf.lpstrDefExt			= "dat";
	opf.lpstrCustomFilter	= NULL;
	opf.lStructSize			= sizeof(OPENFILENAME);
	opf.hwndOwner			= NULL;
	opf.lpstrFilter			= NULL;
	opf.lpstrFile			= szFileName;
	opf.nMaxFile			= _MAX_PATH;
	opf.nMaxFileTitle		= _MAX_FNAME;
	opf.lpstrInitialDir		= NULL;
	opf.lpstrTitle			= string;
	opf.lpstrFileTitle		= NULL ; 
	
	if(accessMode == GENERIC_READ)
		GetOpenFileName(&opf);
	else
		GetSaveFileName(&opf);

	return CreateFile(szFileName, 
		accessMode, 
		0, 
		NULL, 
		howToCreate, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);


}

