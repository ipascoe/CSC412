//================================================================================================
// Title: main.c
// Authors: Jean-Yves Herve, Mike Martel & Ian Pascoe
// For Final Project due 12-20-2018
// 
// VERSION 2
// 
// This program simulates a series of robots, boxes and doors, where the robots are to push
// the boxes into its assigned door. Dimensions of grid and the number of boxes, robots and 
// doors are specified on the command line as arguments. Version two makes the robots run
// simultaneously with the use of POSIX threads.
//================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
//
#include "gl_frontEnd.h"

// For later use with threads
typedef struct Robot
{
	pthread_t threadID;
	int boxIndex;
	int doorIndex;
	int robotLoc[2];
	int boxLoc[2];
}Robot;

//==================================================================================
//	Function prototypes
//==================================================================================

// Provided
void displayGridPane(void);
void displayStatePane(void);

// Edited
void initializeApplication(void);

// Added
void pathToBox(Robot* robot);
void positionRobot(Robot* robot, Direction dir);
void boxPathToDoor(Robot* robot);
void move(Direction dir, Robot* robot);
void push(Direction dir, Robot* robot);
void testRobotSuccess(Robot* robot);
void endSimulation(void);
void* robotThreadFunc(void *);

//==================================================================================
//	Application-level global variables
//==================================================================================

//	Don't touch
extern const int	GRID_PANE, STATE_PANE;
extern const int 	GRID_PANE_WIDTH, GRID_PANE_HEIGHT;
extern int	gMainWindow, gSubwindow[2];

//	The state grid and its dimensions (arguments to the program) -- Don' rename
int** grid;
int numRows = -1;	//	height of the grid
int numCols = -1;	//	width
int numBoxes = -1;	//	also the number of robots
int numDoors = -1;	//	The number of doors.

int numLiveThreads = 0;		//	the number of live robot threads

//	robot sleep time between moves (in microseconds)
const int MIN_SLEEP_TIME = 1000;
int robotSleepTime = 100000;

//	An array of C-string where you can store things you want displayed
//	in the state pane to display
const int MAX_NUM_MESSAGES = 8;
const int MAX_LENGTH_MESSAGE = 32;
char** message;
int numMessages = 0;

// These are the global coordinate array pointers for doors
int** doorLoc;
int* doorAssign;

Robot* robots;

// Output file pointer
FILE* fp;

pthread_mutex_t renderLock;
pthread_mutex_t logLock;

//===================================================================================
// Function Implementations
//===================================================================================

