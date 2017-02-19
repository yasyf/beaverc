#!/bin/bash

DEFAULT="\e[39m"
GREEN="\e[32m"
RED="\e[31m"

check() {
  cat "$1" | ./main > /dev/null
  if [[ $? = $2 ]]; then
    echo -e "$GREEN ✓ $1 $DEFAULT"
  else
    echo -e "$RED ✘ $1 $DEFAULT"
  fi
}

check_good() {
  check $1 0
}

check_bad() {
  check $1 1
}

for f in tests/good*.mit
do
  check_good "$f"
done

for f in tests/test*.mit
do
  check_good "$f"
done

for f in tests/bad*.mit
do
  check_bad "$f"
done
