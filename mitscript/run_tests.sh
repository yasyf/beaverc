#!/bin/bash

DEFAULT="\e[39m"
GREEN="\e[32m"
RED="\e[31m"

good() {
  echo -e "$GREEN ✓ $1 $DEFAULT [$2]"
}

bad() {
  echo -e "$RED ✘ $1 $DEFAULT [$2]"
}

check_parse() {
  cat "$1" | ./bin/pprinter > /dev/null
  if [[ $? -eq $2 ]]; then
    good "$1" "parse"
  else
    bad "$1" "parse"
  fi
}

check_compile() {
  output=$(diff "$1bc" <(./bin/bccompiler < "$1"))
  if [[ -z $output ]]; then
    good "$1" "compile"
  else
    bad "$1" "compile"
    echo "$output"
  fi
}

check_interpret() {
  output=$(diff "$1.out" <(./bin/interpreter "$1"))
  if [[ -z $output ]]; then
    good "$1" "interpret"
  else
    bad "$1" "interpret"
    echo "$output"
  fi
}

check_interpret_vm_s() {
  output=$(diff "$1.out" <(./bin/vm -s "$1"))
  if [[ -z $output ]]; then
    good "$1" "interpret_vm_s"
  else
    bad "$1" "interpret_vm_s"
    echo "$output"
  fi
}

check_interpret_vm_b() {
  output=$(diff "$1.out" <(./bin/vm -b "$1bc"))
  if [[ -z $output ]]; then
    good "$1" "interpret_vm_b"
  else
    bad "$1" "interpret_vm_b"
    echo "$output"
  fi
}

check_good_parse() {
  check_parse $1 0
}

check_bad_parse() {
  check_parse $1 1
}

make pprinter bccompiler interpreter vm

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
  check_interpret_vm_s "$f"
done

for f in tests/bytecodetest*.mit
do
  check_interpret "$f"
  check_compile "$f"
  check_interpret_vm_s "$f"
  check_interpret_vm_b "$f"
done

for f in tests/staff/test*.mit
do
  check_interpret "$f"
  check_interpret_vm_s "$f"
done
