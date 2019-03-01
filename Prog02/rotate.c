#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "imageIO_TGA.h"

char* createOutput(char* in, char* out);
void rotate90Left(ImageStruct* img1, ImageStruct* img2);
void rotate90Right(ImageStruct* img1, ImageStruct* img2);
void rotate180(ImageStruct* img1, ImageStruct* img2);

/*main handles all of the arguments and calls functions to modify them*/
int main(int argc, char* argv[]){
    assert(argc==4);
    char* rType = argv[1];
    assert(strlen(rType)<=2 && strlen(rType)>0);
    char* inputPath = argv[2];
    char* outputPath = argv[3];
    ImageStruct inputImg = readTGA(inputPath);
    ImageStruct outputImg;
    /*this conditional tests for the proper left and right argument provided in
      the call to the program.*/
    if(strlen(rType)==1 && rType[0]=='l'){
        outputImg = newImage(inputImg.height, inputImg.width, RGBA32_RASTER,1);
        rotate90Left(&inputImg, &outputImg);
    } else if(strlen(rType)==1 && rType[0]=='r'){
        outputImg = newImage(inputImg.height, inputImg.width, RGBA32_RASTER,1);
        rotate90Right(&inputImg, &outputImg);
    } else if(strlen(rType)==2 && (rType[0]=='l' || rType[0]=='r') && (rType[1]=='l' || rType[1]=='r')){
        outputImg = newImage(inputImg.width, inputImg.height, RGBA32_RASTER,1);
        rotate180(&inputImg, &outputImg);
    } else {
        printf("Not a valid rotation type.\n");
    }
    char* out = createOutput(inputPath, outputPath);
    writeTGA(out,&outputImg);
    return 0;
}

/*same output function as the other programs*/
char* createOutput(char* in, char* out){
    char* filename = basename(in);
    char* newfilename = (char*) calloc(strlen(filename)-4,sizeof(char));
    for(int i=0;i<strlen(filename)-4;i++){
        newfilename[i]=filename[i];
    } newfilename = strcat(newfilename," [rotated].tga");
    newfilename = strcat(out,newfilename);
    return newfilename;
}

//simple 90 degree rotation. Swap j and i while reading from the original
void rotate90Left(ImageStruct* img1, ImageStruct* img2){
    int* raster1 = (int*)(img1->raster);
    int* raster2 = (int*)(img2->raster);
    for(int i=0; i<img1->height; i++){
        for(int j=0;j<img1->width;j++){
            raster2[j*img1->height+(img1->height-1-i)]=raster1[i*img1->width+j];
        }
    }
}

//90 degree rotation to the right. Similar implementation as 90left
void rotate90Right(ImageStruct* img1, ImageStruct* img2){
    int* raster1 = (int*)(img1->raster);
    int* raster2 = (int*)(img2->raster);
    for(int i=0; i<img1->height; i++){
        for(int j=0;j<img1->width;j++){
            raster2[j*img1->height+i]=raster1[i*img1->width+j];
        }
    }
}

//Just re-implemented the mirrorImage function from the provided main.
void rotate180(ImageStruct* img1, ImageStruct* img2){
    int* raster1 = (int*)(img1->raster);
    int* raster2 = (int*)(img2->raster);
    unsigned int effectiveWidth  = img1->bytesPerRow / img1->bytesPerPixel;
    for (unsigned int i=0, mirrorI=img1->height-1; i<img1->height; i++, mirrorI--) {
        for (unsigned int j=0; j<img1->width; j++) {
            raster2[i*effectiveWidth + j] = raster1[mirrorI*effectiveWidth + j];
        }
    }
}