// -----------------------------------------------------
// Finds the route for the robot to get to a
// coordinate adjecent to the box
// -----------------------------------------------------
void pathToBox(Robot* robot)
{	
	// variables necessary for the algorithm passed from the struct
	int* boxLoc = robot->boxLoc;
	int* robotLoc = robot->robotLoc;

	// extract elements from the coordinate arrays for the algorithm
	int robotCol = robotLoc[1];
    int robotRow = robotLoc[0];
	int boxCol = boxLoc[1];
    int boxRow = boxLoc[0];

	// variables for the x and y displacements
	// robot to box
	int displacementX = boxCol - robotCol;
    int displacementY = boxRow - robotRow;

	//---------------------------------------------------
	// These conditional equations move the robot 
	// from its original position to the robot
	//---------------------------------------------------
	// These conditionals get the robot to the x-coord +or- 1 of the box
	if(displacementX < 0 && displacementY != 0)
	{
		for(int i=abs(displacementX); i>0; i--)
		{
			move(WEST, robot);
			displacementX = robot->boxLoc[1]-robot->robotLoc[1];
		}

		if(displacementY < 0)
		{
			for(int i=abs(displacementY)-1; i>0; i--)
			{
				move(NORTH, robot);
				displacementY = robot->boxLoc[0]-robot->robotLoc[0];
			}
		}

		else
		{
			for(int i=abs(displacementY)-1; i>0; i--)
			{
				move(SOUTH, robot);
				displacementY = robot->boxLoc[0]-robot->robotLoc[0];
			}
		}
	}

	else if(displacementX > 0 && displacementY !=0)
	{
		for(int i=abs(displacementX); i>0; i--)
		{
			move(EAST, robot);
			displacementX = robot->boxLoc[1]-robot->robotLoc[1];
		}

		if(displacementY < 0)
		{
			for(int i=abs(displacementY)-1; i>0; i--)
			{
				move(NORTH, robot);
				displacementY = robot->boxLoc[0]-robot->robotLoc[0];
			}
		}

		else
		{
			for(int i=abs(displacementY)-1; i>0; i--)
			{
				move(SOUTH, robot);
				displacementY = robot->boxLoc[0]-robot->robotLoc[0];
			}
		}		
	}
	// --------------------------------------------------------------------
	// These conditionals move the robot to the y-coord +or- 1 of the box
	else if(displacementY < 0 && displacementX != 0)
	{
		for(int i=abs(displacementY); i>0; i--)
		{
			move(NORTH, robot);
			displacementY = robot->boxLoc[0]-robot->robotLoc[0];
		}

		if(displacementX < 0)
		{
			for(int i=abs(displacementX)-1; i>0; i--)
			{
				move(WEST, robot);
				displacementX = robot->boxLoc[1] - robot->robotLoc[1];
			}
		}

		else
		{
			for(int i=abs(displacementX)-1; i>0; i--)
			{
				move(EAST, robot);
				displacementX = robot->boxLoc[1] - robot->robotLoc[1];
			}
		}	
	}

	//-----------------------------------------------------------------
	// lastly handles if one of the robots spawns in the exact x or y
	// coordinate of the box on the grid
	else
	{
		if(displacementX == 0)
		{
			if(displacementY < 0)
			{
				for(int i=abs(displacementY)-1; i>0; i--)
				{
					move(NORTH, robot);
					displacementY = robot->boxLoc[0] - robot->robotLoc[0];
				}
			}
			
			else if(displacementY > 0)
			{
				for(int i=abs(displacementY)-1; i>0; i--)
				{
					move(SOUTH, robot);
					displacementY = robot->boxLoc[0] - robot->robotLoc[0];
				}
			}			
		}
		
		else
		{
			if(displacementX < 0)
			{
				for(int i=abs(displacementX)-1; i>0; i--)
				{
					move(WEST, robot);
					displacementX = robot->boxLoc[1] - robot->robotLoc[1];
				}
			}

			else if(displacementX > 0)
			{
				for(int i=abs(displacementX)-1; i>0; i--)
				{
					move(EAST, robot);
					displacementX = robot->boxLoc[1] - robot->robotLoc[1];
				}
			}
		}
	}
}

//-------------------------------------------------------
// Once getting the robot adjacent to the box the robot
// postions itself to perform the pushing. Just gets
// passed the robot struct which contains all necessary
// data.
//-------------------------------------------------------
void positionRobot(Robot* robot, Direction dir)
{	
	// Necessary variables for the algorithm
	int* boxLoc = robot->boxLoc;
	int* robotLoc = robot->robotLoc;

	// Variables for important coordinates
	int robotRow = robotLoc[0];
	int boxRow = boxLoc[0];

	int robotCol = robotLoc[1];
	int boxCol = boxLoc[1];

	if(dir == NORTH)
	{
		if(robotRow == boxRow-1 && robotCol == boxCol)
		{
			if(robot->boxIndex%2 == 0)
			{
				move(EAST, robot);
				move(SOUTH, robot);
				move(SOUTH, robot);
				move(WEST, robot);
			}

			else
			{
				move(WEST, robot);
				move(SOUTH, robot);
				move(SOUTH, robot);
				move(EAST, robot);				
			}
		}

		else if(robotCol == boxCol-1 && robotRow == boxRow)
		{
			move(SOUTH, robot);
			move(EAST, robot);
		}

		else if(robotCol == boxCol+1 && robotRow == boxRow)
		{
			move(SOUTH, robot);
			move(WEST, robot);
		}
	}

	else if(dir == SOUTH)
	{
		if(robotRow == boxRow+1 && robotCol == boxCol)
		{
			if(robot->boxIndex%2 == 0)
			{
				move(EAST, robot);
				move(NORTH, robot);
				move(NORTH, robot);
				move(WEST, robot);
			}

			else
			{
				move(WEST, robot);
				move(NORTH, robot);
				move(NORTH, robot);
				move(EAST, robot);				
			}
		}

		else if(robotCol == boxCol-1 && robotRow == boxRow)
		{
			move(NORTH, robot);
			move(EAST, robot);
		}

		else if(robotCol == boxCol+1 && robotRow == boxCol)
		{
			move(NORTH, robot);
			move(WEST, robot);
		}		
	}

	else if(dir == EAST)
	{
		if(robotCol == boxCol+1 && robotRow == boxRow)
		{
			if(robot->boxIndex%2 == 0)
			{
				move(NORTH, robot);
				move(WEST, robot);
				move(WEST, robot);
				move(SOUTH, robot);
			}

			else
			{
				move(SOUTH, robot);
				move(WEST, robot);
				move(WEST, robot);
				move(NORTH, robot);				
			}
		}

		else if(robotRow == boxRow-1 && robotCol == boxCol)
		{
			move(WEST, robot);
			move(SOUTH, robot);
		}

		else if(robotRow == boxRow+1 && robotCol == boxCol)
		{
			move(WEST, robot);
			move(NORTH, robot);
		}		
	}

	else if(dir == WEST)
	{
		if(robotCol == boxCol-1 && robotRow == boxRow)
		{
			if(robot->boxIndex%2 == 0)
			{
				move(NORTH, robot);
				move(EAST, robot);
				move(EAST, robot);
				move(SOUTH, robot);
			}

			else
			{
				move(SOUTH, robot);
				move(EAST, robot);
				move(EAST, robot);
				move(NORTH, robot);				
			}
		}

		else if(robotRow == boxRow-1 && robotCol == boxCol)
		{
			move(EAST, robot);
			move(SOUTH, robot);
		}

		else if(robotRow == boxRow+1 && robotCol == boxCol)
		{
			move(EAST, robot);
			move(NORTH, robot);
		}		
	}
}


