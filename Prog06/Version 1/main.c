/*--------------------------------------------------------------------------------------------------------------+ 
Title: main.c
Author: Jean Yves Herve 
Edited: Ian Pascoe 12-3-2018
For Prog06 Due 12-5-2018

This is the 1st version of 2. It takes 1 argument which is a file containing specifications for the GLUT window
as well as a function that will be drawn out, the number of replacement rules to be implemented and lastly, the
rules themselves. This program is very buggy and will sometimes not draw the last segment of the function it
receives. Not quite sure why or how this is possible but it continues to frustrate me.

Compiled using:
	gcc main.c gl_frontEnd.c -lm -lGL -lglut -lpthread -o v1

Sample call to executable:
	./v1 Rules/specs.txt
+---------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
//
#include "Node.h"
#include "gl_frontEnd.h"

// Struct I added to store rules.
typedef struct Rule
{
	char left[1000];
	char right[1000];
	float scale;
} Rule;

// Struct for the data passed to threads
typedef struct tInfo
{
	pthread_t tID;
	int index;
	Node* leftEQ;
	Node* rightEQ;
	float scale;
} tInfo;

//	is defined by some compilers, not by some others.
#ifndef M_PI
	#define     M_PI				3.1415927f
#endif

//	Enable this if you want/need to use joinable threads in your code.
//	The main thread (which *must* be the glut/GL thread absolutely cannot
//	be blocked waiting for a thread.  Otherwise the display won't refresh
//	anymore.
#define USE_JOINABLE_THREADS	0

//==================================================================================
//	Function prototypes
//==================================================================================
void displayGridPane(void);
void displayStatePane(void);

//----------------------------------------------------------------------------------
// Added by Ian Pascoe:
Node* array_to_list(char* initialString);
int find_subset(Node** set, Node** subSet, Node** replacement, float scale);
void replace_subset(Node** replace, Node** instanceStart, Node** instanceEnd, float scale);

void* threadFunc(void* arg);
//----------------------------------------------------------------------------------

void swapGrids(void);
unsigned int cellNewState(unsigned int i, unsigned int j);

#if USE_JOINABLE_THREADS
	void* mainThreadFunc(void*);
#endif

//==================================================================================
//	Application-level global variables
//==================================================================================

//	Don't touch
GLint gWindowWidth, gWindowHeight;

//	Feel free to rename these variables if you don't like the identifier
Node* curveList;
GLfloat angleUnit;
GLfloat startX, startY;
GLfloat initialLength;

// Mutex lock for version 1 of assignment
pthread_mutex_t listLock;

//==================================================================================
//	These are callback functions there are setup in glFrontEnd.c
//	You should never call them directly:  They will be called by glut.
//	I marked parts that are "don't touch."
//==================================================================================

//	This callback function is called when a keyboard event occurs
//	I include it here just in case you want to add controls to speedup or
//	slowdown the threads (adjusting the base sleep tim)
void keyboardHandler(unsigned char c, int x, int y)
{
	switch (c)
	{
		//	'ESC' --> exit the application
		case 27:
			exit(0);
			break;
		
		default:
			break;
	}
	glutPostRedisplay();
}


void drawCurve(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(startX, startY, 0);
	glColor3f(1.f, 0.f, 0.f);

	pthread_mutex_lock(&listLock);

	Node* currentNode = curveList;
	while (currentNode != NULL)
	{
		switch(currentNode->letter)
		{
			case 'F':
			glBegin(GL_LINES);
				glVertex2f(0.f, 0.f);
				glVertex2f(currentNode->length, 0.f);
			glEnd();
			glTranslatef(currentNode->length, 0.f, 0.f);
			break;

			case 'f':
			glTranslatef(currentNode->length, 0.f, 0.f);
			break;

			case '+':
			glRotatef(angleUnit, 0.f, 0.f, 1.f);
			break;

			case '-':
			glRotatef(-angleUnit, 0.f, 0.f, 1.f);
			break;
			
			default:
				printf("Invalid character in curve string.\n");
				exit(10);
		}
		currentNode = currentNode->next;
	}

	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();

	pthread_mutex_unlock(&listLock);
}

#if USE_JOINABLE_THREADS
	void* mainThreadFunc(void* arg)
	{
		//	This thread doesn't need any argument all the important variables
		//	would be global, unless you want to pass the path to the input file
		
		//	This is presumably where you would start creating the first threads
		int count = 0;
		int keepGoing = 1;
		while (keepGoing)
		{
			//	presumably here you would be looking for threads that terminate
			//	and spawn new ones.
			printf("main thread func looping [%d]\n", count++);
			//	I have nothing to do but sleep and loop forever.
			usleep(10000);
		}
		
		//	Nothing interesting to return
		return NULL;
	}
#endif

// This function simply turns an array to a doubly-linked list
Node* array_to_list(char* array)
{
	//	Create the head node
	Node* newList = (Node*) calloc(1, sizeof(Node));
	newList->prev = NULL;
	newList->head = newList;

	// Can't have a list with no nodes
	if(strlen(array) == 0)
	{
		fprintf(stderr, "Invalid string provided\n");
		exit(EXIT_FAILURE);
	}

	// Specifically handles lists with 1 node
	if(strlen(array) == 1)
	{
		newList->index = 0;
		newList->letter = array[0];
		newList->length = initialLength;
		newList->tail = newList;
		newList->next = NULL;
		goto done;
	}

	// Create tail node
	Node* newListTail = (Node*) calloc(1, sizeof(Node));
	newListTail->index = strlen(array)-1;
	newListTail->letter = array[newListTail->index];
	newListTail->next = NULL;
	newListTail->head = newList;
	newListTail->tail = newListTail;

	newList->tail = newListTail;

	//	I will use this pointer as I move along my list
	Node* currentNode = newList;

	//	Add contents of string read from file to the new curve list
	for(int i=0; i<strlen(array)-1; i++)
	{
		if(array[i] == 'F' || array[i] == 'f' || array[i] == '+' || array[i] == '-')
		{
			currentNode->index = i;
			currentNode->head = newList;
			currentNode->tail = newListTail;
			currentNode->letter = array[i];
			currentNode->length = initialLength;

			// Not second to last element in array:
			// next is the next element in array
			if(i != strlen(array)-2)
			{
				currentNode->next = (Node*) calloc(1, sizeof(Node));
				currentNode->next->prev = currentNode;
				currentNode = currentNode->next;
			}

			// Second to last element in array:
			// next is the tail created earlier
			else
			{
				currentNode->next = newListTail;
				currentNode->next->prev = currentNode;
			}
		}

		else
		{
			fprintf(stderr, "Invalid character in curve string\n");
			exit(EXIT_FAILURE);
		}	
	}

	done:
	return newList;
}

//=======================================================================
//	Finds instances of the left side of the replacement rule.
//	Nodes start and end will hold the pointers of the first and last
//	nodes that hold the letters of the left side instance.
//=======================================================================
int find_subset(Node** set, Node** subSet, Node** replacement, float scale)
{
	Node* setNode = *set;
	Node* subSetNode = *subSet;
	Node* replaceSet = *replacement;
	Node* startPtr = *set;
	Node* endPtr = NULL;
	
	while(setNode != NULL) 
	{
		printf("Making pass\n");
		if(setNode->letter == subSetNode->letter)
		{
			printf("Match Found\n");
			if(subSetNode == subSetNode->tail)
			{
				printf("Full Match Found\n");
				endPtr = setNode;
				setNode = setNode->next;

				// Once match is found, replace it with the replacement
				replace_subset(&replaceSet, &startPtr, &endPtr, scale);
				printf("Replacement completed\n");

				usleep(10);

				subSetNode = subSetNode->head;
				startPtr = setNode;

			}

			else
			{
				printf("Still searching for rest of key\n");
				subSetNode = subSetNode->next;
				setNode = setNode->next;
			}
		}
		
		else 
		{
			printf("Not a match\n");
			subSetNode = subSetNode->head;
			startPtr = startPtr->next;
			setNode = startPtr;
		}

		usleep(2);
	}

	return 0;
}

//=============================================================================================
// This function replaces the instance of the left side of the replacement equation with the
// right side of the equation. It will be called within the find subset function above.
// ============================================================================================
void replace_subset(Node** replaceSet, Node** instanceStart, Node** instanceEnd, float scale)
{	
	// Take care of scaling first
	Node* counter = *replaceSet;
	while(counter != NULL)
	{
		counter->length = ((*instanceStart)->length)*scale;
		counter = counter->next;
	} free(counter);

	Node* replacement = *replaceSet;

	// Below handles the various locations where the equation could be.
	// 1. If the left side of the equation is the original equation.
	// 
	if(*instanceStart == curveList && *instanceEnd == curveList->tail)
	{
		printf("Start=End\n");
		*instanceStart = replacement;
	}

	// 2. If left side of equation is at the head of the original.
	else if(curveList == *instanceStart)
	{
		printf("Head\n");
		replacement->tail->next = (*instanceEnd)->next;
		*instanceStart = replacement;

		counter = curveList;
		while(counter != NULL)
		{
			counter->head = replacement;
			counter->tail = curveList->tail;
			counter = counter->next;
		}
	}

	// 3. If left side of equation is at end of the original
	else if(*instanceEnd == curveList->tail)
	{
		printf("Tail\n");
		replacement->prev = *instanceStart->prev;
		*instanceStart->prev->next = replacement;
		
		counter = curveList;
		while(counter != NULL)
		{
			counter->head = curveList;
			counter->tail = replacement->tail;
			counter = counter->next;
		}
	}

	// 4. Any other location in the original equation.
	else
	{
		printf("Other\n");
		replacement->prev = start->prev;
		replacement->prev->next = replacement;
		end = end->next;
		replacement->tail->next = end;
		replacement->tail->next->prev = replacement->tail;

		counter = curveList;
		while(counter != NULL)
		{
			counter->head = curveList->head;
			counter->tail = curveList->tail;
			counter = counter->next;
		}
	}
}

// Function for the threads to perform
void* threadFunc(void* arg)
{
	// Get thread info from the struct passed to arg
	tInfo* info = (tInfo *) arg;

	// Set our lock
	pthread_mutex_lock(&listLock);

	printf("Thread %d has acquired the mutex\n", info->index+1);
	
	// Perform critical section ----> find left side and replace it with the right side.
	int err = find_subset(&curveList, &info->leftEQ, &info->rightEQ, info->scale);

	if(err == -1)
	{
		printf("Thread %d has released the mutex\nRule not found\n", info->index+1);
		pthread_mutex_unlock(&listLock);
		pthread_exit(&err);
	}

	// I just print out the resulting equation for record keeping.
	printf("Curve after replacement %d: ", info->index+1);
	Node* a = curveList;
	while(a != NULL)
	{
		printf("%c", a->letter);
		a = a->next;
	} printf("\n");

	printf("Thread %d has released the mutex\n", info->index+1);
	// Unlock mutex
	pthread_mutex_unlock(&listLock);

	return NULL;
}

//------------------------------------------------------------------------
//	Added ability to take the 1 argument:
//	./prog06 <filepath holding specs of curve>
//------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	printf("==================================================================================================\n");
	//	Assert proper argument
	assert(argc = 2);

	//	Input file path variable
	char* spec_file = argv[1];

	//	Open input file for reading
	FILE *fptr = fopen(spec_file, "r");

	//	Necessary variables for the different specs
	gWindowWidth;
	gWindowHeight;
	startX;
	startY;
	angleUnit;
	initialLength;
	char initialString[100];
	int numRules=0;
	Rule* rules;
	char buf[1000];

	//=================================================
	//	Here I assume the format of the input file 
	//	from the assignment instructions:
	//
	//	line1: <window width> <window height>
	//	line2: <start x-coord> <start y-coord>
	//	line3: <initial segment length>
	//	line4: <angle unit in degrees>
	//	line5: <initial curve string>
	//	line6: <number of rules>
	//	line7: <left side> <right side> <scale>
	//	>line7: ... more rules
	//
	//	I print the variables just for book-keeping
	//	and console log.
	//=================================================
	int line=1;
	int ruleIndex=0;
	while(fgets(buf, 1000, fptr) != NULL)
	{
		switch(line)
		{
			case 1:
				sscanf(buf, "%d %d\n", &gWindowWidth, &gWindowHeight);
				printf("Window Dimensions:\t\t%d %d\n", gWindowWidth, gWindowHeight);
				break;

			case 2:
				sscanf(buf, "%f %f\n", &startX, &startY);
				printf("Start Coordinates:\t\t%.2f %.2f\n", startX, startY);
				break;

			case 3:
				sscanf(buf, "%f\n", &initialLength);
				printf("Initial Segment Length:\t\t%.2f\n", initialLength);
				break;
			
			case 4:
				sscanf(buf, "%f\n", &angleUnit);
				printf("Angle of Rotation:\t\t%.2f\n", angleUnit);				
				break;
			
			case 5:
				sscanf(buf, "%s\n", initialString);
				printf("Initial String:\t\t\t%s\n", initialString);
				break;

			case 6:
				sscanf(buf, "%d\n", &numRules);
				printf("# of Rules to Apply:\t\t%d\n", numRules);
				rules = calloc(numRules, sizeof(Rule));
				break;

			default:
				sscanf(buf, "%s %s %f\n", rules[ruleIndex].left, rules[ruleIndex].right, &rules[ruleIndex].scale);
				ruleIndex++;
				break;
		} line++;
	} 
	printf("==================================================================================================\n");

	// Use new convert to list function to convert initial string to the required list format.
	curveList = array_to_list(initialString);

	// Initialize the mutex
	if(pthread_mutex_init(&listLock, NULL) != 0)
	{
		printf("Failed to initialize the mutex\n");
		exit(EXIT_FAILURE);
	}

	// Create a thread for each rule to be applied
	tInfo threads[numRules];
	for(int i=0; i<numRules; i++)
	{
		threads[i].index = i;
		threads[i].leftEQ = array_to_list(rules[i].left);
		threads[i].rightEQ = array_to_list(rules[i].right);
		threads[i].scale = rules[i].scale;
		
		int createError = pthread_create (&threads[i].tID, NULL, threadFunc, threads + i);
		int detachError = pthread_detach(threads[i].tID);

		if (createError != 0 || detachError != 0) 
		{
			fprintf (stderr, "Could not create thread %d\n", i);
			exit (EXIT_FAILURE);
		}
	}

	usleep(100);
	
	//	If you need joinable threads, then thread creation should happen here
	#if USE_JOINABLE_THREADS
		pthread_t mainThread;
		int errCode = pthread_create (&mainThread, NULL, mainThreadFunc, NULL);
		if (errCode != 0)
		{
			printf ("could not pthread_create main thread. Error code %d: %s\n",
					 errCode, strerror(errCode));
			exit (13);
		}
	#endif

	//	This takes care of initializing glut and the GUI.
	//	You shouldn’t have to touch this
	initializeFrontEnd(argc, argv);
	

	//	glut/gl magic.  Don't touch.
	//	Now we enter the main loop of the program and to a large extend
	//	"lose control" over its execution.  The callback functions that 
	//	we set up earlier will be called when the corresponding event
	//	occurs
	glutMainLoop();
	
	//	This will never be executed (the exit point will be in one of the
	//	call back functions).
	return 0;
}