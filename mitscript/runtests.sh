#!/bin/bash

DEFAULT="\033[39m"
GREEN="\033[32m"
RED="\033[31m"

memusg() {
  # detect operating system and prepare measurement
  case $(uname) in
      Darwin|*BSD) measure() { gtime -v $@; } ;;
      Linux) measure() { /usr/bin/time -v $@; } ;;
      *) echo "$(uname): unsupported operating system" >&2; exit 2 ;;
  esac

  measure $@ 2>&1 >/dev/null | grep "Maximum resident set" | awk 'NF{ print $NF }';
}

good() {
  echo -e "$GREEN ✓ $1 $DEFAULT [$2]"
}

bad() {
  echo -e "$RED ✘ $1 $DEFAULT [$2]"
}

check_memory() {
  echo "$1"
  bin/vm -mem 0 -s "$1"
  mem=$(memusg bin/vm -mem 0 -s "$1")
  if [[ "$mem" -lt "4096" ]]; then
    good "$1" "check_mem"
  else
    echo "used $mem kb"
    bad "$1" "check_mem"
  fi
}

for f in tests/garbagetest*.mit
do
  check_memory "$f"
done
