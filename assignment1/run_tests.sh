#!/bin/bash

DEFAULT="\e[39m"
GREEN="\e[32m"
RED="\e[31m"

check_parse() {
  cat "$1" | ./printer > /dev/null
  if [[ $? -eq $2 ]]; then
    echo -e "$GREEN ✓ $1 $DEFAULT"
  else
    echo -e "$RED ✘ $1 $DEFAULT"
  fi
}

check_interpret() {
  output=$(diff "$1.out" <(./interpreter "$1"))
  if [[ -z $output ]]; then
    echo -e "$GREEN ✓ $1 $DEFAULT"
  else
    echo -e "$RED ✘ $1 $DEFAULT"
    echo "$output"
  fi
}

check_good_parse() {
  check_parse $1 0
}

check_bad_parse() {
  check_parse $1 1
}

for f in tests/good*.mit
do
  check_good_parse "$f"
done

for f in tests/test*.mit
do
  check_good_parse "$f"
done

for f in tests/bad*.mit
do
  check_bad_parse "$f"
done

for f in tests/interptest*.mit
do
  check_interpret "$f"
done

for f in tests/staff/test*.mit
do
  check_interpret "$f"
done
