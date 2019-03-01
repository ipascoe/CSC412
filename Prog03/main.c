//
//  main.cpp
//  Directory Operations, C version
//
//	This program takes as argument the path to a folder and
//	outputs the name of all the files found in that folder
//
//	This version does it in two passes.
//		o The first pass counts the number of files in the directory
//		then the program allocates the array that will store file
//		names (now that the number of files is known)
//		o The second pass now reads and stores the names
//	Edited 2018-03-30

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int main(int argc, const char* argv[]) {

	char* dataRootPath = argv[1];
	
    DIR* directory = opendir(dataRootPath);
    if (directory == NULL) {
		printf("data folder %s not found\n", dataRootPath);
		return 1;
	}
	
    struct dirent* entry;
	int counter = 0;
	
	//	First pass: count the entries
    while ((entry = readdir(directory)) != NULL) {
        char* name = entry->d_name;
        if (name[0] != '.') {
			counter++;
        }
    }
	closedir(directory);
	
	printf("%d files found\n", counter);

	//	Now allocate the array of file names
    char** fileName = (char**) malloc(counter*sizeof(char*));

	//	Second pass: read the file names
	int k=0;
	directory = opendir(dataRootPath);
    while ((entry = readdir(directory)) != NULL) {
        char* name = entry->d_name;
        //	Ignores "invisible" files (name starts with . char)
        if (name[0] != '.') {
			fileName[k] = malloc((strlen(name) +1)*sizeof(char));
			strcpy(fileName[k], name);
			k++;
        }
    }
	closedir(directory);


	for (int k=0; k<counter; k++)
		printf("\t%s\n", fileName[k]);

	return 0;
}
