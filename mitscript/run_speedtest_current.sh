#!/bin/bash

echo_commit() {
    git log -1 --pretty=format:"%C(auto,yellow)%h %C(auto,magenta)%an %C(auto,green)%ad %C(auto,reset)%s" --date=short $1
}

run_single_test() {
    TIMEFORMAT='%U'
    exec 3>&1 4>&2
    var=$( { time ./bin/vm --mem 10000 speedtest/benchmark2.mit < speedtest/benchmark2.input 1>&3 2>&4; } 2>&1 )  # Captures time only.
    exec 3>&- 4>&-
    echo $var
}

run_multiple_tests() {
    COUNT=4
    sum=0
    for i in $(seq 1 $COUNT); do
        result=$(run_single_test)
        sum=$(echo $sum+$result | bc -l)
    done
    printf "Time: %.4f\n" $(echo $sum/$COUNT | bc -l)
}

run_multiple_tests
