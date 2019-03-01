/*-----------------------------------------------------------------------------------------------------------------
Title: split.c
Author: Ian Pascoe
For Prog05 due 11/21/18

This program takes an image as input and outputs three different images with only one of the color channels
visible. Arguments include one for the input path to an image, output path to a directory to place this
image after splitting the color channels, and the number of threads that are to be used to do the computation.

Compilation Command:
    gcc split.c imageIO_TGA.c rasterImage.c -lpthread -o split

Call to program on command line:
    ./split Data/clown.tga Output 5
--------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include <pthread.h> // for POSIX Threads
#include <unistd.h>
#include <math.h>

#include "imageIO_TGA.h"


//  struct for the thread data needed
typedef struct tInfo {
    pthread_t tID;
    int index;
    int startRow;
    int endRow;
    ImageStruct* img1;
    ImageStruct* img2;
    ImageStruct* img3;
}tInfo;

char* createOutput(char* in, char* out);
void isoGreenChannel(ImageStruct* image, int start, int end);
void isoRedChannel(ImageStruct* image, int start, int end);
void isoBlueChannel(ImageStruct* image, int start, int end);
void* threadFunc(void* arg) {
    tInfo* info = (tInfo *) arg;
    isoRedChannel(info->img1, info->startRow, info->endRow);
    isoGreenChannel(info->img2, info->startRow, info->endRow);
    isoBlueChannel(info->img3, info->startRow, info->endRow);
    return(NULL);
}

//--------------------------------------------------------------------------------------------
//  Main handles the command line arguments (input path, output path, number of threads)
//  and hands of the work to other functions once done handling that data.
//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[]){
  assert(argc==4); //will abort if the proper amount of arguments are not provided
    
    //  Read essential variable from command line
    char* inputPath = argv[1];
    char* outputPath = argv[2];
    int numThreads = atoi(argv[3]);

    //  Initialize images for each color channel
    ImageStruct red=readTGA(inputPath);
    ImageStruct green=readTGA(inputPath);
    ImageStruct blue=readTGA(inputPath);

    //  Find the amount of rows to be worked on by each thread
    int rowsPerThread= red.height/numThreads;

    //  Array of thread info structs
    tInfo threads[numThreads];
	int errCode;
	for(int i=0; i<numThreads; i++) {
		threads[i].index = i;
		threads[i].startRow = i*rowsPerThread;
        if((i+1) == numThreads) {
            threads[i].endRow = red.height;
        } else {
            threads[i].endRow = (i+1)*rowsPerThread;
        }
		threads[i].img1 = &red;
        threads[i].img2 = &green;
        threads[i].img3 = &blue;
		printf("Creating Thread %d\n", i);
		errCode = pthread_create (&threads[i].tID, NULL, threadFunc, threads + i);
		
		if (errCode != 0) {
            fprintf (stderr, 
                     "Could not pthread_create thread %d. %d/%s\n",
                     i, errCode, strerror(errCode));
            exit (EXIT_FAILURE);
        } usleep(10000);
	}

	//	Now renjoin the threads
    for (int i = 0; i<numThreads; i++) {
        void* retVal;
        errCode = pthread_join (threads[i].tID, &retVal);
        if (errCode != 0) {
            fprintf (stderr, "Error joining thread %d. %d/%s\n", i, errCode, strerror(errCode));
        } else {
			printf ("Joined with Thread %d\n", i);
		}
    }

    //--------------------------------------------------------------------
    //  This portion of code just keeps the output file path
    //  string clean. I was struggling with a segentation fault here
    //  because of strcat, this solved it.
    //--------------------------------------------------------------------
    
    //  Handles red output
    char* fname = createOutput(inputPath, outputPath);
    char* redf = (char*) calloc(strlen(fname)+6,sizeof(char));
    redf = strcat(fname,"_r.tga\0");
    writeTGA(redf, &red);

    //  Handles green output
    fname[strlen(fname)-6]='\0';
    char* greenf = (char*) calloc(strlen(fname)+6,sizeof(char));
    greenf = strcat(fname, "_g.tga\0");
    writeTGA(greenf, &green);

    //  Handles blue output
    fname[strlen(fname)-6]='\0';
    char* bluef = (char*) calloc(strlen(fname)+6,sizeof(char));
    bluef = strcat(fname,"_b.tga\0");
    writeTGA(bluef, &blue);
    fname[strlen(fname)-6]='\0';
    
    return 0;
}

//-----------------------------------------------------------
//  Same output creation function as the other programs.
//  Implemented by me instead of Herve, however.
//-----------------------------------------------------------
char* createOutput(char* in, char* out){
    if(out[strlen(out)-1] != '/') {
        strcat(out, "/");
    }
    char* filename = basename(in);
    char* newfilename = (char*) calloc(strlen(filename)-4,sizeof(char));
    for(int i=0;i<strlen(filename)-4;i++){
        newfilename[i]=filename[i];
    } newfilename = strcat(out,newfilename);
    return newfilename;
}



//------------------------------------------------------------------------------------------------
//  Each of the following takes an image and kills 2 of the 3 color channels leaving 1 isolated.
//  Implemented similar to the killGreenChannel funciton in main.c example of Prog02. These
//  differ from prog02 by the parameters. These have a start row and end row for the threads
//  work on.
//------------------------------------------------------------------------------------------------

void isoGreenChannel(ImageStruct* image, int start, int end) {
    int* raster = (int*)(image->raster);
    for (unsigned int i=start; i<=end && i<=image->height; i++) {
        for (unsigned int j=0; j<image->width; j++) {
            unsigned char* rgba = (unsigned char*) (raster + i*image->width + j);
            rgba[0] = 0x00;
            rgba[2] = 0x00;
        }
    }
}

void isoRedChannel(ImageStruct* image, int start, int end) {
    int* raster = (int*)(image->raster);
    for (unsigned int i=start; i<=end && i<=image->height; i++) {
        for (unsigned int j=0; j<image->width; j++) {
            unsigned char* rgba = (unsigned char*) (raster + i*image->width + j);
            rgba[1] = 0x00;
            rgba[2] = 0x00;
        }
    }
}

void isoBlueChannel(ImageStruct* image, int start, int end) {
    int* raster = (int*)(image->raster);
    for (unsigned int i=start; i<=end && i<=image->height; i++) {
        for (unsigned int j=0; j<image->width; j++) {
            unsigned char* rgba = (unsigned char*) (raster + i*image->width + j);
            rgba[0] = 0x00;
            rgba[1] = 0x00;
        }
    }
}
