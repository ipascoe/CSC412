/*--------------------------------------------------------------------------------------------------------------------------------
Title: crop.c
Created by Jean-Yves Hervé on 2018-10-03.
Modified by Ian Pascoe
For Prog05 Due 11/21/18

This program crops a .tga image from an (x,y) coordinate at the top-left corner of the cropped selection
to a distance horizontally rightward and vertically downward from that coordinate. These specifications
are provided as arguments. Input file path and output directory path are also provided arguments. The
last argument is the amount of POSIX threads to create to do the processing.

What has been added to JYH's program is the multithreading ability. This is done using POSIX threads
to work on specific portions of pixels in the image.

Compilation Command:
	gcc crop.c imageIo_TGA.c raster.c -lpthread -o crop

Example program call on command line:
	./crop ../Images/clown.tga ../Output 0 0 156 156 4
		-This would crop an image from the top left corner to the coordinate (156,156) on the x,y pixel array
 		using 4 threads.
----------------------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h> // For use of threads
#include <unistd.h>
#include <libgen.h>

#include "imageIO_TGA.h"

#if 0
#pragma mark -
#pragma mark Custom data types and global variables
#endif

// An enum type for all the errors that this program specifically handles
typedef enum ErrorCode
{
	NO_ERROR = 0,
	WRONG_NUMBER_OF_ARGUMENTS = 20,
	INVALID_CROP_X_TYPE,
	INVALID_CROP_Y_TYPE,
	INVALID_CROP_CORNER,
	INVALID_CROP_WIDTH_TYPE,
	INVALID_CROP_HEIGHT_TYPE,
	INVALID_CROP_SIZE
} ErrorCode;

//	Struct for the data used in each thread
typedef struct tInfo 
{
	pthread_t tID;
	int index;
	int startRow;
	int endRow;
	int cropCornerX;
	int cropCornerY;
	int cropWidth;
	int cropHeight;
	ImageStruct* img1;
	ImageStruct* img2;
}tInfo;

#if 0
#pragma mark -
#pragma mark Function prototypes
#endif

//	Declaration of the new thread function needed for POSIX threads
void* threadFunc(void* arg);

/** interprets the program's input argument to determine the crop region.
 *	@param	argv	list of input argument to the program
 *	@param	cropCornerX		x coordinate of crop region's corner
 *	@param	cropCornerY		y coordinate of crop region's corner
 *	@param	cropWidth		crop region's width
 *	@param	cropHeight		crop region's height
 *	@return		an error code (0 if no error)
 */
int extractCropRegion(char* argv[],
					  unsigned int imageWidth, unsigned int imageHeight,
					  unsigned int* cropCornerX, unsigned int* cropCornerY,
					  unsigned int* cropWidth, unsigned int* cropHeight);

/**	Produces a new image that is a cropped part of the input image
 *	@param	image			the image to crop
 *	@param 	cropCornerX		x coordinate of the upper-left corner of the crop region
 *	@param 	cropCornerY		y coordinate of the upper-left corner of the crop region
 *							(counted from the top, so height - row index - 1
 *	@param 	cropWidth		width of the crop region
 *	@param 	cropHeight		height of the top region
 *	@return		the new image resulting from applying the crop
 */
void cropImage(ImageStruct *imageIn, ImageStruct *imageOut,
					  unsigned int cropCornerX, unsigned int cropCornerY,
					  unsigned int cropWidth, unsigned int cropHeight, int startRow, int endRow);

/**	In this app, just prints out an error message to the console and
 *	exits with the proper error code.  In a fancier version, could
 *	write to a log and "swallow" some non-critical errors.
 *
 *	@param code		the code of the error to report/process
 *	@param input	the input string that caused the error (NULL otherwise)
 */
void errorReport(ErrorCode code, const char* input);


/**	Produces a complete path to the output image file.
 *	If the input file path was ../../Images/cells.tga and the
 *	and the output folder path is ../Output [with or without final slash),
 *	then the output file path will be ../Output/cells [cropped].tga
 *
 *	@param inputImagePath	path to the input image
 *	@param outFolderPath	path to the output folder
 *	@return	complete path to the desired output file.
 */
char* produceOutFilePath(char* inputImagePath, char* outFolderPath);

#if 0
#pragma mark -
#pragma mark Function implementations
#endif

