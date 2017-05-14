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

run() {
  message "Running $1"
  ./bin/vm -s "$1" > "$1.out"
}

make bccompiler vm

for f in tests/bytecodetest*.mit tests/interptest*.mit tests/asmtest*.mit
do
  compile "$f"
  run "$f"
done
