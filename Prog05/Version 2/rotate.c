/*-----------------------------------------------------------------------------------------------------------------
Title: rotate.c
Created by Jean-Yves Hervé on 2018-10-03.
Modified by Ian Pascoe
For Prog05 due 11/21/18

This program rotates an image by multiples of 90 degrees. The degree of rotation is supplied as
an argument on the command line. The path to the image file and path to output directory is also
supplied as an argument.

What has been adapted to JYH's code is the ability to use POSIX threads to split up the workload.

Compilation command:
	gcc rotate.c imageIO_TGA.c rasterImage.c -lpthread -o rotate

Call to program on command line:
	./rotate

VERSION 2: ORIGINAL ARGUMENTS WILL NOW BE PROVIDED ON STDIN FOR THE USE OF PIPES
-----------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <pthread.h> //	For POSIX threads

#include "imageIO_TGA.h"

#if 0
#pragma mark -
#pragma mark Custom data types and global variables
#endif

/**	Simple enum type to report optional arguments for the program
 */
typedef enum RotationVal
{
	NO_ROTATION = 0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270,
	//
	NUM_ROTATIONS	//	automatically 4
	
}RotationVal;

typedef struct tInfo {
	pthread_t tID;
	int index;
	int startRow;
	int endRow;
	ImageStruct* inImage;
	ImageStruct* outImage;
	RotationVal rot;
}tInfo;

/**	An array to store the suffix strings for the output file, for each
 *	rotation value.
 */
const char* ROT_SUFFIX[NUM_ROTATIONS] = {
	"",
	" [r]",
	" [rr]",
	" [l]"
};

/**	An enum type for all the errors that this program specifically handles
 */
typedef enum ErrorCode
{
	NO_ERROR = 0,
	//
//	//	file-related	-->	Not used in this application
//	FILE_NOT_FOUND,
//	CANNOT_OPEN_FILE,
//	WRONG_FILE_TYPE,
//	CANNOT_WRITE_FILE,
	//
	//	command line argument errors
	WRONG_NUMBER_OF_ARGUMENTS,
	NO_DASH_ON_ROT_STRING,
	INVALID_ROT_STRING,
	//
	NUM_ERROR_CODES		//	correct value because I don't skip codes
	
} ErrorCode;

/**	Going overly cute here:  Error message for each of the errors
 *	supported.  This one is tricky because it's easy to get the order
 *	different from that of the enum type
 */
const char* ERROR_STR[NUM_ERROR_CODES] = {
	"",		//	NO_ERROR
//	//	file-related errors
//	"",	//	FILE_NOT_FOUND,
//	"",	//	CANNOT_OPEN_FILE,
//	"",	//	WRONG_FILE_TYPE,
//	"",	//	CANNOT_WRITE_FILE,
	//	command line argument errors
	"Incorrect number of arguments.\nProper usage: rotate -{r|l}+ inputImagePath outFolderPath\n",	//	WRONG_NUMBER_OF_ARGUMENTS
	"Rotation specification must start with a dash",	//	NO_DASH_ON_ROT_STRING,
	"Rotation specification can only contain letters l or r"	//	INVALID_ROT_STRING,
};

#if 0
#pragma mark -
#pragma mark Function prototypes
#endif

/**	Processes the rotation-specifying string to determine the
 *	rotation to apply.
 *	@param	rotStr	the rotation-specifying string in the form -[r|l]+
 *	@param	rotVal	pointer to a RotationVal enum variable
 *	@return	an error code
 */
ErrorCode determineRotation(const char* rotStr, RotationVal* rotVal);

/**	In this app, just prints out an error message to the console and
 *	exits with the proper error code.  In a fancier version, could
 *	write to a log and "swallow" some non-critical errors.
 *
 *	@param code		the code of the error to report/process
 *	@param input	the input string that caused the error (NULL otherwise)
 */
void errorReport(ErrorCode code, const char* input);

/**	Produces a complete path to the output image file.
 *	If the input file path was ../../Images/clown and the 90-degree-rotated is to be
 *	written the output folder path is ../Output [with or without final slash),
 *	then the output file path will be ../Output/clown [r].tga
 *
 *	@param inputImagePath	path to the input image
 *	@param outFolderPath	path to the output folder
 *	@param rotVal			the rotation applied
 *	@return	complete path to the desired output file.
 */
char* produceOutFilePath(char* inputImagePath, char* outFolderPath,
						 RotationVal rotVal);


/**	Produces a copy of the input image (rotation by 0).
 *	This function currently only works for RGBA32_RASTER images.
 *	@param  image	pointer to the RGBA32_RASTER image to copy
 *	@return	a new image struct that stores a copy of the input image
 */
void copyImage(const ImageStruct* inImage, ImageStruct* outImage);

/**	Produces a rotated copy of the input image (rotated by 90 degree clockwise).
 *	This function currently only works for RGBA32_RASTER images.
 *	@param	image	pointer to the RGBA32_RASTER image to rotate
 *	@return	a new image struct that stores the rotated image
 */
