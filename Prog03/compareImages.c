/*------------------------------------------------------------------------------

Created by Ian Pascoe
for Prog 03 assignment due 10/11/18

Program compares two image files that are given on the command line.
Will return 1 if the images are identical. This includes rotations of
90, 180, 270 degrees.

-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include "imageIO_TGA.h"

int compareImages(ImageStruct *img1, ImageStruct *img2);

/*------------------------------------------------------------------------------

Main handles the amount of arguments on provided on the command line.
It will throw an error if the images cannot be opened, or if the amount
of arguments is more or less than expected. Also returns the result after
the compare images function is called.

-------------------------------------------------------------------------------*/
int main(int argc, char* argv[]){
  assert(argc==3);
  char* source = argv[1];
  char* test = argv[2];
  ImageStruct sImg = readTGA(source);
  ImageStruct tImg = readTGA(test);
  int result = compareImages(&sImg, &tImg);
  printf("Result: %d\n", result);
  return result;
}

/*

CompareImages is the meat of this program. It takes two parameters that are
targa images and compares them. First it tests for an exact match, then 90 degree
rotated match, then 180 degrees rotated match, lastly a 270 degree rotated match.

 */
int compareImages(ImageStruct *img1, ImageStruct *img2) {
  int result;
  int* raster1 = (int*)(img1->raster);
  int* raster2 = (int*)(img2->raster);
  //checks first for an exact match
    for(int i=0;i<img1->height;i++){
      for(int j=0;j<img1->width;j++){
	if(raster1[i*img1->height+j] != raster2[i*img2->height+j]){
	  result = 0;
	  goto secondCheck;
	} else {
	  result = 1;
	}
      }
    } if(result == 1) {goto end;}
  //90 degree rotation
  secondCheck:
    for(int i=0; i<img1->height; i++){
      for(int j=0;j<img1->width;j++){
	if(raster2[(img2->height-j-1)*img2->width+i]!= raster1[i*img1->width+j]){
	  result = 0;
	  goto thirdCheck;
	} else {
	  result=1;
	}
      }
    } if(result == 1) {goto end;}
  //180 degree rotation
  thirdCheck:
    for(int i=0;i<img1->height;i++){
      for(int j=0;j<img1->width;j++){
	if(raster2[(img2->height-i-1)*img2->width+(img2->width-j-1)]!= raster1[i*img1->width+j]) {
	  result = 0;
	  goto fourthCheck;
	} else {
	  result=1;
	}
      }
    } if(result == 1) {goto end;}
  //270 degree rotation
  fourthCheck:
    for(int i=0;i<img1->height;i++){
      for(int j=0;j<img1->width;j++){
	if(raster2[j*img2->width+img2->width-i-1]!= raster1[i*img1->width+j]){
	  result = 0;
	  goto end;
	} else {
	  result=1;
	}
      }
    }
    end:
    return result;
}
