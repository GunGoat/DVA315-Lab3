#ifndef LIST_H
#define LIST_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "../Shared files/wrapper.h"

// TODO: This should probably be calculated based on the deltatime between frames
#define DT 10

typedef struct List {
	planet_type *head;
}List;

// Creates a new list 
List* CreateList();

// Adds a new node to the end of the list
void AddLastNode(List *list, planet_type *node);

// Adds a new node to the beginning of the list
void AddFirstNode(List *list, planet_type *node);

// Remove the specified node
void RemoveNode(List *list, planet_type *p);

// Prints out the nodes in the list to the console
void PrintList(List *list);

// Update the position and velocity of one planet
void Trajectory(List *list, planet_type *p1);

//Adds test planets (remove before realese)
void AddTestNodes(List *list);

#endif /* WRAPPER_H */