void rotateImage90(const ImageStruct* inImage, ImageStruct* outImage, int startRow, int endRow);

/**	Produces a rotated copy of the input image (rotated by 180 degree clockwise).
 *	This function currently only works for RGBA32_RASTER images.
 *	@param	image	pointer to the RGBA32_RASTER image to rotate
 *	@return	a new image struct that stores the rotated image
 */
void rotateImage180(const ImageStruct* inImage, ImageStruct* outImage, int startRow, int endRow);

/**	Produces a rotated copy of the input image (rotated by 270 degree clockwise).
 *	This function currently only works for RGBA32_RASTER images.
 *	@param	image	pointer to the RGBA32_RASTER image to rotate
 *	@return	a new image struct that stores the rotated image
 */
void rotateImage270(const ImageStruct* inImage, ImageStruct* outImage, int startRow, int endRow);

void* threadFunc(void* args);


#if 0
#pragma mark -
#pragma mark Function implementations
#endif

//---------------------------------------------------------------------
//	Differs from version 1 because it takes the original arguments
//	standard input instead of the command line for the use of pipes.
//---------------------------------------------------------------------
int main()
{
	while(1) {
		//	Just to look prettier in the code, I give meaningful names to my arguments
		char inputImagePath[100];
		char outFolderPath[100];
		unsigned int numThreads;

		//	Gather the rotation argument
		RotationVal rot;
		char rotation[50];

		fscanf(stdin, "%s %s %s %u", rotation, inputImagePath, outFolderPath, &numThreads);

		//	Interpret rotation
		int err = determineRotation(rotation, &rot);
		if (err)
			errorReport(err, rotation);

		//	Read the image
		ImageStruct inImage = readTGA(inputImagePath);
		ImageStruct outImage;
		switch(rot)
		{
			case NO_ROTATION:
			{
				outImage = newImage(inImage.width, inImage.height, RGBA32_RASTER, 1);
			}
			break;
		
			case ROTATE_90:
			{
				outImage = newImage(inImage.height, inImage.width, RGBA32_RASTER, 1);
			}
			break;
		
			case ROTATE_180:
			{
				outImage = newImage(inImage.width, inImage.height, RGBA32_RASTER, 1);
			}
			break;
		
			case ROTATE_270:
			{
				outImage = newImage(inImage.height, inImage.width, RGBA32_RASTER, 1);
			}
			break;
		
			//	do shut warnings up
			default:
				break;
		}
	
		//	Produce the path to the output image
		char* outImagePath = produceOutFilePath(inputImagePath, outFolderPath, rot);

		//	New portion of code for creating and joining threads
		int rowsPerThread= inImage.height/numThreads;
	    tInfo threads[numThreads];
		int errCode;
		for(int i=0; i<numThreads; i++) {
			threads[i].index = i;
			threads[i].startRow = i*rowsPerThread;
        	if((i+1) == numThreads) {
            	threads[i].endRow = inImage.height;
        	} else {
            	threads[i].endRow = (i+1)*rowsPerThread;
        	}
			threads[i].inImage = &inImage;
       		threads[i].outImage = &outImage;
			threads[i].rot = rot;
			printf("Creating Thread %d\n", i);
			errCode = pthread_create (&threads[i].tID, NULL, threadFunc, threads + i);
		
			if (errCode != 0) {
            	fprintf (stderr, 
                	     "Could not pthread_create thread %d. %d/%s\n",
                    	 i, errCode, strerror(errCode));
            	exit (EXIT_FAILURE);
        	}
		}

		//	Now renjoin the threads
    	for (int i = 0; i<numThreads; i++) {
       		void* retVal;
        	errCode = pthread_join (threads[i].tID, &retVal);
        	if (errCode != 0) {
            	fprintf (stderr, "Error joining Thread %d. %d/%s\n", i, errCode, strerror(errCode));
        	} else {
				printf ("Joined with Thread %d\n", i);
			}
    	}
	
		writeTGA(outImagePath, &outImage);

		//	Cleanup and terminate
		free(inImage.raster);
		free(outImage.raster);
		free(outImagePath);
	}
	return 0;
}

ErrorCode determineRotation(const char* rotStr, RotationVal* rotVal)
{
	//	string should start with a dash
	if (rotStr[0] != '-')
		return NO_DASH_ON_ROT_STRING;
	
	//	string should have characters besides the dash
	if (strlen(rotStr) < 2)
		return INVALID_ROT_STRING;
	
	*rotVal = NO_ROTATION;
	//	Iterate through the string, keep the count of poisitive and
	//	negative rotations
	for (unsigned int k=1; k<strlen(rotStr); k++)
	{
		switch (rotStr[k])
		{
			case 'r':
			case 'R':
			*rotVal = (*rotVal + 1) % NUM_ROTATIONS;
			break;
			
			case 'l':
			case 'L':
			*rotVal = (*rotVal + NUM_ROTATIONS - 1) % NUM_ROTATIONS;
			break;
			
			//	anything else is an error
			default:
				return INVALID_ROT_STRING;
				
		}
	}
	
	return NO_ERROR;
}