//--------------------------------------------------------
// After the robot has been positioned, the robot begins
// pushing the box to the door in both x and y directions
//--------------------------------------------------------
void boxPathToDoor(Robot* robot)
{	
	int robotIndex = robot->boxIndex;
	Direction dir;

	bool boxAtDoor = ((robot->boxLoc[0] == doorLoc[robot->doorIndex][0]) && (robot->boxLoc[1] == doorLoc[robot->doorIndex][1]));

	while(boxAtDoor == false)
	{
		int boxRow = robot->boxLoc[0];
		int doorRow = doorLoc[doorAssign[robotIndex]][0];
		int displacementY = doorRow - boxRow;
		int boxCol = robot->boxLoc[1];
		int doorCol = doorLoc[doorAssign[robotIndex]][1];
		int displacementX = doorCol - boxCol;

		// robot must push box north
		if(displacementY < 0)
		{
			dir = NORTH;
			positionRobot(robot, dir);
			push(dir, robot);
		}

		// robot must push box south
		else if(displacementY > 0)
		{
			dir = SOUTH;
			positionRobot(robot, dir);
			push(dir, robot);
		}

		// robot must push box west
		else if(displacementX < 0)
		{
			dir = WEST;
			positionRobot(robot, dir);
			push(dir, robot);
		}

		// robot must push box east
		else if(displacementX > 0)
		{
			dir = EAST;
			positionRobot(robot, dir);
			push(dir, robot);
		}
		boxAtDoor = ((robot->boxLoc[0] == doorLoc[robot->doorIndex][0]) && (robot->boxLoc[1] == doorLoc[robot->doorIndex][1]));
	}
}

// Takes care of moving the robots
void move(Direction dir, Robot* robot)
{	
	pthread_mutex_lock(&logLock);
	int index = robot->boxIndex;

	if(dir == NORTH)
	{
		robot->robotLoc[0] -= 1;
		fprintf(fp, "Robot %d Move N\n", index);
	}
	
	else if(dir == SOUTH)
	{
		robot->robotLoc[0] += 1;
		fprintf(fp, "Robot %d Move S\n", index);
	}
	
	else if(dir == EAST)
	{
		robot->robotLoc[1] += 1;
		fprintf(fp, "Robot %d Move E\n", index);
	}
	
	else if(dir == WEST)
	{
		robot->robotLoc[1] -= 1;
		fprintf(fp, "Robot %d Move W\n", index);
	}

	pthread_mutex_unlock(&logLock);
	usleep(robotSleepTime);
}

