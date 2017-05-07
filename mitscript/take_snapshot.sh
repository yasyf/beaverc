#!/bin/bash

make clean vm

NAME=$(git rev-parse HEAD)

cp bin/vm speedtest/$NAME

echo "$NAME,--mem 0 -s" >> speedtest/snapshots.txt