//--------------------------------------------------------------
//	Main function, expecting as arguments:
//		inputImagePath outFolderPath x y width height threadCount
//	It returns an error code (0 for no error)
//--------------------------------------------------------------
int main(int argc, char* argv[])
{
	//	we need at least one argument
	if (argc != 8)
	{
		printf("Proper usage: crop inputImagePath outFolderPath x y width height numThreads\n");
		return WRONG_NUMBER_OF_ARGUMENTS;
	}
	
	//	Just to look prettier in the code, I give meaningful names to my arguments
	char* inputImagePath = argv[1];
	char* outFolderPath = argv[2];

	//	Read the image
	ImageStruct imageIn = readTGA(inputImagePath);

	
	//	parameters of the crop region
	unsigned int cropCornerX, cropCornerY, cropWidth, cropHeight;

	//	extract number of threads to create from argument list
	int numThreads = atoi(argv[7]);

	//	extract parameters of crop region for testing
	extractCropRegion(argv, imageIn.width, imageIn.height,
					  &cropCornerX, &cropCornerY, &cropWidth, &cropHeight);

	ImageStruct imageOut = newImage(cropWidth, cropHeight, RGBA32_RASTER, 1);
	
	//	Compute the size of an average group of pixels that the threads will work on
	int pixelGroup = imageOut.height/numThreads;

	//-----------------------------------------------------------------------
	//	This portion of code creates and tests the threads that are to be
	//	created for the main objective of this version of the assignment.
	//-----------------------------------------------------------------------
	tInfo threads[numThreads];
	int errCode;
	for(int i=0; i<numThreads; i++) {
		threads[i].index = i;

		//	Compute first row for thread to work on
		threads[i].startRow = i*pixelGroup;

		//---------------------------------------------------------
		//	Compute the last row for thread to work on.
		//	If it is the last row, just set endRow to the last 
		//	row of the image.
		//---------------------------------------------------------
		if((i+1) == numThreads) {
			threads[i].endRow = imageOut.height;
		} else {
			threads[i].endRow = (i+1)*pixelGroup;
		}
		threads[i].cropCornerX = cropCornerX;
		threads[i].cropCornerY = cropCornerY;
		threads[i].cropWidth = cropWidth;
		threads[i].cropHeight = cropHeight;
		threads[i].img1 = &imageIn;
		threads[i].img2 = &imageOut;
		printf("Creating Thread %d\n", i);
		errCode = pthread_create (&threads[i].tID, NULL, threadFunc, threads + i);
		
		if (errCode != 0) {
            fprintf (stderr, 
                     "could not pthread_create thread %d. %d/%s\n",
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

	// Produce the apth to the output file
	char* outFilePath = produceOutFilePath(inputImagePath, outFolderPath);

	//	Write out the cropped image
	int err = writeTGA(outFilePath, &imageOut);
	
	//	Cleanup allocations.
	free(imageIn.raster);
	free(imageOut.raster);
	free(outFilePath);
	
	return err;
}

int extractCropRegion(char* argv[],
					  unsigned int imageWidth, unsigned int imageHeight,
					  unsigned int* cropCornerX, unsigned int* cropCornerY,
					  unsigned int* cropWidth, unsigned int* cropHeight)
{
	if (sscanf(argv[3], "%u", cropCornerX) != 1)
		errorReport(INVALID_CROP_X_TYPE, argv[3]);

	if (sscanf(argv[4], "%u", cropCornerY) != 1)
		errorReport(INVALID_CROP_Y_TYPE, argv[3]);

	//	Note: since we read into an unsigned int, a negative value would come out
	//	as a large positive value
	if ((*cropCornerX >= imageWidth) || (*cropCornerY >= imageHeight))
		errorReport(INVALID_CROP_CORNER, NULL);

	if (sscanf(argv[5], "%u", cropWidth) != 1)
		errorReport(INVALID_CROP_WIDTH_TYPE, argv[3]);

	if (sscanf(argv[6], "%u", cropHeight) != 1)
		errorReport(INVALID_CROP_HEIGHT_TYPE, argv[3]);

	//	Note: since we read into an unsigned int, a negative value would come out
	//	as a large positive value
	if ((*cropCornerX + *cropWidth >= imageWidth) ||
		(*cropCornerY + *cropHeight >= imageHeight))
		errorReport(INVALID_CROP_SIZE, NULL);
	
	//	Otherwise, all is ok, go back to crop
	return 0;
}

/*----------------------------------------------------------------------------------------------
I have modified crop so that it accepts the additional parameters startRow and endRow.
These integers are computed in main and tell the active thread which rows of the image to
work on in the crop function.
-----------------------------------------------------------------------------------------------*/
void cropImage(ImageStruct *imageIn,
				ImageStruct *imageOut,
				unsigned int cropCornerX, 
				unsigned int cropCornerY,
				unsigned int cropWidth, 
				unsigned int cropHeight,
				int startRow,
				int endRow)
{
	//	Beware that the images are stored upside-down from the way we view them,
	//	So I need to invert the row indices.
	for (unsigned int i = startRow; i<endRow; i++)
	{
		memcpy((unsigned char*) imageOut->raster + (imageOut->height - i - 1)*imageOut->bytesPerRow,
			   (unsigned char*) imageIn->raster + (imageIn->height - i - cropCornerY - 1)*imageIn->bytesPerRow + cropCornerX * imageIn->bytesPerPixel,
			   cropWidth*imageIn->bytesPerPixel);
	}
}

//---------------------------------------------------------------------------------------
//	There was a slight error in JYH's version of this function where it would
//	seg fault if the name of the input image was more than 5 letters. Fixed this using
//	libgen.h basename() function to allow it to dynamically allocate space for the
//	original filename.
//---------------------------------------------------------------------------------------
char* produceOutFilePath(char* inputImagePath, char* outFolderPath)
{
	// Produce the name of the output file
	//-------------------------------------
	//	First, find the start of the input file's name.  Start from the end
	//	and move left until we hit the first slash or the left end of the string.
	unsigned long k = strlen(inputImagePath) - 5;
	while ((k>=1) && (inputImagePath[k-1] != '/'))
		k--;
	
	//	*EDITED TO WORK WITH ALL FILENAME LENGTHS*
	char* outFilePath = (char*) malloc(strlen(outFolderPath) + strlen(" [cropped].tga") + strlen(basename(inputImagePath)));
	
	strcpy(outFilePath, outFolderPath);
	//	If outFolderPath didn't end with a slash, add it
	if (outFolderPath[strlen(outFolderPath)-1] != '/')
		strcat(outFilePath, "/");

	//	Produce the name of the input file minus extension
	char* inputFileRootName = (char*) malloc(strlen(inputImagePath+k) +1);
	strcpy(inputFileRootName, inputImagePath+k);
	//	chop off the extension by replacing the dot by '\0'
	inputFileRootName[strlen(inputFileRootName)-4] = '\0';

	//	Append root name to output path, add " [cropped].tga"
	strcat(outFilePath, inputFileRootName);
	strcat(outFilePath, " [cropped].tga");
	
	//	free heap-allocated data we don't need anymore
	free(inputFileRootName);
	
	return outFilePath;
}

void errorReport(ErrorCode code, const char* input)
{
	if (code != NO_ERROR)
	{
		switch (code)
		{
			case WRONG_NUMBER_OF_ARGUMENTS:
			break;
			
			case INVALID_CROP_X_TYPE:
				printf("Third argument is not a positive integer: %s\n", input);
			break;
			
			case INVALID_CROP_Y_TYPE:
				printf("Fourth argument is not a positive integer: %s\n", input);
			break;
			
			case INVALID_CROP_CORNER:
				printf("The crop region's upper-left corner must be within the image.\n");
			break;
			
			case INVALID_CROP_WIDTH_TYPE:
				printf("Fifth argument is not a positive integer: %s\n", input);
			break;
			
			case INVALID_CROP_HEIGHT_TYPE:
				printf("Sixth argument is not a positive integer: %s\n", input);
			break;
			
			case INVALID_CROP_SIZE:
			break;
			
			default:
				break;
		}
		exit(code);
	}
}

/*------------------------------------------------------------------------------------------------
This is the main thread function. It calls the modified crop function adjusted for threads.
Each thread will work a specified number of rows in the crop area of the image. These rows are
computed in main.
-------------------------------------------------------------------------------------------------*/
void* threadFunc(void* arg) {

	//	Pass the data from the thread info struct to the void* argument
	tInfo* info = (tInfo *) arg;

	//	Data from the structs is passed as parameters to the crop function
	cropImage(info->img1, info->img2, info->cropCornerX, info->cropCornerY, 
	          info->cropWidth, info->cropHeight, info->startRow, info->endRow);

	return(NULL);
}