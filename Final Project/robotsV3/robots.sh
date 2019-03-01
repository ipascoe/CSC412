#!/bin/bash

#=============================================================================================
# Title: robots.sh
# Authors: Ian Pascoe & Michael Martel
# For Final Project Due 12-20-2018

# This script runs the robot simulation program for this project a certain amount of times
# as specified in the arguments. For accurate log file results, delete all current
# robotSimulOut files in the source directory. This script will place the output files it
# produces in a 'Multi-Sim Output' directory, so the log file indexes must match those of
# the index of simulations being run by this script.
#=============================================================================================

# Compile the C source code
gcc -Wall main.c gl_frontEnd.c -lm -lGL -lglut -lpthread -o robots

# This will create the output directory for the multiple simulations.
# We went with a windows style numbering system for the repeated
# directories.
outName="Multi-Sim Output"
if [[ -e $outName ]] ; then
    i=2
    while [[ -e "$outName ($i)" ]] ; do
        let i++
    done
    outName="$outName ($i)"
fi
mkdir "$outName"

# Arguments are expected to be:
# ./robots.sh <X & Y dimension of square grid> <# boxes/robots> <# doors> <# simulations to run>
dimensions=$1
numRobotsBoxes=$2
numDoors=$3
numSimulations=$4

# Asserts doors are within required bounds
if [ $3 -lt 1 ] || [ $3 -gt 3 ]; then
    echo "# of doors must be between 1 and 3"
    exit
fi

# Run simulation specified # of times
for i in `seq 1 $4`; do
    ./robots $1 $1 $2 $3
    sleep 0.2
done

# This loop moves the output files to the output directory
# created earlier
maxSimIndex=`expr $4 - 1`
for i in `seq 0 $maxSimIndex`; do
    mv "robotSimulOut $i.txt" -t "$outName"
done

# remove the executable file -> no longer needed
rm robots