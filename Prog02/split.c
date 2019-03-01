#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "imageIO_TGA.h"

char* createOutput(char* in, char* out);
void isoGreenChannel(ImageStruct* image);
void isoRedChannel(ImageStruct* image);
void isoBlueChannel(ImageStruct* image);

//Main reads command line arguments. Looks for the input path and output path
int main(int argc, char* argv[]){
  assert(argc==3); //will abort if the proper amount of arguments are not provided
    char* inputPath = argv[1];
    char* outputPath = argv[2];
    ImageStruct red=readTGA(inputPath);
    ImageStruct green=readTGA(inputPath);
    ImageStruct blue=readTGA(inputPath);
    isoGreenChannel(&green);
    isoRedChannel(&red);
    isoBlueChannel(&blue);
    char* fname = createOutput(inputPath, outputPath);
    char* redf = (char*) calloc(strlen(fname)+6,sizeof(char));
    redf = strcat(fname,"_r.tga\0");
    writeTGA(redf, &red);
    fname[strlen(fname)-6]='\0';
    char* greenf = (char*) calloc(strlen(fname)+6,sizeof(char));
    greenf = strcat(fname, "_g.tga\0");
    writeTGA(greenf, &green);
    fname[strlen(fname)-6]='\0';
    char* bluef = (char*) calloc(strlen(fname)+6,sizeof(char));
    bluef = strcat(fname,"_b.tga\0");
    writeTGA(bluef, &blue);
    fname[strlen(fname)-6]='\0';
    return 0;
}

//same output creation function
char* createOutput(char* in, char* out){
    char* filename = basename(in);
    char* newfilename = (char*) calloc(strlen(filename)-4,sizeof(char));
    for(int i=0;i<strlen(filename)-4;i++){
        newfilename[i]=filename[i];
    } newfilename = strcat(out,newfilename);
    return newfilename;
}

//Each of the following takes an image and kills 2 of the 3 color channels leaving 1 isolated.
//Implemented similar to the killGreenChannel funciton in main.c
void isoGreenChannel(ImageStruct* image) {
    int* raster = (int*)(image->raster);
    for (unsigned int i=0, mirrorI=image->height-1; i<image->height; i++, mirrorI--) {
        for (unsigned int j=0; j<image->width; j++) {
            unsigned char* rgba = (unsigned char*) (raster + i*image->width + j);
            rgba[0] = 0x00;
            rgba[2] = 0x00;
        }
    }
}

void isoRedChannel(ImageStruct* image) {
    int* raster = (int*)(image->raster);
    for (unsigned int i=0, mirrorI=image->height-1; i<image->height; i++, mirrorI--) {
        for (unsigned int j=0; j<image->width; j++) {
            unsigned char* rgba = (unsigned char*) (raster + i*image->width + j);
            rgba[1] = 0x00;
            rgba[2] = 0x00;
        }
    }
}

void isoBlueChannel(ImageStruct* image) {
    int* raster = (int*)(image->raster);
    for (unsigned int i=0, mirrorI=image->height-1; i<image->height; i++, mirrorI--) {
        for (unsigned int j=0; j<image->width; j++) {
            unsigned char* rgba = (unsigned char*) (raster + i*image->width + j);
            rgba[0] = 0x00;
            rgba[1] = 0x00;
        }
    }
}
