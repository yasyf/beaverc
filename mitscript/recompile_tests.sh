#!/bin/bash

DEFAULT="\033[39m"
GREEN="\033[32m"

message() {
  echo -e "$GREEN $1 $DEFAULT"
}

compile() {
  message "Recompiling $1"
  ./bin/bccompiler < "$1" > "$1bc"
}

make bccompiler

for f in tests/bytecodetest*.mit tests/interptest*.mit
do
  compile "$f"
done
