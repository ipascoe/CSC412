//================================================================================================
// Title: main.c
// Authors: Jean-Yves Herve, Mike Martel & Ian Pascoe
// For Final Project due 12-20-2018
// 
// VERSION 1
// 
// This program simulates a series of robots, boxes and doors, where the robots are to push
// the boxes into its assigned door. Dimensions of grid and the number of boxes, robots and 
// doors are specified on the command line as arguments.
//================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
//
#include "gl_frontEnd.h"

// For later use with threads
typedef struct Robot
{
	int boxIndex;
	int doorIndex;
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
void positionRobot(Robot* robot);
void boxPathToDoor(Robot* robot);
void move(Direction dir, int index);
void push(Direction dir, int index);

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

// These are the global coordinate pointers for robots, boxes, and doors
int** robotLoc;
int** boxLoc;
int* doorAssign;
int** doorLoc;

// Output file pointer
FILE* fp;

Robot* robots;

//===================================================================================
// Function Implementations
//===================================================================================

// Finds the route for the robot to get behind the box in preparation for pushing
void pathToBox(Robot* robot)
{	
	int boxIndex = robot->boxIndex;

	int robotCol = robotLoc[boxIndex][1];
    int robotRow = robotLoc[boxIndex][0];
	int boxCol = boxLoc[boxIndex][1];
    int boxRow = boxLoc[boxIndex][0];
	int displacementX = boxCol - robotCol;
    int displacementY = boxRow - robotRow;

	if(displacementX < 0 && displacementY != 0)
	{
		for(int i=0; i<abs(displacementX); i++)
		{
			move(WEST, boxIndex);
		}

		if(displacementY < 0)
		{
			for(int i=0; i<abs(displacementY)-1; i++)
			{
				move(NORTH, boxIndex);
			}
		}

		else
		{
			for(int i=0; i<abs(displacementY)-1; i++)
			{
				move(SOUTH, boxIndex);
			}
		}
	}

	else if(displacementX > 0 && displacementY !=0)
	{
		for(int i=0; i<abs(displacementX); i++)
		{
			move(EAST, boxIndex);
		}

		if(displacementY < 0)
		{
			for(int i=0; i<abs(displacementY)-1; i++)
			{
				move(NORTH, boxIndex);
			}
		}

		else
		{
			for(int i=0; i<abs(displacementY)-1; i++)
			{
				move(SOUTH, boxIndex);
			}
		}		
	}

	else if(displacementY < 0 && displacementX != 0)
	{
		for(int i=0; i<abs(displacementY); i++)
		{
			move(NORTH, boxIndex);
		}

		if(displacementX < 0)
		{
			for(int i=0; i<abs(displacementX)-1; i++)
			{
				move(WEST, boxIndex);
			}
		}

		else
		{
			for(int i=0; i<abs(displacementX)-1; i++)
			{
				move(EAST, boxIndex);
			}
		}	
	}

	else
	{
		if(displacementX == 0)
		{
			if(displacementY < 0)
			{
				for(int i=0; i<abs(displacementY)-1; i++)
				{
					move(NORTH, boxIndex);
				}
			}

			else if(displacementY > 0)
			{
				for(int i=0; i<abs(displacementY)-1; i++)
				{
					move(SOUTH, boxIndex);
				}				
			}
		}
		
		else
		{
			if(displacementX < 0)
			{
				for(int i=0; i<abs(displacementX)-1; i++)
				{
					move(WEST, boxIndex);
				}
			}

			else if(displacementX > 0)
			{
				for(int i=0; i<abs(displacementX)-1; i++)
				{
					move(EAST, boxIndex);
				}
			}
		}
	}
}

// This function moves the robot around the door so that it can be pushed in the
// correct direction
void positionRobot(Robot* robot)
{	
	int boxIndex = robot->boxIndex;
	int doorIndex = robot->doorIndex;

	int robotRow = robotLoc[boxIndex][0];
	int boxRow = boxLoc[boxIndex][0];
	int doorRow = doorLoc[doorIndex][0];

	int robotCol = robotLoc[boxIndex][1];
	int boxCol = boxLoc[boxIndex][1];
	int doorCol = doorLoc[doorIndex][1];

	int displacementX = doorCol-boxCol;
	int displacementY = doorRow-boxRow;

	if(displacementY > 0)
	{
		if(robotRow == boxRow+1)
		{
			move(EAST, boxIndex);
			move(NORTH, boxIndex);
			move(NORTH, boxIndex);
			move(WEST, boxIndex);
		}

		else if(robotCol == boxCol-1)
		{
			move(NORTH, boxIndex);
			move(EAST, boxIndex);
		}

		else if(robotCol == boxCol+1)
		{
			move(NORTH, boxIndex);
			move(WEST, boxIndex);
		}
	}

	else if(displacementY < 0)
	{
		if(robotRow == boxRow-1)
		{
			move(WEST, boxIndex);
			move(SOUTH, boxIndex);
			move(SOUTH, boxIndex);
			move(EAST, boxIndex);
		}

		else if(robotCol == boxCol-1)
		{
			move(SOUTH, boxIndex);
			move(EAST, boxIndex);
		}

		else if(robotCol == boxCol+1)
		{
			move(SOUTH, boxIndex);
			move(WEST, boxIndex);
		}		
	}

	else
	{
		if(displacementX < 0)
		{
			if(robotCol == boxCol-1)
			{
				move(SOUTH, boxIndex);
				move(EAST, boxIndex);
				move(EAST, boxIndex);
				move(NORTH, boxIndex);
			}

			else if(robotRow == boxRow-1)
			{
				move(EAST, boxIndex);
				move(SOUTH, boxIndex);
			}

			else if(robotRow == boxRow+1)
			{
				move(EAST, boxIndex);
				move(NORTH, boxIndex);
			}
		}

		else
		{
			if(robotCol == boxCol+1)
			{
				move(NORTH, boxIndex);
				move(WEST, boxIndex);
				move(WEST, boxIndex);
				move(SOUTH, boxIndex);
			}

			else if(robotRow == boxRow-1)
			{
				move(WEST, boxIndex);
				move(SOUTH, boxIndex);
			}

			else if(robotRow == boxRow+1)
			{
				move(WEST, boxIndex);
				move(NORTH, boxIndex);
			}
		}
	}
}

// This function will direct the robot when pushing the
// box to the door.
void boxPathToDoor(Robot* robot)
{	
	int robotIndex = robot->boxIndex;
	int doorIndex = robot->doorIndex;
	Direction dir;
	
	positionRobot(robot);

	int boxRow = boxLoc[robotIndex][0];
	int doorRow = doorLoc[doorIndex][0];
	int displacement = doorRow - boxRow;

	// robot must push box north
	if(displacement < 0)
	{
		dir = NORTH;
		for(int i = 0; i < abs(displacement); i++)
		{
			push(dir, robotIndex);
		}
	}

	// robot must push box south
	else if(displacement >= 0)
	{
		dir = SOUTH;
		for(int i = 0; i < displacement; i++)
		{
			push(dir, robotIndex);

		}
	}

	positionRobot(robot);

	int boxCol = boxLoc[robotIndex][1];
	int doorCol = doorLoc[doorIndex][1];
	displacement = doorCol - boxCol;

	// robot must push box west
	if(displacement < 0)
	{
		dir = WEST;
		for(int i = 0; i < abs(displacement); i++)
		{	
			push(dir, robotIndex);

		}
	}

	// robot must push box east
	else if(displacement >= 0)
	{
		dir = EAST;
		for(int i = 0; i < displacement; i++)
		{
			push(dir, robotIndex);

		}
	}
}

// Takes care of moving the robots
void move(Direction dir, int index)
{	
	if(dir == NORTH)
	{
		robotLoc[index][0] -= 1;
		fprintf(fp, "Robot %d Move N\n", index);
	}
	
	else if(dir == SOUTH)
	{
		robotLoc[index][0] += 1;
		fprintf(fp, "Robot %d Move S\n", index);
	}
	
	else if(dir == EAST)
	{
		robotLoc[index][1] += 1;
		fprintf(fp, "Robot %d Move E\n", index);
	}
	
	else if(dir == WEST)
	{
		robotLoc[index][1] -= 1;
		fprintf(fp, "Robot %d Move W\n", index);
	}

	myDisplay();
	usleep(robotSleepTime);
}

// Takes care of pushing the boxes
void push(Direction dir, int index)
{
	if(dir == NORTH)
	{
		boxLoc[index][0] -= 1;
		robotLoc[index][0] -= 1;
		fprintf(fp, "Robot %d Push N\n", index);
	}
	
	if(dir == SOUTH)
	{
		boxLoc[index][0] += 1;
		robotLoc[index][0] += 1;
		fprintf(fp, "Robot %d Push S\n", index);
	}
	
	if(dir == EAST)
	{
		boxLoc[index][1] += 1;
		robotLoc[index][1] += 1;
		fprintf(fp, "Robot %d Push E\n", index);
	}
	
	if(dir == WEST)
	{
		boxLoc[index][1] -= 1;
		robotLoc[index][1] -= 1;
		fprintf(fp, "Robot %d Push W\n", index);
	}

	myDisplay();
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
		// if statements below will test whether the box has reached its final destination
		// if all boxes are in desired location, we decided to kill the GLUT window 
		if(robotLoc[i][1] == (doorLoc[doorAssign[i]][1]-1))
		{
			if(boxLoc[i][0] == doorLoc[doorAssign[i]][0] && boxLoc[i][1] == doorLoc[doorAssign[i]][1])
			{
				robotLoc[i][1]++;
				numLiveThreads--;
				fprintf(fp, "Robot %d End\n", robots[i].boxIndex);
			}
		}
		
		if(robotLoc[i][1] == (doorLoc[doorAssign[i]][1]+1))
		{
			if(boxLoc[i][0] == doorLoc[doorAssign[i]][0] && boxLoc[i][1] == doorLoc[doorAssign[i]][1])
			{
				robotLoc[i][1]--;
				numLiveThreads--;
				fprintf(fp, "Robot %d End\n", robots[i].boxIndex);
			}
		}

		drawRobotAndBox(i, robotLoc[i][0], robotLoc[i][1], boxLoc[i][0], boxLoc[i][1], doorAssign[i]);
	}

	if(numLiveThreads == 0)
	{
		fprintf(fp, "All robots completed their tasks successfully!\n\n[END SIMULATION]");
		for(int j=0; j<numBoxes; j++)
		{
			free(robotLoc[j]);
			free(boxLoc[j]);
		}

		for(int j=0; j<numDoors; j++)
		{
			free(doorLoc[j]);
		}

		free(robotLoc);
		free(boxLoc);
		free(robots);
		free(doorAssign);
		free(doorLoc);
		exit(EXIT_SUCCESS);
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

	drawState(numMessages, message);
	
	
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

	numLiveThreads = numBoxes;

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

	//=====================================================
	// Allocate the coordinate arrays
    robotLoc = (int**) calloc(numBoxes, sizeof(int*));
	for(int i = 0; i<numBoxes; i++)
	{
		robotLoc[i] = (int*) calloc(2, sizeof(int));
	}

	boxLoc = (int**) calloc(numBoxes, sizeof(int*));
	for(int i = 0; i<numBoxes; i++)
	{
		boxLoc[i] = (int*) calloc(2, sizeof(int));
	}

	doorLoc = (int**) calloc(numDoors, sizeof(int*));
	for(int i = 0; i<numDoors; i++)
	{
		doorLoc[i] = (int*) calloc(2, sizeof(int));
	}
	//======================================================

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
			if(boxLoc[j][0] == rowVal && boxLoc[j][1] == colVal)
			{
				colVal = rand() % (numCols-2)+1;
				rowVal = rand() % (numRows-2)+1;
				i=0;
			}
		}
		//========================================================================

		boxLoc[i][0] = rowVal;
		boxLoc[i][1] = colVal;
    }

	// randomly assign boxes to doors
	for(int i = 0; i<numBoxes; i++)
	{
		doorAssign[i] = rand()%numDoors;
		robots[i].doorIndex = doorAssign[i];				
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
			if(boxLoc[j][0] == rowVal && boxLoc[j][1] == colVal)
			{
				colVal = rand() % numCols;
				rowVal = rand() % numRows;
				i=0;
			}
		}
		for(int j=0; j<i; j++)
		{
			if(robotLoc[j][0] == rowVal && robotLoc[j][1] == colVal)
			{
				colVal = rand() % numCols;
				rowVal = rand() % numRows;
				i=0;
			}
		}
		//=================================================================

		robotLoc[i][0] = rowVal;
		robotLoc[i][1] = colVal;
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
				i, boxLoc[i][0], boxLoc[i][1]);
	}
	fprintf(fp, "\n");

	for(int i=0; i<numBoxes; i++)
	{
		fprintf(fp, "Robot %d Location: [Row: %d, Column: %d] | Door Assignment: %d\n", 
				i, robotLoc[i][0], robotLoc[i][1], doorAssign[i]);
	}
	fprintf(fp, "\n");



	// This call brings us to the movement of the robots which will be a chained series
	// of function calls starting with pathToBox
	for(int i = 0; i < numBoxes; i++)
	{
		pathToBox(&robots[i]);
		boxPathToDoor(&robots[i]);
	}
}