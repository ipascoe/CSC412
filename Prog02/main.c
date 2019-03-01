 //
//  main.c
//  Handout code for Prog 02, CSC412 - Fall 2018
//
//	This small program shows how to use the image I/O library
//	to read and write images in the uncompressed TGA file format,
//	and to create a new image raster.
//
//  Created by Jean-Yves Hervé on 2018-09-18.
//

#include <stdio.h>
#include <stdlib.h>	//	for malloc & calloc
#include <string.h>	//	for memcpy
//
#include "imageIO_TGA.h"

//----------------------
//	Function prototypes
//----------------------
void mirrorImage(ImageStruct* image1, ImageStruct* image2);	
void killGreenChannel(ImageStruct* image);

//----------------------
//	Hard-coded paths
//----------------------
#define INPUT_PATH		"../Images/bottles.tga"
#define OUTPUT_PATH1	"../Output/out1.tga"
#define OUTPUT_PATH2	"../Output/out2.tga"


int main(int argc, char** argv)
{
	//-----------------------------------------------------------------------------
	//	Part 1
	//	I read my image using a hard-coded path.  Obviously, you would want to
	//	get the path from the list of arguments.
	//-----------------------------------------------------------------------------
	ImageStruct image1 = readTGA(INPUT_PATH);
	
	//	We only work with color images
	if (image1.type != RGBA32_RASTER)
	{
		printf("The file %s does not contain a color image.\n", INPUT_PATH);
		exit(10);
	}
	
	//-----------------------------------------------------------------------------
	//	Part 2
	//	I create 1 blank (well, really, all black) image with the same dimensions as
	//	the image I just read
	//-----------------------------------------------------------------------------
	ImageStruct image2 = newImage(image1.width, image1.height, RGBA32_RASTER, 1);

	//-----------------------------------------------------------------------------
	//	Part 3
	//	I perform some simple operations on my image
	//-----------------------------------------------------------------------------
	mirrorImage(&image1, &image2);
	
	killGreenChannel(&image1);
			
	//-----------------------------------------------------------------------------
	//	Part 4
	//	Output the images
	//-----------------------------------------------------------------------------
	writeTGA(OUTPUT_PATH1, &image1);
	writeTGA(OUTPUT_PATH2, &image2);
	
	//	Here I am in the main function, so all memory allocated would get freed when 
	//	I quit, but, just for the purpose of keeping good habits, I delete my rasters.
	free(image1.raster);
	free(image2.raster);
	
	return 0;
}

//====================================================================
#if 0
#pragma mark -
#pragma mark Image Processing Functions
#endif
//====================================================================

//	Write into image2 a mirrored-vertically copy of image1
void mirrorImage(ImageStruct* image1, ImageStruct* image2)
{
	int* raster1 = (int*)(image1->raster);
	int* raster2 = (int*)(image2->raster);

	//	Here, I do it two different ways.  
	//	Edit the #if statement below to change the version running
	#if 1

		//	Version 1: Copy the pixels one by one

		//	"effective width" is nominal width + row padding (to multiple of 1/4/8/16/etc.)
		unsigned int effectiveWidth  = image1->bytesPerRow / image1->bytesPerPixel;

		for (unsigned int i=0, mirrorI=image1->height-1; i<image1->height; i++, mirrorI--)
		{
			for (unsigned int j=0; j<image1->width; j++)
			{
				raster2[i*effectiveWidth + j] = raster1[mirrorI*effectiveWidth + j];
				
				//	If you had implemented the 2D raster, you simply write
				//	raster2[i][j] = raster1[mirrorI][j]
				
			}
		}

	#else

		//	Version 2: Copy entire rows, using memcpy
		for (unsigned int i=0, mirrorI=image1->height-1; i<image1->height; i++, mirrorI--)
		{
			//	Do you know why I need to cast the raster pointer to char*?
			//	Don't move away from here until you understand why.
			memcpy(	(char*) raster2 + i*image2->bytesPerRow,		//	destination
					(char*) raster1 + mirrorI*image1->bytesPerRow,	//	source	
					image1->bytesPerRow);							//	size of copy

		}

	#endif
}

//	Now modify image in-place by "killing" the green color channel 
//	Again, I do it two different ways, so that you can use the one 
//	you prefer.
void killGreenChannel(ImageStruct* image)
{
	int* raster = (int*)(image->raster);
	
	#if 1
	
		//	Version 1: map an array of 4 unsigned char at the location of the pixel
		//	"effective width" is nominal width + row padding (to multiple of 1/4/8/16/etc.)

		for (unsigned int i=0, mirrorI=image->height-1; i<image->height; i++, mirrorI--)
		{
			for (unsigned int j=0; j<image->width; j++)
			{
				unsigned char* rgba = (unsigned char*) (raster + i*image->width + j);
				
				//	rgba[0] -> red, rgba[1] -> green, rgba[2] -> blue, rgba[3] -> alpha
				rgba[1] = 0x00;
			}
		}		
	
	#else

		//	Version 2: use bitwise operators

		for (unsigned int i=0, mirrorI=image->height-1; i<image->height; i++, mirrorI--)
		{
			for (unsigned int j=0; j<image->width; j++)
			{
				//	x86 CPUs are small-endian, therefore the order of bytes in an int
				//	is inverted.  So within the int it looks as if we have ABGR
				//	
				//										preserve -----------+-+---+
				//															| |   |
				//															A B G R
				raster[i*image->width + j] = raster[i*image->width + j] & 0xFFFF00FF;
				//																|
				//											kill ---------------+
			}
		}		
	
	
	#endif
	
}
