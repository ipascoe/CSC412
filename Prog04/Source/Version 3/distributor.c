/*----------------------------------------------------------------------------*\

Author: Ian Pascoe
Prog04 Version 3

This is a helper function for v3.c. It acts as the distributor as described
in the assignment description. This code was taken from the createChild
function from v1.c and v2.c.

Needs to be compiled using:
gcc distributor.c -o distributor

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
#include <math.h>

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

int main(int argc, char* argv[]){
  int procNum = atoi(argv[1]);
  int groupSize = atoi(argv[2]);
  char buf[10];
  snprintf(buf, 10, "%d.txt", procNum);
  FILE *fp = fopen(buf, "r");
  char** group = (char**) malloc(groupSize*sizeof(char*));
  for(int i=0; i<groupSize; i++) {
    group[i] = malloc(1000*sizeof(char));
    fgets(group[i], 1000, fp);
  } fclose(fp);
  fp = fopen(buf, "w");
  qsort(group, groupSize, sizeof(char*), compareStrings);
  for(int i=0; i<groupSize; i++) {
    fprintf(fp, "%s", group[i]);
  } fclose(fp);
  if(fork() == 0) {
    char buf1[10];
    char buf2[10];
    snprintf(buf1, 10, "%d", procNum);
    snprintf(buf2, 10, "%d", groupSize);
    char* args[] = {"./processor", buf1, buf2, NULL};
    execvp(args[0], args);
  } usleep(100000);
  return 0;
}
