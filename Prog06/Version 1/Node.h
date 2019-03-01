//
//  Node.h
//  Prog 06
//
//  Created by Jean-Yves Hervé on 2018-11-28.
//	You cannot remove nor rename any of the fields, and you need to
//	keep them all up to date.  On the other hand, feel free to add any
//	field that your program may need

#ifndef NODE_H
#define NODE_H

typedef struct Node
{
	char letter;
	float length;
	struct Node* prev;
	struct Node* next;

	// Added the following:

	// Index for reference
	int index;

	// Used if nodes are in a list
	struct Node* head;
	struct Node* tail;
} Node;

#endif //	NODE_H
