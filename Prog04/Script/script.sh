#!/bin/csh

#-----------------------------------------------------------------------------\
# Author: Ian Pascoe							      |
# Prog04 Due 11/5/18							      |
# 									      |
# This script takes arguments of a source directory that contains text files  |
# and an output file path that will contain the destination for the processes |
# once they are place in order by the script.				     |
#
# I could not find a use for the output path since my c programs already create
# their own. instead i have it just called with the input path.
#-----------------------------------------------------------------------------/

inpath=$1
cd ./"$inpath"
procCount=0
for f in *.txt; do
  count=$(head -c 1 $f)
  if (($procCount < $count));
  then
    procCount=$currentCount
  fi
done
cd ..
gcc v1.c -o v1
./v1 $procCount "$inpath"
