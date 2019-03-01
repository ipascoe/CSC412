#include <stdio.h>
#include <stdlib.h>
//
#include "rasterImage.h"

//---------------------------------------
//	Image utility functions.
//	Should be moved to some Image.c file
//---------------------------------------
ImageStruct newImage(unsigned int width, unsigned int height, ImageType type,
					 unsigned int wordSizeRowPadding)
{
	if (width<3 || height<3)
	{
		printf("Image size should be at least 3x3\n");
		exit(14);
	}
	if (wordSizeRowPadding!=1 && wordSizeRowPadding!=4 &&
		wordSizeRowPadding!=8 && wordSizeRowPadding!=16 &&
		wordSizeRowPadding!=32 && wordSizeRowPadding!=64)
	{
		printf("wordSizeRowPadding must be one of: 1, 4, 8, 16, 32, or 64\n");
		exit(15);
	}

	ImageStruct img;
	img.width = width;
	img.height = height;
	img.type = type;
	switch (type)
	{
		case RGBA32_RASTER:
		img.bytesPerPixel = 4;
		break;
		
		case GRAY_RASTER:
		img.bytesPerPixel = 1;
		break;
		
		case FLOAT_RASTER:
		img.bytesPerPixel = sizeof(float);
		break;
	}

	img.bytesPerRow = ((img.bytesPerPixel * width + wordSizeRowPadding - 1)/wordSizeRowPadding)*wordSizeRowPadding;

	unsigned int effectiveWidth = img.bytesPerRow / img.bytesPerPixel;	
	img.raster = (void*) calloc(height*effectiveWidth, img.bytesPerPixel);
//        img.raster2D = (void**) calloc(effectiveWidth, sizeof(void*));
	return img;
}
