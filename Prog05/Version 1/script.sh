#!/bin/bash

#----------------------------------------------------------------------------------
# Title: script.sh
# Author: Ian Pascoe
# For Prog05 Due 11/18/18
#
# This script expects 4 arguments:
# 1. # of threads to create
# 2. Path to a drop folder to watch(using inotifywait)
# 3. Path to a folder to put images(data folder)
# 4. Path to an output folder
# 
# inotify-tools must be installed for this script to work.
#----------------------------------------------------------------------------------

# Compile all programs
gcc crop.c rasterImage.c imageIO_TGA.c -lpthread -o crop
gcc rotate.c rasterImage.c imageIO_TGA.c -lpthread -o rotate
gcc split.c rasterImage.c imageIO_TGA.c -lpthread -o split

echo -e "Programs compiled!"

# Initialize the variables we need.
threadCount=$1
dropFolder=$2
dataFolder=$3
outputPath=$4

#-----------------------------------------------------------
# First conditional is to check for identical directories.
# Does not check for symbolic or hard links.
#-----------------------------------------------------------
if [ $2 != $3 ] && [ $3 != $4 ] && [ $2 != $4 ]
then

    #---------------------------------------------
    # Next set of conditionals checks to see if
    # the directories exist yet. If not, create
    # them.
    #---------------------------------------------
    if [ -d $2 ]; then
        echo "Directory $2 exists"
    else
        mkdir $2
        echo "Directory $2 created"
    fi
    
    if [ -d $3 ]; then
        echo "Directory $3 exists"
    else
        mkdir $3
        echo "Directory $3 created"
    fi

    if [ -d $4 ]; then
        echo "Directory $4 exists"
    else
        mkdir $4
        echo "Directory $4 created"
    fi

    #-------------------------------------------------------------
    # This function call utilizes inotify-tools which will
    # monitor the drop folder for two specific types of files,
    # .job & .tga files.
    #-------------------------------------------------------------
    inotifywait -m $dropFolder -e create -e moved_to |
        while read $dropFolder action file; do
            
            # If .tga image, move to data folder
            if [[ "$file" =~ .*tga$ ]]; then
                mv "$dropFolder/$file" $dataFolder
                echo -e "$file moved to $dataFolder"

            # If .job file, read line by line and execute tasks
            elif [[ "$file" =~ .*job$ ]]; then
                while IFS= read -r line
                do
                    lline=($line)
                    count=${#lline[@]}
                    if [[ ${lline[0]} = "split" ]] && [[ $count -eq 2 ]]; then
                        echo "Task found in .job file:"
                        echo "Splitting ${lline[1]} using $1 threads."
                        lline+=("$outputPath")
                        lline+=("$1")
                        ./${lline[@]}
                        echo "Done!"
                    elif [[ ${lline[0]} = "crop" ]] && [[ $count -eq 6 ]]; then
                        echo "Task found in .job file:"
                        echo "Cropping ${lline[1]} using $1 threads."
                        ./${lline[0]} ${lline[1]} "$outputPath" ${lline[2]} ${lline[3]} ${lline[4]} ${lline[5]} "$1"
                        echo "Done!"
                    elif [[ ${lline[0]} = "rotate" ]] && [[ $count -eq 3 ]]; then
                        echo "Task found in .job file:"
                        echo "Rotating ${lline[2]} using $1 threads."
                        ./${lline[0]} ${lline[1]} ${lline[2]} "$outputPath" "$1"
                        echo "Done!"
                    fi
                done <"$dropFolder/$file"
            else
                echo "Unsupported File Type"
            fi
        done

# If any of the directories are identical, exit the program and throw error message
else
    echo -e "Two or more of the directories provided are identical\nUse separate directories"
    exit 
fi 