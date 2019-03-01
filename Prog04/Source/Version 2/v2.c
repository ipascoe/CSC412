/*---------------------------------------------------------------------------------------------------------*\
*
* Author: Ian Pascoe
* Prog04 Due 11/5/18
*
* This program is the second version of several. Unlike the first, this program splits the work
* up amongst 2n processes.
*
* Arguments to this program are the number of child processes to
* create along with the folder that contains all of the data to be distributed.
* This folder is presumed to have only text files that are arbitrarily named.
*
* For example, could be run using: ./v2 2 '../../DataSet'
*
* Compiled by:
* gcc v2.c -o v2
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
void readFiles(char** group, int groupSize);
void sortInGroup(char** group, int groupSize);
int compareStrings(const void* string1, const void* string2);
void createChildren(int procCount, int groupSize, char** files);
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
//------------------------------------------------------------------------------
// Main is responsible for calling the functions that I created below.
// It creates the child processes by calling createChildren and then reads
// the temp text file to finally sort the group contents once concatenated
// together.
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
	int procCount = atoi(argv[1]);
	char* sourcePath = argv[2];
  int sourceCount=0;
  char** files;
  getFilePaths(sourcePath, &files, &sourceCount);
  int groupSize = ceil(sourceCount/procCount);
  createChildren(procCount, groupSize, files);
  char** masterList = (char**) malloc(sourceCount*sizeof(char*));
  FILE *fp = fopen("temp.txt","r");
  for(int i=0; i<sourceCount; i++) {
    masterList[i] = malloc(1000*sizeof(char));
    fgets(masterList[i], 1000, fp);
  }
  fclose(fp);
  system("mkdir Output");
  fp = fopen("Output/output.c","a");
  sortInGroup(masterList, sourceCount);
  for(int i=0; i<sourceCount; i++) {
		int tmp1, tmp2;
		sscanf(masterList[i], "%d %d", &tmp1, &tmp2);
		int wSpace = 2;
		wSpace+=numDigits(tmp1);
		wSpace+=numDigits(tmp2);
		masterList[i]=masterList[i]+wSpace;
    fprintf(fp, "%s", masterList[i]);
  }
  fclose(fp);
  remove("temp.txt");
	return 0;
}
/*---------------------------------------------------------------------------------------------------------------*\
This function creates the child processes that this version is responsible for. This one function creates the
2n functions. Each child that is initially created, creates another child. The first child takes care of creating
the groups of files. Therefore, each process acts like a distributor. The child of this child is the processor.
Each processor writes to the temp.txt file, which essentially acts like the server for the final architecture.
From there, main takes care of sorting the final masterList.
\*---------------------------------------------------------------------------------------------------------------*/
void createChildren(int procCount, int groupSize, char** files) {
  for(int i=1; i<=procCount; i++){
    if(fork()==0){
      char** group = (char**) malloc(groupSize*sizeof(char*));
      int k=0;
      for(int j=groupSize*i-groupSize; j<groupSize*i; j++) {
        group[k]= malloc(strlen(files[j])*sizeof(char));
        strcpy(group[k],files[j]);
        k++;
      }
      readFiles(group, groupSize);
      pid_t p = fork();
      if(p==0){
        sortInGroup(group, groupSize);
        FILE *fp = fopen("temp.txt", "a");
        for(int m=0; m<groupSize; m++){
          fprintf(fp, "%s", group[m]);
        } fclose(fp);
      }
      exit(0);
    }
    usleep(10000);
  }
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
This function reads the contents of the data files and puts them in an array of strings.
\*---------------------------------------------------------------------------------------------------------------*/
void readFiles(char** group, int groupSize) {
	for(int i=0; i<groupSize; i++) {
			char line[100];
			FILE *fptr = fopen(group[i], "r");
			fgets(line, 100, fptr);
			free(group[i]);
			group[i] = malloc(strlen(line)*sizeof(char));
			strcpy(group[i], line);
	}
}
/*---------------------------------------------------------------------------------------------------------------*\
This function will sort the contents that were read by readFiles() and sort them by the index. which
is the second number in each line. This function is called twice, once to sort the contents in each group. then
a final time to sort the masterList.
\*---------------------------------------------------------------------------------------------------------------*/
void sortInGroup(char** group, int groupSize) {
	 qsort(group, groupSize, sizeof(char*), compareStrings);
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
