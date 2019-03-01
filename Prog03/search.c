/*------------------------------------------------------------------------------
 * 
 * Author: Ian Pascoe
 * Prog 03 - Due 10/11/18
 * 
 * This program was tested using the sample images form Prog 02 (clown.tga,
 * peppers.tga, cells.tga). In the search directory (Images2) there was the exact 
 * image and a rotated version of each. In the source directory there were the three 
 * basic images.
 * 
 * Tested by running:
 * ./search '../Prog03/Images' '../Prog03/Images2'
 * 
 * Compiled by running:
 * gcc -Wall search.c rasterImage.c imageIO_TGA.c -o search
 * 
\*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <assert.h>
#include "imageIO_TGA.h"

/*----------------------------------------------------------------
 * 
 * CompareImages was written in the first part of the assignment
 * 
\*---------------------------------------------------------------*/
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
/*----------------------------------------------------------------------------*\
 * This function takes the file path that has the images to be tested and      |
 * recursively searches through them. While doing this it tests the image      |
 * found using the compare images function. The image found while searching    |
 * is tested against every image in the source file then moves to the next.    | 
\*----------------------------------------------------------------------------*/
     
void getFilesAndCompare(const char *basePath, char** sourceFiles, int sourceCount) {
    char path[100];
    struct dirent *ent;
    DIR *dir;
    dir = opendir(basePath);
    // Unable to open directory
    if (!dir) {
        return; 
    }
    while ((ent = readdir(dir)) != NULL) {
        int comp=0;
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, ent->d_name);
            if(path[strlen(path)-4] == '.' && path[strlen(path)-3] == 't' && path[strlen(path)-2] == 'g' && path[strlen(path)-1] == 'a'){
                //now test the files using compareimages as created before
                ImageStruct img1 = readTGA(path);
                for(int i=0;i<sourceCount;i++){
                    //creates an empty string for output text file name
                    char buf[100];
                    char* outputText=(char*) calloc(strlen(sourceFiles[i]),sizeof(char));
                    //names the text file after the source image file
                    strcpy(outputText,basename(sourceFiles[i]));
                    snprintf(buf,sizeof(buf),"Output/%s.txt",outputText);
                    FILE *fptr = fopen(buf, "a");
                    ImageStruct img2 = readTGA(sourceFiles[i]);
                    comp = compareImages(&img1,&img2);
                    if(comp == 1){
                        //handles what is written to the file
                        fprintf(fptr,"%s\n",path); 
                    } else { continue; }     
                }
            }
            getFilesAndCompare(path, sourceFiles, sourceCount);  
        } 
    }
    // Close directory stream
    closedir(dir);
}
/*----------------------------------------------------------------------------*\
 * Most of the credit for this main function goes to Herve. I re-purposed the  |
 * directory operations main.c file to handle my source images and put them in |
 * a string for testing.                                                       | 
\*----------------------------------------------------------------------------*/        
int main(int argc, char* argv[]) {
    system("mkdir Output"); //make output directory
    char* sourcePath = argv[1];
    DIR* directory = opendir(sourcePath);
    if (directory == NULL) {
        printf("data folder %s not found\n", sourcePath);
	return 1;
    }
    struct dirent* entry;
    int sourceCount = 0;
    //	First pass: count the entries
    while ((entry = readdir(directory)) != NULL) {
        char* name = entry->d_name;
        if (name[0] != '.') {
            sourceCount++;
        }
    }
    closedir(directory);
    //	Now allocate the array of file names
    char** sourceFiles = (char**) malloc(sourceCount*sizeof(char*));
    //	Second pass: read the file names
    int k=0;
    directory = opendir(sourcePath);
    while ((entry = readdir(directory)) != NULL) {
        char* name = entry->d_name;
        //  Ignores "invisible" files (name starts with . char)
        if (name[0] != '.' && name[1] != '.') {
            sourceFiles[k] = malloc((strlen(name)+strlen(sourcePath)+1)*sizeof(char));
            strcpy(sourceFiles[k], sourcePath);
            strcat(sourceFiles[k],"/");
            strcat(sourceFiles[k],name);
            k++;
        }
    }
    closedir(directory);
    getFilesAndCompare(argv[2],sourceFiles,sourceCount);
    return 0;
}
