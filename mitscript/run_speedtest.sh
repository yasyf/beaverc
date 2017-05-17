#!/bin/bash

DEFAULT="\033[39m"
GREEN="\033[32m"
RED="\033[31m"

good() {
  echo -e "$GREEN ✓ $1 $DEFAULT [$2]"
}

bad() {
  echo -e "$RED ✘ $1 $DEFAULT [$2]"
}


make vm

good "Running textproc"
time ./bin/vm --memory-usage --opt=all --mem 5 tests/PerfTests/textproc.mit < tests/PerfTests/textproc.input > /dev/null
good "Running treeproc"
time ./bin/vm --memory-usage --opt=all --mem 1000 tests/PerfTests/treeproc.mit < tests/PerfTests/treeproc.input > /dev/null
good "Running bignum"
time ./bin/vm --memory-usage --opt=all --mem 1000 tests/PerfTests/bignum.mit < tests/PerfTests/bignum.input > /dev/null
good "Running carsim"
time ./bin/vm --memory-usage --opt=all --mem 1000 tests/PerfTests/carsim.mit > /dev/null
good "Running kmediods"
time ./bin/vm --memory-usage --opt=all --mem 1000 tests/PerfTests/kmediods.mit < tests/PerfTests/kmediods.input > kmediods.mit.output
diff kmediods.mit.output tests/PerfTests/kmediods.output
if [ $? -ne 0 ]; then
    bad "Output isn't what we expected"
fi
rm kmediods.mit.output
good "Running life"
time ./bin/vm --memory-usage --opt=all --mem 1000 tests/PerfTests/life.mit < tests/PerfTests/life.input > life.mit.output
diff life.mit.output tests/PerfTests/life.output
if [ $? -ne 0 ]; then
    bad "Output isn't what we expected"
fi
rm life.mit.output
