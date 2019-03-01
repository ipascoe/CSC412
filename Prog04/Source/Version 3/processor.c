/*----------------------------------------------------------------------------*\

Author: Ian Pascoe
Prog04 Version 3

This is a helper program for v3.c. It is essentially the 'processor' that
is described in the instructions. This code was originally contained within
the createChild function in v2.c, except now it uses text files to communicate.

Needs to be compiled by using:
gcc processor.c -o processor

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

int main(int argc, char* argv[]) {
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
  fp = fopen("temp.txt", "a");
  for(int i=0; i<groupSize; i++) {
    fprintf(fp, "%s", group[i]);
  } fclose(fp);
  remove(buf);
  return 0;
}
