/********************************************************************\
* server.c                                                           *
*                                                                    *
* Desc: example of the server-side of an application                 *
* Revised: Dag Nystrom & Jukka Maki-Turja                     *
*                                                                    *
* Based on generic.c from Microsoft.                                 *
*                                                                    *
*  Functions:                                                        *
*     WinMain      - Application entry point                         *
*     MainWndProc  - main window procedure                           *
*                                                                    *
* NOTE: this program uses some graphic primitives provided by Win32, *
* therefore there are probably a lot of things that are unfamiliar   *
* to you. There are comments in this file that indicates where it is *
* appropriate to place your code.                                    *
* *******************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "../Shared files/wrapper.h"
#include "list.h"

List *list;  
int pid_index = 0;
void planetThread(planet_type *input);
HANDLE planetMutex;


							/* the server uses a timer to periodically update the presentation window */
							/* here is the timer id and timer period defined                          */

#define UPDATE_FREQ     10	/* update frequency (in ms) for the timer */

							/* (the server uses a mailslot for incoming client requests) */



/*********************  Prototypes  ***************************/
/* NOTE: Windows has defined its own set of types. When the   */
/*       types are of importance to you we will write comments*/ 
/*       to indicate that. (Ignore them for now.)             */
/**************************************************************/

LRESULT WINAPI MainWndProc( HWND, UINT, WPARAM, LPARAM );
DWORD WINAPI mailThread(LPVOID);



HDC hDC;		/* Handle to Device Context, gets set 1st time in MainWndProc */
				/* we need it to access the window for printing and drawin */

