/*********************************************
* client.c
*
* Desc: lab-skeleton for the client side of an
* client-server application
* 
* Revised by Dag Nystrom & Jukka Maki-Turja
* NOTE: the server must be started BEFORE the
* client.
*********************************************/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "../Shared files/wrapper.h"

HANDLE readslot;
HANDLE writeslot;
char lock = 0;
char *unread;

int natural_number(int min, int max); // Returns a natural numbers within defined range
double complex_number(double min, double max); // Returns a complex numbers within defined range
void randomize_planet(planet_type* p, char* pid); // Randomize input values for planet
void create_planet(planet_type* p, char* pid); // Manually add input values for planet
void responseThread(); // Receives messages from the server 


void main(void) {

	DWORD bytesWritten;
	srand(time(NULL));
	int loops = 2000;
	char choice;

	unread = malloc(1024);
	unread[0] = '\0';

	char pid[30];
	sprintf(pid, "%ul", GetCurrentProcessId());

	writeslot = mailslotConnect("mailbox");

	if (writeslot == INVALID_HANDLE_VALUE) {
		printf("Failed to get a handle to the mailslot!!\nHave you started the server?\n");
		return;
	}

	readslot = mailslotCreate(pid);
	threadCreate(responseThread, NULL);

	while(1) {

		planet_type* p = malloc(sizeof(planet_type));
		
		// Get user input
		printf("Press 1 to create planet\nPress 2 to randomize planet\n");
		choice = getch();
		if (choice == '1'){
			create_planet(p, pid);
		}
		else if (choice == '2') {
			randomize_planet(p, pid);
		}
		else if (choice == '3') {
			strcpy(p->name, "The big one");
			p->sx = 300;
			p->sy = 300;
			p->vx = 0;
			p->vy = 0;
			p->mass = 100000000;
			p->life = 999999999;
			p->next = NULL;
			strcpy(p->pid, pid);
			mailslotWrite(writeslot, p, sizeof(planet_type));
		}
		else if (choice == '4') {
			strcpy(p->name, "Lefty");
			p->sx = 250;
			p->sy = 300;
			p->vx = 0;
			p->vy = 0.008;
			p->mass = 1000;
			p->life = 999999999;
			p->next = NULL;
			strcpy(p->pid, pid);
			mailslotWrite(writeslot, p, sizeof(planet_type));
			strcpy(p->name, "Righty");
			p->sx = 350;
			p->sy = 300;
			p->vx = 0;
			p->vy = -0.008;
			p->mass = 1000;
			p->life = 999999999;
			p->next = NULL;
			strcpy(p->pid, pid);
			mailslotWrite(writeslot, p, sizeof(planet_type));
		}
		else if (choice == '5') {
			strcpy(p->name, "Circularis");
			p->sx = 300;
			p->sy = 200;
			p->vx = -0.008;
			p->vy = 0.0;
			p->mass = 10000;
			p->life = 999999999;
			p->next = NULL;
			strcpy(p->pid, pid);
			mailslotWrite(writeslot, p, sizeof(planet_type));
			strcpy(p->name, "Ellipticy");
			p->sx = 300;
			p->sy = 250;
			p->vx = -0.015;
			p->vy = 0.0;
			p->mass = 5000;
			p->life = 999999999;
			p->next = NULL;
			strcpy(p->pid, pid);
			mailslotWrite(writeslot, p, sizeof(planet_type));
		}
		else if (choice == '6') {
			strcpy(p->name, "Black hole");
			p->sx = rand()%950+50;
			p->sy = rand() % 950 + 50;
			p->vx = 0.0;
			p->vy = 0.0;
			p->mass = INT_MAX;
			p->life = INT_MAX;
			p->next = NULL;
			strcpy(p->pid, pid);
			mailslotWrite(writeslot, p, sizeof(planet_type));
		}
		else {
			printf("Invalid input\n\n");
		}

		// If the user input was valid try to send request to the server
		if(choice == '1' || choice == '2'){
			printf("Adding planet \"%s\" with position (%lf, %lf), velocity (%lf, %lf), mass %lf and life %d\n", 
				p->name, p->sx, p->sy, p->vx, p->vy, p->mass, p->life);
			bytesWritten = mailslotWrite(writeslot, p, sizeof(planet_type));

			if (bytesWritten!=-1)
				printf("Data sent to server (bytes = %d)\n\n", bytesWritten);
			else
				printf("Failed sending data to server\n\n");

			if (strlen(unread) > 0) {
				unread[strlen(unread) - 2] = '\0';
				printf("The planet(s): %s died while you were away.\n\n", unread);
				unread[0] = '\0';
			}
		}
	}

	mailslotClose (writeslot);

	return;
}

