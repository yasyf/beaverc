#!/bin/bash

make clean vm

NAME=$(git rev-parse HEAD)

cp bin/vm snapshots/$NAME

echo "$NAME,--mem 0 -s" >> snapshots/snapshots.txt
