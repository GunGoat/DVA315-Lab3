#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "list.h"

// TODO: This should probably be calculated based on the deltatime between frames
#define DT 15

// Creates a new list 
List* CreateList() {
	List *list = (List*)malloc(sizeof(List));
	if (list == NULL) {
		printf("Could not allocate memory for list");
		return NULL;
	}

	list->head = NULL;
	printf("Successfully created a new list\n");
	return list;
}

// Adds a new node to the end of the list
void AddLastNode(List *list, planet_type *node) {
	if (list->head == NULL) {
		list->head = node;
		printf("Successfully added %s to the list\n", node->name);
	}
	else {
		planet_type *iterator = list->head;
		while (iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = node;
		printf("Successfully added %s to the list\n", node->name);
	}
}

// Adds a new node to the beginning of the list
void AddFirstNode(List *list, planet_type *node) {
	node->next = list->head;
	list->head = node;
	printf("Successfully added %s to the list\n", node->name);
}

//Adds test planets (remove before realese)
void AddTestNodes(List *list) {
	//					name		X-coord		Y-coord		X-velocity	Y-velocity		mass			next	life	pid	
	//planet_type p1 = { "Planet1",	300,		300,		0,			0,				pow(10,8),		NULL,	15,		"This is planet 1" };
	//planet_type p2 = { "Planet2",	200,		300,		0,			0.008,			1000,			NULL,	15,		"This is planet 2" };

	//AddLastNode(list, p1);
	//AddLastNode(list, p2);
}

// Remove the specified node
void RemoveNode(List *list, planet_type *p) {
	if (list->head == NULL) {
		printf("Could not remove any nodes since the list is empty\n");
		return;
	}
	else if (list->head == p) {
		planet_type *temp = list->head;
		list->head = list->head->next;
		free(temp);
		printf("Successfully removed node with the name \"%s\" and the pid \"%s\" from the list\n", p->name, p->pid);
		return;
	}
	else {
		planet_type *previous = list->head;
		planet_type *iterator = list->head->next;
		while (iterator != NULL) {
			if (iterator == p) {
				previous->next = iterator->next;
				free(iterator);
				printf("Successfully removed node with the name \"%s\" and the pid \"%s\" from the list\n", p->name, p->pid);
				return;
			}
			previous = iterator;
			iterator = iterator->next;
		}
		printf("Successfully removed node with the name \"%s\" and the pid \"%s\" from the list\n", p->name, p->pid);
	}
}

// Prints out the nodes in the list to the console
void PrintList(List *list) {
	if (list == NULL) {
		printf("Error! Could not print list\n");
	}
	else {
		planet_type *iterator = list->head;
		printf("\nBeginning of list\n");
		while (iterator != NULL) {
			printf("\n\tname: %s\n", iterator->name);
			printf("\tposition: (%e, %e)\n", iterator->sx, iterator->sy);
			printf("\tvelocity: (%e, %e)\n", iterator->vx, iterator->vy);
			printf("\tmass: %e\n", iterator->mass);
			printf("\tlife: %d\n", iterator->life);
			printf("\tpid: %s\n", iterator->pid);

			iterator = iterator->next;
		}
		printf("\nEnd of list\n\n");
	}
}

void Trajectory(List *list, planet_type *p1) {

	planet_type *p2 = list->head;
	double G = 6.67259*pow(10, -11);	// The universal gravitational constant
	double tot_ax = 0, tot_ay = 0;		// The total acceleration on the y-axis and x-axis
	double r, a, ax, ay;
	printf("(OLD) name: %s\nposition: (%e, %e)\nvelocity: (%e, %e)\n\n", p1->name, p1->sx, p1->sy, p1->vx, p1->vy);
	while (p2 != NULL){
		if (p1 != p2) {
			r = sqrt(pow((p1->sx - p2->sx), 2) + pow((p1->sy - p2->sy), 2)); // Equation 2: Distance between the planets center of mass
			a = G * p2->mass / (r * r);		// Equation 1: Acceleration of planet p1
			ax = a * (p2->sx - p1->sx) / r;	// Equation 3: Acceleration of planet p1 on the X-axis
			ay = a * (p2->sy - p1->sy) / r;	// Equation 4: Acceleration of planet p1 on the Y-axis

			printf("\tname: %s \n\tr: %e \n\ta: %e\n\tax: %e\n\tay: %e\n\n", p2->name, r, a, ax, ay);

			tot_ax += ax;
			tot_ay += ay;
		}
		p2 = p2->next;
	}
	printf("\taccelaeration on y: %e m/s\n", tot_ay);
	printf("\taccelaeration on x: %e m/s\n\n", tot_ax);

	double vx = p1->vx + tot_ax * DT;	// Equation 5:	Velocity of planet p1 on the X-axis
	double vy = p1->vy + tot_ay * DT;	//				Velocity of planet p1 on the Y-axis
	double sx = p1->sx + vx * DT;		// Equation 6:	New position of planet p1 on the X-axis
	double sy = p1->sy + vy * DT;		//				New position of planet p1 on the Y-axis

	p1->vx = vx;
	p1->vy = vy;
	p1->sx = sx;
	p1->sy = sy;
	printf("(NEW) name: %s\nposition: (%e, %e)\nvelocity: (%e, %e)\n\n", p1->name, p1->sx, p1->sy, p1->vx, p1->vy);
}

/*
int main() {
	List* list = CreateList();
	//					name		X-coord				Y-coord		X-velocity	Y-velocity			mass				next	life	pid	
	planet_type p0 = { "Sun",		0.0,				0.0,		0.0,		0,					1.9885*pow(10,30),	NULL,	15,		"The sun" };
	planet_type p1 = { "Mercury",	57 * pow(10,9),		0.0,		0.0,		47.89*pow(10,3),	3.3011*pow(10,23),	NULL,	15,		"The planet closest to the sun" };
	planet_type p2 = { "Venus",		108 * pow(10,9),	0.0,		0.0,		35.03*pow(10,3),	4.8675*pow(10,24),	NULL,	15,		"this be a cat" };
	planet_type p3 = { "Earth",		150 * pow(10,9),	0.0,		0.0,		29.79*pow(10,3),	5.9723*pow(10,24),	NULL,	15,		"The vanilla planet" };
	planet_type p4 = { "Mars",		228 * pow(10,9),	0.0,		0.0,		24.13*pow(10,3),	6.4171*pow(10,23),	NULL,	15,		"The roman god of war" };
	planet_type p5 = { "Jupiter",	779 * pow(10,9),	0.0,		0.0,		13.06*pow(10,3),	1.8982*pow(10,27),	NULL,	15,		"The biggest planet" };
	planet_type p6 = { "Saturn",	1.43*pow(10,12),	0.0,		0.0,		9.64*pow(10,3),		5.6846*pow(10,26),	NULL,	15,		"The planet with a ring" };
	planet_type p7 = { "Uranus",	2.88*pow(10,12),	0.0,		0.0,		6.81*pow(10,3),		8.6810*pow(10,25),	NULL,	15,		"The butt of many jokes" };
	planet_type p8 = { "Neptune",	4.50*pow(10,12),	0.0,		0.0,		5.43*pow(10,3),		1.0243*pow(10,26),	NULL,	15,		"The farthest away planet" };
	planet_type p9 = { "Pluto",		5.91*pow(10,12),	0.0,		0.0,		4.74*pow(10,3),		1.3030*pow(10,22),	NULL,	15,		"The wannabe planet" };

	AddLastNode(list, p1);
	AddLastNode(list, p2);
	AddLastNode(list, p3);
	AddLastNode(list, p4);
	AddLastNode(list, p5);
	AddLastNode(list, p6);
	AddLastNode(list, p7);
	AddLastNode(list, p8);
	AddLastNode(list, p9);
	RemoveNode(list, "Pluto");

	PrintList(list);

	Trajectory(list, &p1);

	printf("%lf %lf", p1.sx, p1.sy);
	scanf("%s");

	return 0;
}
*/