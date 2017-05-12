#!/bin/bash

DEFAULT="\033[39m"
GREEN="\033[32m"
RED="\033[31m"
MAX_MEM="4096"

memusg() {
  $@ | tail -1 | cut -d' ' -f1
}

good() {
  echo -e "$GREEN ✓ $1 $DEFAULT [$2]"
}

bad() {
  echo -e "$RED ✘ $1 $DEFAULT [$2]"
}

check_memory() {
  mem=$(memusg bin/vm --show-memory --mem 0 -s "$1")
  if [[ "$mem" =~ ^[0-9]+$ ]]; then
    if [[ "$mem" -lt "$MAX_MEM" ]]; then
      good "$1" "check_mem $mem kb"
    else
      echo "used $mem > $MAX_MEM kb"
      bad "$1" "check_mem $mem kb"
    fi
  else
    bad "$1" "program was killed or errored"
  fi
}

for f in tests/garbagetest*.mit
do
  check_memory "$f"
done
