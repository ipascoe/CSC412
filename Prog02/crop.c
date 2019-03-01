#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "imageIO_TGA.h"

char* createOutput(char* in, char* out);
void crop(ImageStruct* full, ImageStruct* cropped, int x, int y);

//Main handles all the arguments and calls the other functions to modify those arguments correctly
int main(int argc, char* argv[]){
    assert(argc==7);
    char* inputPath = argv[1];
    ImageStruct input = readTGA(inputPath);
    char* exitPath = argv[2];
    unsigned long int xcoor = atol(argv[3]);
    unsigned long int ycoor = atol(argv[4]);
    unsigned long int width = atoi(argv[5]);
    unsigned long int height = atoi(argv[6]);
    ImageStruct output = newImage(width, height, RGBA32_RASTER, 1);
    crop(&input, &output, xcoor, ycoor);
    exitPath = createOutput(inputPath, exitPath);
    writeTGA(exitPath, &output);
    return 0;
}

/*This function takes two images as arguments.
It reads the desired pixels from the full image and puts them in
the smaller cropped image.*/
void crop(ImageStruct* full, ImageStruct* cropped, int x, int y){
    int* raster1 = (int*)(full->raster);
    int* raster2 = (int*)(cropped->raster);
    assert((cropped->width+x)<=full->width && (cropped->height+y)<=full->height);
    int k=0;
    for(int i=(full->height-1);i>=y;i--){
        for(int j=x;j<full->width;j++){
            if(abs(j-x)<cropped->width && abs(i-y)<cropped->height){ 
	      raster2[k]=raster1[((full->height-1)-i)*full->width+j];
                k++;
            } else {
                break;
            }
        }
    }
}

/*Creates the proper output format for the cropped image.
Ex) Adds the [cropped].tga file extention*/
char* createOutput(char* in, char* out){
    char* filename = basename(in);
    char* newfilename = (char*) calloc(strlen(filename)-4,sizeof(char));
    for(int i=0;i<strlen(filename)-4;i++){
        newfilename[i]=filename[i];
    } newfilename = strcat(newfilename," [cropped].tga");
    newfilename = strcat(out,newfilename);
    return newfilename;
}