int natural_number(int min, int max)
{
	int range = (max - min);
	return (rand() % range) + min;
}

double complex_number(double min, double max)
{
	double range = (max - min);
	double div = RAND_MAX / range;

	if (rand() % 2 == 0)
		return min + (rand() / div);
	else
		return -(min + (rand() / div));
}

char* randomize_name() {
	char blocks[][3] = { "n\0", "wa\0", "ra\0", "ma\0", "ta\0", "sa\0", "ka\0", "mi\0", "hi\0", "ki\0", "ku\0", "su\0", "fu\0", "mu\0", "nu\0", "ne\0", "te\0", "se\0", "ke\0", "re\0", "ro\0", "yo\0", "ho\0", "mo\0", "no\0", "to\0", "so\0", "ko\0", "ll\0", "m\0", "nn\0", "b\0", "tt\0", "si\0", "ck\0", "a\0", "o\0", "u\0", "i\0", "e\0", "m\0", "s\0", "pp\0" };
	int nblocks = rand() % 5 + 3;
	int i;
	char output[25] = {'\0'}; //8*3+1
	for (i = 0; i < nblocks; i++) {
		strcat(output, blocks[rand() % 43]);
	}
	output[0] = toupper(output[0]);
	return output;
}

void randomize_planet(planet_type* p, char* pid) {
	strcpy(p->name, randomize_name());
	p->sx = (double)natural_number(0, 1000);
	p->sy = (double)natural_number(0, 1000);
	p->vx = complex_number(0.0, 0.01);
	p->vy = complex_number(0.0, 0.01);
	p->mass = (double)natural_number(10000000, 100000000);
	p->life = (double)natural_number(100, 100000);
	p->next = NULL;
	strcpy(p->pid, pid);
}

void create_planet(planet_type* p, char* pid) {
	int c;
	lock = 1;
	printf("Name: ");
	if (fgets(p->name, 20, stdin) != NULL) { // Remove newline from name
		size_t len = strlen(p->name);
		if (len > 0 && p->name[len - 1] == '\n') {
			p->name[--len] = '\0';
		}
	}
	printf("X position: ");
	scanf(" %lf", &p->sx);
	printf("Y position: ");
	scanf(" %lf", &p->sy);
	printf("X velocity: ");
	scanf(" %lf", &p->vx);
	printf("Y velocity: ");
	scanf(" %lf", &p->vy);
	printf("Mass: ");
	scanf(" %lf", &p->mass);
	printf("Life: ");
	scanf(" %d", &p->life);
	p->next = NULL;
	strcpy(p->pid, pid);
	
	while ((c = getchar()) != '\n' && c != EOF);//Remove the newline from input stream
	lock = 0;
}

void responseThread() {
	DWORD bytesRead;

	//char buffer[1024];
	planet_type* buffer = malloc(sizeof(planet_type));
	while (1) {

		bytesRead = mailslotRead(readslot, buffer, sizeof(planet_type));
		if (bytesRead != 0) {
			planet_type *pt = (planet_type *)malloc(sizeof(planet_type));
			memcpy(pt, buffer, sizeof(planet_type));
			if (lock == 1) {
				strcat(unread, strcat(pt->name, ", "));
			}
			else {
				printf("\n\"%s\" is dead! Long live \"%s\"\n", pt->name, pt->name);
				printf("\nPress 1 to create planet\nPress 2 to randomize planet\n");
			}
			free(pt);
		}
		Sleep(1000);
	}
}