/********************************************************************\
*  Function: int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)    *
*                                                                    *
*   Purpose: Initializes Application                                 *
*                                                                    *
*  Comments: Register window class, create and display the main      *
*            window, and enter message loop.                         *
*                                                                    *
*                                                                    *
\********************************************************************/

							/* NOTE: This function is not too important to you, it only */
							/*       initializes a bunch of things.                     */
							/* NOTE: In windows WinMain is the start function, not main */

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow ) {

	// Initialize planet thread mutex
	planetMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex
	if (planetMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
	}

	HWND hWnd;
	DWORD threadID;
	MSG msg;
	list = CreateList(); // Initilazie list
	

							/* Create the window, 3 last parameters important */
							/* The tile of the window, the callback function */
							/* and the backgrond color */

	hWnd = windowCreate (hPrevInstance, hInstance, nCmdShow, "Himmel", MainWndProc, COLOR_WINDOW+1);

							/* start the timer for the periodic update of the window    */
							/* (this is a one-shot timer, which means that it has to be */
							/* re-set after each time-out) */
							/* NOTE: When this timer expires a message will be sent to  */
							/*       our callback function (MainWndProc).               */
  
	windowRefreshTimer (hWnd, UPDATE_FREQ);
  

							/* create a thread that can handle incoming client requests */
							/* (the thread starts executing in the function mailThread) */
							/* NOTE: See online help for details, you need to know how  */ 
							/*       this function does and what its parameters mean.   */
							/* We have no parameters to pass, hence NULL				*/
  

	threadID = threadCreate (mailThread, NULL); 
  

							/* (the message processing loop that all windows applications must have) */
							/* NOTE: just leave it as it is. */
	while( GetMessage( &msg, NULL, 0, 0 ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return msg.wParam;
}


/********************************************************************\
* Function: mailThread                                               *
* Purpose: Handle incoming requests from clients                     *
* NOTE: This function is important to you.                           *
/********************************************************************/
DWORD WINAPI mailThread(LPVOID arg) {

	//char buffer[1024];
	planet_type* buffer = malloc(sizeof(planet_type));
	DWORD bytesRead;
	static int posY = 0;
	HANDLE fromClientMailslot;

							/* create a mailslot that clients can use to pass requests through   */
							/* (the clients use the name below to get contact with the mailslot) */
							/* NOTE: The name of a mailslot must start with "\\\\.\\mailslot\\"  */

	
	fromClientMailslot = mailslotCreate ("mailbox");

	for(;;) {				
							/* (ordinary file manipulating functions are used to read from mailslots) */
							/* in this example the server receives strings from the client side and   */
							/* displays them in the presentation window                               */
							/* NOTE: binary data can also be sent and received, e.g. planet structures*/
 
	bytesRead = mailslotRead (fromClientMailslot, buffer, sizeof(planet_type));

	if(bytesRead!= 0) {
		planet_type *input = (planet_type *) malloc(sizeof(planet_type));
		memcpy(input, buffer, sizeof(planet_type)); // Save client input

		threadCreate((LPTHREAD_START_ROUTINE)planetThread, input);// Creates thread for the new planet

							/* NOTE: It is appropriate to replace this code with something */
							/*       that match your needs here.                           */
		posY++;  
							/* (hDC is used reference the previously created window) */							
		//TextOut(hDC, 10, 50+posY%200, str, (DWORD) (strlen(str)+1));
	}
	else {
							/* failed reading from mailslot                              */
							/* (in this example we ignore this, and happily continue...) */
    }
  }

  return 0;
}


/********************************************************************\
* Function: LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM) *
*                                                                    *
* Purpose: Processes Application Messages (received by the window)   *
* Comments: The following messages are processed                     *
*                                                                    *
*           WM_PAINT                                                 *
*           WM_COMMAND                                               *
*           WM_DESTROY                                               *
*           WM_TIMER                                                 *
*                                                                    *
\********************************************************************/
/* NOTE: This function is called by Windows when something happens to our window */

LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
  
	PAINTSTRUCT ps;
	DWORD mutexWaitResult;
	int posX;
	int posY;
	HANDLE context;
	static DWORD color = 0;
	planet_type *iterator;
  
	switch( msg ) {
							/**************************************************************/
							/*    WM_CREATE:        (received on window creation)
							/**************************************************************/
		case WM_CREATE:       
			hDC = GetDC(hWnd);  
			break;   
							/**************************************************************/
							/*    WM_TIMER:         (received when our timer expires)
							/**************************************************************/
		case WM_TIMER:

							/* NOTE: replace code below for periodic update of the window */
							/*       e.g. draw a planet system)                           */
							/* NOTE: this is referred to as the 'graphics' thread in the lab spec. */

							/* here we draw a simple sinus curve in the window    */
							/* just to show how pixels are drawn                  */

			// Wait for mutex
			mutexWaitResult = WaitForSingleObject(
				planetMutex,    // handle to mutex
				INFINITE);  // no time-out interval

			switch (mutexWaitResult) {
			case WAIT_OBJECT_0:
				__try {
					iterator = list->head;
					while (iterator != NULL) {
						posX = (int)iterator->sx;
						posY = (int)iterator->sy;

						//Draws a cube consisting of nine pixel which represents a planet 
						int rad = 3;
						for (int x = -rad; x <= rad; x++) {
							for (int y = -rad; y <= rad; y++) {
								if ((pow(x, 2) + pow(y, 2)) <= rad * rad) {
									SetPixel(hDC, posX + x, posY + y, (COLORREF)color);
								}

							}
						}

						iterator = iterator->next;
						color += 12; // Change color
					}
				}
				__finally {
					// Always release the mutex
					ReleaseMutex(planetMutex);

				}
				break;
			case WAIT_ABANDONED:
				// Mutex was abandoned
				printf("Critical section error: Planet mutex was abandoned");
				return;
			}
			
			windowRefreshTimer(hWnd, UPDATE_FREQ);
			break;
							/****************************************************************\
							*     WM_PAINT: (received when the window needs to be repainted, *
							*               e.g. when maximizing the window)                 *
							\****************************************************************/

		case WM_PAINT:
							/* NOTE: The code for this message can be removed. It's just */
							/*       for showing something in the window.                */
							/* (you can safely remove the following line of code)		 */

			context = BeginPaint(hWnd, &ps);
			EndPaint( hWnd, &ps );
			break;


							/**************************************************************\
							*     WM_DESTROY: PostQuitMessage() is called                  *
							*     (received when the user presses the "quit" button in the *
							*      window)                                                 *
							\**************************************************************/
		case WM_DESTROY:
			PostQuitMessage( 0 );

							/* NOTE: Windows will automatically release most resources this */
     						/*       process is using, e.g. memory and mailslots.           */
     						/*       (So even though we don't free the memory which has been*/     
     						/*       allocated by us, there will not be memory leaks.)      */

			ReleaseDC(hWnd, hDC); /* Some housekeeping */
			break;

							/**************************************************************\
							*     Let the default window proc handle all other messages    *
							\**************************************************************/
		default:
			return( DefWindowProc( hWnd, msg, wParam, lParam )); 
   }
   return 0;
}

int isPlanetDead(planet_type *p) {
	if (--p->life <= 0)
		return 1; //Natural causes
	else if (p->sy <= 0)
		return 2; //Up, up and away!
	else if (p->sy >= 1000)
		return 3; //Highway to hell
	else if (p->sx <= 0)
		return 4; //Go west!
	else if (p->sx >= 1000)
		return 5; //Easter is here!

	return 0;
}

void planetThread(planet_type *p) {

	DWORD mutexWaitResult;
	int dead;

	AddFirstNode(list, p); // Add the planet to the list

	HANDLE writeslot = mailslotConnect(p->pid);

	while (1) {
		// Wait for mutex
		mutexWaitResult = WaitForSingleObject(
			planetMutex,    // handle to mutex
			INFINITE);  // no time-out interval

		switch (mutexWaitResult) {
			case WAIT_OBJECT_0:
				__try {
					Trajectory(list, p); // Update the position

					if ((dead = isPlanetDead(p)) != 0) {
						p->life = dead;
						mailslotWrite(writeslot, p, sizeof(planet_type));
						mailslotClose(writeslot);
						RemoveNode(list, p);
						return;
					}
				}
				__finally {
					// Always release the mutex
					ReleaseMutex(planetMutex);
					Sleep(10); //Take a break

				}
				break;
			case WAIT_ABANDONED:
				// Mutex was abandoned
				printf("Critical section error: Planet mutex was abandoned");
				return;
		}
	}
}