#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "imageIO_TGA.h"

int getWidth(ImageStruct *img);
int getHeight(ImageStruct *img);

//main does most of the work here. takes filepath and gets the width and height of image
int main(int argc, char** argv){
    assert(argc==2 || argc==3);
    if(argc==2){
        assert(argv[1]!=NULL);
        ImageStruct inputimg = readTGA(argv[1]);
        int width = getWidth(&inputimg);
        int height = getHeight(&inputimg);
        printf("%d %d\n", width, height);
    } else if(argc==3) {
        char* flag = argv[1];
        ImageStruct inputimg = readTGA(argv[2]);
	//this conditional just tests the input parameters to make sure theyre correct, and then applies them
        if(flag[0]=='-' && flag[1]=='w' && strlen(flag)==2){
            int width = getWidth(&inputimg);
            printf("%d\n",width);
        } else if(flag[0]=='-' && flag[1]=='h' && strlen(flag)==2){
            int height = getHeight(&inputimg);
            printf("%d\n",height);
        } else if(flag[0]=='-' && flag[1]=='v' && strlen(flag)==2){
            int width = getWidth(&inputimg);
            int height = getHeight(&inputimg);
            printf("Width: %d Height: %d\n",width, height);
        } else if(flag[0]=='-' && flag[1]=='w' && flag[2]=='v' && strlen(flag)==3){
            int width = getWidth(&inputimg);
            printf("Width: %d\n",width);
        } else if(flag[0]=='-' && flag[1]=='v' && flag[2]=='w' && strlen(flag)==3){
            int width = getWidth(&inputimg);
            printf("Width: %d\n",width);
        } else if(flag[0]=='-' && flag[1]=='h' && flag[2]=='v' && strlen(flag)==3){
            int height = getHeight(&inputimg);
            printf("Height: %d\n",height);
        } else if(flag[0]=='-' && flag[1]=='v' && flag[2]=='h' && strlen(flag)==3){
            int height = getHeight(&inputimg);
            printf("Height: %d\n",height);
        } else {
            printf("Incorrect Flag\nCorrect Options: -v -w -h -wv -vh -vw -hv\n");
        }
    }
    return 0;
}

//Don't think this needs any explanation
int getWidth(ImageStruct *img){
    int width = img->width;
    return width;
}

int getHeight(ImageStruct *img){
    int height = img->height;
    return height;
}
