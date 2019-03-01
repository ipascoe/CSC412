#!/bin/bash

dim=$(./dimensions $1)
./rotate l $1 $2
./split $1 $2


width=${dim:0:3}
height=${dim:4:6}

hw=$(($width/2))
hh=$(($height/2))

./crop $1 $2 0 0 "$hw" "$hh"


