#!/bin/bash

echo_commit() {
    git log -1 --pretty=format:"%C(auto,yellow)%h %C(auto,magenta)%an %C(auto,green)%ad %C(auto,reset)%s" --date=short $1
}

run_single_test() {
    TIMEFORMAT='%U'
    exec 3>&1 4>&2
    var=$( { time ./speedtest/$1 ${@:2} ./tests/PerfTests/textproc.mit < ./tests/PerfTests/textproc.input 1>&3 2>&4; } 2>&1 )  # Captures time only.
    exec 3>&- 4>&-
    echo $var
}

run_multiple_tests() {
    COUNT=1
    sum=0
    for i in $(seq 1 $COUNT); do
        result=$(run_single_test $1 ${@:2})
        sum=$(echo $sum+$result | bc -l)
    done
    printf "Time: %.4f\n" $(echo $sum/$COUNT | bc -l)
}

test_commit() {
    echo_commit $1
    run_multiple_tests $1 ${@:2}
}

while read line; do
    COMMIT=$(echo $line | cut -d, -f1);
    ARGS=$(echo $line | cut -d, -f2);
    test_commit $COMMIT $ARGS
done < speedtest/snapshots.txt