// Takes care of pushing the boxes
void push(Direction dir, Robot* robot)
{
	pthread_mutex_lock(&logLock);
	int index = robot->boxIndex;

	if(dir == NORTH)
	{
		robot->boxLoc[0] -= 1;
		robot->robotLoc[0] -= 1;
		fprintf(fp, "Robot %d Push N\n", index);
	}
	
	if(dir == SOUTH)
	{
		robot->boxLoc[0] += 1;
		robot->robotLoc[0] += 1;
		fprintf(fp, "Robot %d Push S\n", index);
	}
	
	if(dir == EAST)
	{
		robot->boxLoc[1] += 1;
		robot->robotLoc[1] += 1;
		fprintf(fp, "Robot %d Push E\n", index);
	}
	
	if(dir == WEST)
	{
		robot->boxLoc[1] -= 1;
		robot->robotLoc[1] -= 1;
		fprintf(fp, "Robot %d Push W\n", index);
	}

	pthread_mutex_unlock(&logLock);
	usleep(robotSleepTime);
}

void displayGridPane(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glutSetWindow(gSubwindow[GRID_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, GRID_PANE_HEIGHT, 0);
	glScalef(1.f, -1.f, 1.f);


    for (int i=0; i<numBoxes; i++)
	{
		testRobotSuccess(&robots[i]);
		drawRobotAndBox(i, robots[i].robotLoc[0], robots[i].robotLoc[1], robots[i].boxLoc[0], robots[i].boxLoc[1], doorAssign[i]);
	}

	if(numLiveThreads == 0)
	{
		endSimulation();
	}

	for (int i=0; i<numDoors; i++)
	{
		//	here I would test if the robot thread is still alive
		//				row				column	
		drawDoor(i, doorLoc[i][0], doorLoc[i][1]);
	}

	//	This call does nothing important. It only draws lines
	//	There is nothing to synchronize here
	drawGrid();

	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();
	glutSetWindow(gMainWindow);
}

void displayStatePane(void)
{
	//	This is OpenGL/glut magic.  Don't touch
	glutSetWindow(gSubwindow[STATE_PANE]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	pthread_mutex_lock(&renderLock);

	drawState(numMessages, message);
	
	pthread_mutex_unlock(&renderLock);
	
	//	This is OpenGL/glut magic.  Don't touch
	glutSwapBuffers();
	glutSetWindow(gMainWindow);
}

// Provided -- Don't touch
void speedupRobots(void)
{
	//	decrease sleep time by 20%, but don't get too small
	int newSleepTime = (8 * robotSleepTime) / 10;
	
	if (newSleepTime > MIN_SLEEP_TIME)
	{
		robotSleepTime = newSleepTime;
	}
}

// Provided -- Don't touch
void slowdownRobots(void)
{
	//	increase sleep time by 20%
	robotSleepTime = (12 * robotSleepTime) / 10;
}

// ============================
// Expected Arguments:
// 1. # of Rows in Grid
// 2. # of Columns in Grid
// 3. # of Doors
// 4. # of Boxes/Robots
//==============================
int main(int argc, char** argv)
{
	// Asserts correct # of arguments
	if(argc != 5){
		fprintf(stderr, "Incorrect number of arguments!\n");
		return -1;
	}

	// Sets the global variables
	numRows = atoi(argv[1]);
	numCols = atoi(argv[2]);
	numBoxes = atoi(argv[3]);
	numDoors = atoi(argv[4]);

	// Asserts correct # of doors (1<=doors<=3)
    if(numDoors < 1 || numDoors > 3){
        fprintf(stderr, "Number of doors must be between 1 and 3!\n");
        return -1;
    }

	//=====================================================================
	// Due to the algorithm we used for generating random coordinates
	// without repetition, we cap the number of objects at 60% of the 
	// grid after running numerous simulations. Gives best performance
	// without seg faulting. Above 60%, coordinate generation gets very
	// slow.
	//=====================================================================
	int numObjects = (2*numBoxes)+numDoors;
	if(numObjects > (0.6*(numRows*numCols)))
	{
		fprintf(stderr, "Too many objects for the grid! High chance of segmentation fault.\n");
		return -1;
	}

	char buf[100];
	int countSims = 0;
	snprintf(buf, 100, "robotSimulOut %d.txt", countSims);
	while(access(buf, F_OK)!=-1)
	{
		countSims++;
		snprintf(buf, 100, "robotSimulOut %d.txt", countSims);
	}

	// Opens the output file for writing
	fp = fopen(buf, "w");

	fprintf(fp, "\n[BEGIN NEW SIMULATION]\n\n");

	// Prints the parameters for the simulation
	fprintf(fp, "Rows: %d Columns: %d Robots/Boxes: %d Doors: %d\n\n", numRows, numCols, numBoxes, numDoors);

	// Glut magic -- Don't touch
	initializeFrontEnd(argc, argv, displayGridPane, displayStatePane);
	
	//	Application-level initialization
	initializeApplication();

	// Lose control here
	glutMainLoop();
	
	//	Free allocated resource before leaving (not absolutely needed, but
	//	just nicer.  Also, if you crash there, you know something is wrong
	//	in your code.
	for (int i=0; i< numRows; i++)
		free(grid[i]);
	free(grid);
	for (int k=0; k<MAX_NUM_MESSAGES; k++)
		free(message[k]);
	free(message);

	fclose(fp);
	free(fp);

	return 0;
}

// Added dynamic memory allocation for the box, door and robot location arrays
void initializeApplication(void)
{
	//	Allocate the grid
	grid = (int**) malloc(numRows * sizeof(int*));
	for (int i=0; i<numRows; i++)
		grid[i] = (int*) malloc(numCols * sizeof(int));
	
	// Allocate memory for messages
	message = (char**) malloc(MAX_NUM_MESSAGES*sizeof(char*));
	for (int k=0; k<MAX_NUM_MESSAGES; k++)
		message[k] = (char*) malloc((MAX_LENGTH_MESSAGE+1)*sizeof(char));

	// Allocate memory for doorLoc
	doorLoc = (int**) calloc(numDoors, sizeof(int*));
	for(int i = 0; i<numDoors; i++)
		doorLoc[i] = (int*) calloc(2, sizeof(int));

	// Allocate robot to door assignment array
	doorAssign = (int*) calloc(numBoxes, sizeof(int));

	// Struct for future threads
	robots = (Robot*) malloc(numBoxes*sizeof(Robot));

    // seed the pseudo-random generator
	srand((unsigned int) time(NULL));

	// randomize location of doors
    for(int i = 0; i < numDoors; i++)
    {   
        int colVal = rand() % numCols;
        int rowVal = rand() % numRows;

		// This loop is to make sure two doors don't spawn in the same location
		for(int j=0; j<i; j++)
		{
			if(doorLoc[j][0] == rowVal && doorLoc[j][1] == colVal)
			{
				colVal = rand() % numCols;
				rowVal = rand() % numRows;
				j=0;
			}
		}

		doorLoc[i][0] = rowVal;
		doorLoc[i][1] = colVal;
    }

	// randomize location of boxes
    for(int i = 0; i < numBoxes; i++)
    {   
		robots[i].boxIndex = i;
		int colVal = rand() % (numCols-2)+1;
		int rowVal = rand() % (numRows-2)+1;

		//=======================================================================
		// Loops below are making sure objects don't spawn in the same location
		for(int j=0; j<numDoors; j++)
		{
			if(doorLoc[j][0] == rowVal && doorLoc[j][1] == colVal)
			{
				colVal = rand() % (numCols-2)+1;
				rowVal = rand() % (numRows-2)+1;
				j=0;
			}
		}

		for(int j=0; j<i; j++)
		{
			if(robots[j].boxLoc[0] == rowVal && robots[j].boxLoc[1] == colVal)
			{
				colVal = rand() % (numCols-2)+1;
				rowVal = rand() % (numRows-2)+1;
				i=0;
			}
		}
		//========================================================================

		robots[i].boxLoc[0] = rowVal;
		robots[i].boxLoc[1] = colVal;
    }

	// randomly assign boxes to doors
	for(int i = 0; i<numBoxes; i++)
	{
		robots[i].doorIndex = rand()%numDoors;
		doorAssign[i] = robots[i].doorIndex;
	}

    // randomize location of robots
    for(int i = 0; i < numBoxes; i++)
    {   
        int colVal = rand() % numCols;
        int rowVal = rand() % numRows;

		//=================================================================
		// Loops below make sure objects don't spawn in the same location
		for(int j=0; j<numDoors; j++)
		{
			if(doorLoc[j][0] == rowVal && doorLoc[j][1] == colVal)
			{
				colVal = rand() % numCols;
				rowVal = rand() % numRows;
				j=0;
			}
		}

		for(int j=0; j<numBoxes; j++)
		{
			if(robots[j].boxLoc[0] == rowVal && robots[j].boxLoc[1] == colVal)
			{
				colVal = rand() % numCols;
				rowVal = rand() % numRows;
				i=0;
			}
		}

		for(int j=0; j<i; j++)
		{
			if(robots[j].robotLoc[0] == rowVal && robots[j].robotLoc[1] == colVal)
			{
				colVal = rand() % numCols;
				rowVal = rand() % numRows;
				i=0;
			}
		}
		//=================================================================

		robots[i].robotLoc[0] = rowVal;
		robots[i].robotLoc[1] = colVal;
    }

	for(int i=0; i<numDoors; i++)
	{
		fprintf(fp, "Door %d Location: [Row: %d, Column: %d]\n", 
				i, doorLoc[i][0], doorLoc[i][1]);
	}
	fprintf(fp, "\n"); 
	
	for(int i=0; i<numBoxes; i++)
	{
		fprintf(fp, "Box %d Location: [Row: %d, Column: %d]\n", 
				i, robots[i].boxLoc[0], robots[i].boxLoc[1]);
	}
	fprintf(fp, "\n");

	for(int i=0; i<numBoxes; i++)
	{
		fprintf(fp, "Robot %d Location: [Row: %d, Column: %d] | Door Assignment: %d\n", 
				i, robots[i].robotLoc[0], robots[i].robotLoc[1], doorAssign[i]);
	}
	fprintf(fp, "\n");

	int errorCode;

	// initialize and test the state of the log file mutex
	errorCode = pthread_mutex_init(&logLock, NULL);
	if(errorCode != 0)
	{
		fprintf(stderr, "Could not initialize log file lock.\n");
		exit(EXIT_FAILURE);
	}

	// initialize and test the state of the render lock
	errorCode = pthread_mutex_init(&renderLock, NULL);
	if(errorCode != 0)
	{
		fprintf(stderr, "Could not initialize GLRender lock.\n");
		exit(EXIT_FAILURE);
	}

	// here we create the robot threads
	for(int i=0; i<numBoxes; i++)
	{
		errorCode = pthread_create(&robots[i].threadID, NULL, robotThreadFunc, robots+i);
		numLiveThreads++;
		if(errorCode != 0)
		{
			fprintf(stderr, "Could not create thread %d.\n", i);
			exit(EXIT_FAILURE);
		}

		pthread_detach(robots[i].threadID);
	}
}

void* robotThreadFunc(void *arg)
{
	Robot* robot = (Robot *) arg;

	pathToBox(robot);
	boxPathToDoor(robot);

	return NULL;
}

void testRobotSuccess(Robot* robot)
{
	// if statements below will test whether the box has reached its final destination
	int doorIndex = robot->doorIndex;
	if(robot->boxLoc[0] == doorLoc[doorIndex][0] && robot->boxLoc[1] == doorLoc[doorIndex][1])
	{
		if(robot->robotLoc[1] == (doorLoc[doorIndex][1]-1))
		{
			robot->robotLoc[1]++;
			numLiveThreads--;

			pthread_mutex_lock(&logLock);
			fprintf(fp, "Robot %d End\n", robot->boxIndex);
			pthread_mutex_unlock(&logLock);
		}

		else if(robot->robotLoc[1] == (doorLoc[doorIndex][1]+1))
		{
			robot->robotLoc[1]--;
			numLiveThreads--;

			pthread_mutex_lock(&logLock);
			fprintf(fp, "Robot %d End\n", robot->boxIndex);
			pthread_mutex_unlock(&logLock);
		}

		else if(robot->robotLoc[0] == (doorLoc[doorIndex][0]+1))
		{
			robot->robotLoc[0]--;
			numLiveThreads--;

			pthread_mutex_lock(&logLock);
			fprintf(fp, "Robot %d End\n", robot->boxIndex);
			pthread_mutex_unlock(&logLock);
		}

		else if(robot->robotLoc[0] == (doorLoc[doorIndex][0]-1))
		{
			robot->robotLoc[0]++;
			numLiveThreads--;

			pthread_mutex_lock(&logLock);
			fprintf(fp, "Robot %d End\n", robot->boxIndex);
			pthread_mutex_unlock(&logLock);
		}
	}
}

void endSimulation(void)
{
	pthread_mutex_lock(&logLock);
	fprintf(fp, "All robots completed their tasks successfully!\n\n[END SIMULATION]");
	pthread_mutex_unlock(&logLock);

	pthread_mutex_destroy(&renderLock); 
	pthread_mutex_destroy(&logLock);

	for(int j=0; j<numDoors; j++)
		free(doorLoc[j]);
	free(doorLoc);

	free(robots);

	exit(EXIT_SUCCESS);
}