void errorReport(ErrorCode code, const char* input)
{
	if (input != NULL)
		printf("%s: %s\n", ERROR_STR[code], input);
	else
		printf("%s\n", ERROR_STR[code]);
	exit(code);
}

//---------------------------------------------------------------------------------------
//	There was a slight error in JYH's version of this function where it would
//	seg fault if the name of the input image was more than 5 letters. Fixed this using
//	libgen.h basename() function to allow it to dynamically allocate space for the
//	original filename.
//---------------------------------------------------------------------------------------
char* produceOutFilePath(char* inputImagePath, char* outFolderPath,
						 RotationVal rotVal)
{
	// Produce the name of the output file
	//-------------------------------------
	//	First, find the start of the input file's name.  Start from the end
	//	and move left until we hit the first slash or the left end of the string.
	unsigned long k = strlen(inputImagePath) - 5;
	while ((k>=1) && (inputImagePath[k-1] != '/'))
		k--;
	
	//	*EDITED TO WORK FOR ALL INPUT FILENAME LENGTHS*
	char* outFilePath = (char*) malloc(strlen(outFolderPath) +
									   strlen(ROT_SUFFIX[rotVal]) + strlen(basename(inputImagePath)));
	strcpy(outFilePath, outFolderPath);
	//	If outFolderPath didn't end with a slash, add it
	if (outFolderPath[strlen(outFolderPath)-1] != '/')
		strcat(outFilePath, "/");

	//	Produce the name of the input file minus extension
	char* inputFileRootName = (char*) malloc(strlen(inputImagePath+k) +1);
	strcpy(inputFileRootName, inputImagePath+k);
	//	chop off the extension by replacing the dot by '\0'
	inputFileRootName[strlen(inputFileRootName)-4] = '\0';

	//	Append root name to output path, add the suffix and the file extension
	strcat(outFilePath, inputFileRootName);
	strcat(outFilePath, ROT_SUFFIX[rotVal]);
	strcat(outFilePath, ".tga");

	//	free heap-allocated data we don't need anymore
	free(inputFileRootName);
	
	return outFilePath;
}


void copyImage(const ImageStruct* inImage, ImageStruct* outImage)
{
	memcpy( (char*) outImage->raster,
			(char*) inImage->raster,
			inImage->height * inImage->bytesPerRow);
}

//	In a rotation by 90 degree clockwise, the pixel at row i, col j in the input image
//	ends up at row outHeight - j - 1, col i in the output image
void rotateImage90(const ImageStruct* inImage, ImageStruct* outImage, int startRow, int endRow)
{
	const int* rasterIn = (int*)(inImage->raster);
	int* rasterOut = (int*)(outImage->raster);

	for (unsigned i=startRow; i<endRow; i++)
	{
		for (unsigned j=0; j<inImage->width; j++)
			rasterOut[(outImage->height-j-1)*outImage->width + i] =
			rasterIn[i*inImage->width + j];
	}
}

//	In a rotation by 180 degree clockwise, the pixel at row i, col j in the input image
//	ends up at row height - i - 1, col weight - j in the output image
void rotateImage180(const ImageStruct* inImage, ImageStruct* outImage, int startRow, int endRow)
{
	const int* rasterIn = (int*)(inImage->raster);
	int* rasterOut = (int*)(outImage->raster);

	for (unsigned i=startRow; i<endRow; i++)
	{
		for (unsigned j=0; j<inImage->width; j++)
			rasterOut[(outImage->height-i-1)*outImage->width + outImage->width - j -1] =
			rasterIn[i*inImage->width + j];
	}
}

//	In a rotation by 270 degree clockwise, the pixel at row i, col j in the input image
//	ends up at row j, col outWidth - i - 1 in the output image
void rotateImage270(const ImageStruct* inImage, ImageStruct* outImage, int startRow, int endRow)
{
	const int* rasterIn = (int*)(inImage->raster);
	int* rasterOut = (int*)(outImage->raster);

	for (unsigned i=startRow; i<endRow; i++)
	{
		for (unsigned j=0; j<inImage->width; j++)
			rasterOut[j*outImage->width + outImage->width - i - 1] =
			rasterIn[i*inImage->width + j];
	}
}

void* threadFunc(void* args) {

	//	Pass the data from the thread info struct to the void* argument
	tInfo* info = (tInfo *) args;

	switch(info->rot)
	{
		case NO_ROTATION:
		{
			copyImage(info->inImage, info->outImage);
		}
		break;
		
		case ROTATE_90:
		{
			rotateImage90(info->inImage, info->outImage, info->startRow, info->endRow);
		}
		break;
		
		case ROTATE_180:
		{
			rotateImage180(info->inImage, info->outImage, info->startRow, info->endRow);
		}
		break;
		
		case ROTATE_270:
		{
			rotateImage270(info->inImage, info->outImage, info->startRow, info->endRow);
		}
		break;
		
		//	do shut warnings up
		default:
			break;
	}

	return(NULL);
}