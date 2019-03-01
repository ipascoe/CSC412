#!/bin/bash

#----------------------------------------------\
# Author: Ian Pascoe                           |
# For Prog 03 - Due 10/11/18                   |
#                                              |
# Script that takes two arguments that are     |
# directories and compares images in both      |
# matches found will then have a symbolic link |
# to them in an output folder.                 |
#----------------------------------------------/

gcc compareImages.c imageIO_TGA.c rasterImage.c -o compare

sourceDir=$1
searchDir=$2

# Tests if the last character of the directories has a slash
sourceLast="$(echo -n $sourceDir | tail -c 1)"
searchLast="$(echo -n $searchDir | tail -c 1)"

slash="/"
# Adds slashes to end of directory path as needed
if [ "$sourceLast" != "/" ]; then
  sourceDir="$sourceDir$slash"
fi

if [ "$searchLast" != "/" ]; then
  searchDir="$searchDir$slash"
fi

output="Output"
# Create output directory
mkdir $output &> /dev/null

#---------------------------------------------------------------\
# I use a nested for loop with the bash 'find' function to find |
# any files in the directory and its subdirectories that have   |
# the file extension '.tga                                      |
#---------------------------------------------------------------/
for testImage in `find $searchDir -name "*.tga"`; do
    for sourceImage in `find $sourceDir -name "*.tga"`; do
	./compare $sourceImage $testImage &> /dev/null
	comp=$?
	if [ $comp -eq 1 ]; then
	    # Make a folder for a match
	    sourceImageName=${sourceImage##*/}
	    testImageName=${testImage##*/}
	    resultsDir="$sourceImageName-results"
	    mkdir "$output$slash$resultsDir" &> /dev/null
	    # Make a symbolic link to the match
	    ln -s $testImage "$output$slash$resultsDir$slash$testImageName-link" &> /dev/null
	fi
    done
done


