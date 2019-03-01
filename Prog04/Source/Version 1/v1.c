/*---------------------------------------------------------------------------------------------------------*\
*
* Author: Ian Pascoe
* Prog04 Due 11/5/18
*
* This program is the first version of several. This version is a mock
* version of future versions that will create new processes to handle
* portions of the work that this program takes care of single-handedly.
*
* Arguments to this program are the number of future child processes to
* create along with the folder that contains all of the data to be distributed.
* This folder is presumed to have only text files that are arbitrarily named.
*
* For example, could be run using: ./v1 2 '../../DataSet'
*
* Compiled by:
* gcc v1.c -o v1
*
\*---------------------------------------------------------------------------------------------------------*/

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
#include <math.h>

void getFilePaths(char* path, char*** filePaths, int* count);
void groupFiles(char*** groups, char** filePaths, int procCount, int groupSize);
void readFiles(char*** groups, int procCount, int groupSize);
void distribute(char*** groups, int procCount, int groupSize);
int compareStrings(const void* string1, const void* string2);
void printToOutput(char*** groups, int procCount, int groupSize);
//Simple function used for removing the numbers at the beginning of the line
int numDigits(int a) {
  if(a==0) return 1;
  if(a<10) return 1;
  if(a<100) return 2;
  if(a<1000) return 3;
  if(a<10000) return 4;
  if(a<100000) return 5;
  if(a<1000000) return 6;
  return 7;
}

int main(int argc, char* argv[]) {
	int procCount = atoi(argv[1]);
	char* sourcePath = argv[2];
	int sourceFileCount;
	char** sourceFiles;
	getFilePaths(sourcePath, &sourceFiles, &sourceFileCount);
	int groupSize = ceil(sourceFileCount/procCount);
	char*** groups = (char***) malloc(procCount*sizeof(char**));
	groupFiles(groups, sourceFiles, procCount, groupSize);
	readFiles(groups, procCount, groupSize);
	distribute(groups, procCount, groupSize);
	printToOutput(groups, procCount, groupSize);
	return 0;
}
/*---------------------------------------------------------------------------------------------------------------*\
This function stricly gets the file paths to all of the files in the DataSet.
These file paths are put into an array of strings. These will used later on
to open each text file and read the contents.
\*---------------------------------------------------------------------------------------------------------------*/
void getFilePaths(char* path, char*** filePaths, int* count) {
	DIR* directory = opendir(path);
	if (directory == NULL) {
		printf("data folder %s not found\n", path);
		exit(0);
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
	*count = counter;
	*filePaths = (char**) malloc(counter*sizeof(char*));
	//	Second pass: read the file names
	directory=opendir(path);
	int k=0;
	while ((entry = readdir(directory)) != NULL) {
		char* name = entry->d_name;
		//  Ignores "invisible" files (name starts with . char)
		if (name[0] != '.') {
			(*filePaths)[k] = malloc((strlen(name)+strlen(path)+2)*sizeof(char));
			strcpy((*filePaths)[k], path);
			strcat((*filePaths)[k],"/");
			strcat((*filePaths)[k], name);
			k++;
		}
	}
	closedir(directory);
}
/*---------------------------------------------------------------------------------------------------------------*\
This function takes into account the amount of processes to create and groups the files
evenly per process. These groups are put into a 2D array of strings (essentially a 3D array).
\*---------------------------------------------------------------------------------------------------------------*/
void groupFiles(char*** groups, char** filePaths, int procCount, int groupSize) {
	int k=0;
	for(int i=0; i<procCount; i++) {
		groups[i] = (char**) malloc(groupSize*sizeof(char*));
		for(int j=0; j<groupSize; j++) {
			groups[i][j] = malloc(strlen(filePaths[k]));
			strcpy(groups[i][j], filePaths[k]);
			k++;
		}
	}
}
/*---------------------------------------------------------------------------------------------------------------*\
This function reads the contents of each file in the DataSet. Then it replaces the file paths
that were originally placed in the 2D array of strings with the contents of the files.
\*---------------------------------------------------------------------------------------------------------------*/
void readFiles(char*** groups, int procCount, int groupSize) {
	for(int i=0; i<procCount; i++) {
		for(int j=0; j<groupSize; j++) {
			char line[100];
			FILE *fptr = fopen(groups[i][j], "r");
			fgets(line, 100, fptr);
			free(groups[i][j]);
			groups[i][j] = malloc(strlen(line)*sizeof(char));
			strcpy(groups[i][j], line);
		}
	}
}
/*---------------------------------------------------------------------------------------------------------------*\
This function distributes the contents of the files to their correct process. This is done
by looking at the index (second number in each line of the files) and comparing them.
I would have implemented the sort in place, but i could not figure out how to do it for a 2D array of strings.
Instead, I put the contents of the 2D array of strings and put them back into a 1D array, sort them
then put them back into the 2D array.
\*---------------------------------------------------------------------------------------------------------------*/
void distribute(char*** groups, int procCount, int groupSize) {
	char** temp = (char**) malloc(procCount*groupSize*sizeof(char*));
	int k=0;
	for(int i=0; i<procCount; i++){
		for(int j=0; j<groupSize; j++){
			temp[k] = malloc(strlen(groups[i][j])*sizeof(char));
			strcpy(temp[k],groups[i][j]);
			k++;
		}
	}
	qsort(temp, procCount*groupSize, sizeof(char*), compareStrings);
	int l=0;
	for(int i=0; i<procCount; i++) {
		for(int j=0; j<groupSize; j++) {
			free(groups[i][j]);
			groups[i][j] = malloc(strlen(temp[l]));
			strcpy(groups[i][j], temp[l]);
			l++;
		}
	}
	for(int i=0; i<k; i++){
		free(temp[k]);
	} free(temp);
}
//Simple comparator function for the qsort library function. Compares the indexes of each line.
int compareStrings(const void* string1, const void* string2){
	const char* s1 = *(const char**)string1;
	const char* s2 = *(const char**)string2;
	int proc1;
	int index1;
	int proc2;
	int index2;
	sscanf(s1, "%d %d", &proc1, &index1);
	sscanf(s2, "%d %d", &proc2, &index2);
	return index1-index2;
}
//Simply prints the final sorted product to the output file.
void printToOutput(char*** groups, int procCount, int groupSize) {
	system("mkdir Output");
	FILE *fp = fopen("Output/output.c", "w");
	for(int i=0; i<procCount; i++){
		for(int j=0; j<groupSize; j++){
			int tmp1, tmp2;
			sscanf(groups[i][j], "%d %d", &tmp1, &tmp2);
			int wSpace = 2;
			wSpace+=numDigits(tmp1);
			wSpace+=numDigits(tmp2);
			groups[i][j]=groups[i][j]+wSpace;
			fprintf(fp, "%s", groups[i][j]);
		}
	}
}
