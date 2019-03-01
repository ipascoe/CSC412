#!/bin/bash

#First if makes sure there is a file path provided
if [ "$#" -le 1 ]; then
   echo -e "Invalid Argument List.\nProper usage: myScript <path to executable> m1 [m2 [m3 [...]]]"
   exit
#Next else if tests whether the first argument is executable and if it is it performs the program
elif [ -x "$1" ]; then
    i=2
    while [ "$i" -le "$#" ]; do
	$1 ${!i}
	i=$((i+1))
    done
    j=2
    while [ "$j" -le "$#" ]; do
	k=$((j+1))
	while [ "$k" -le "$#" ]; do
	    $1 ${!j} ${!k}
	    k=$((k+1))
	done
	j=$((j+1))
    done
#This else case handles any other errors aka the file doesnt exist or is not executable
else
    echo -e "Invalid Argument List.\nProper usage: myScript <path to executable> m1 [m2 [m3 [...]]]"
    exit
fi
