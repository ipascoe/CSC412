//
//  gl_frontEnd.c
//  GL threads
//
//  Created by Jean-Yves Hervé on 2017-04-24.
//  Copyright © 2017 Jean-Yves Hervé. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//
#include "gl_frontEnd.h"


//---------------------------------------------------------------------------
//  Private functions' prototypes
//---------------------------------------------------------------------------
void myTimer(int val);

//---------------------------------------------------------------------------
//  Global variables
//---------------------------------------------------------------------------
extern GLint gWindowWidth, gWindowHeight;



void myTimer(int value)
{
	//	value not used.  Warning suppression
	(void) value;
	
    drawCurve();
	
	//	And finally I perform the rendering
	glutTimerFunc(100, myTimer, 0);

    glutPostRedisplay();
}


void initializeFrontEnd(int argc, char** argv)
{
	//	Initialize glut and create a new window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);


	glutInitWindowSize(gWindowWidth, gWindowHeight);
	glutCreateWindow("Programming Assignment 06 - L-systems and Threads");
	glutDisplayFunc(drawCurve);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
		glOrtho(0.0f, (GLfloat) gWindowWidth, 0.0f, (GLfloat) gWindowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
	glutTimerFunc(100, myTimer, 0);
	glutKeyboardFunc(keyboardHandler);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
}
