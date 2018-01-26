#!/bin/bash

options=(--disable-out-file \
         --disable-out-stderr \
         --disable-timestamp \
         --disable-realtime \
         --disable-monotonic \
         --disable-finfo \
         --disable-colors)

iterations=$((2**${#options[@]} - 1))
for i in `seq 0 1 $iterations`
do
    opts=
    flags=`echo "obase=2;$i" | bc | rev`
    for j in `seq 1 1 ${#flags}`
    do
        if [ "${flags:j-1:1}" == "1" ]
        then
            opts+="${options[j - 1]} "
        fi
    done

    echo -n "[$i/$iterations] ${opts}... "
    echo -n "clean... "
    clean_output=`make clean 2>&1`

    if [ $? -ne 0 ]
    then
        echo "failed"
        echo "$clean_output"
        exit 1
    fi

    echo -n "configure... "
    configure_output=`./configure $opts -C 2>&1`

    if [ $? -ne 0 ]
    then
        echo "failed"
        echo "$configure_output"
        exit 1
    fi

    echo -n "make... "
    make_output=`make -j 2>&1`

    if [ $? -ne 0 ]
    then
        echo "failed"
        echo "$make_output"
        exit 1
    fi

    echo -n "tests... "
    tests_output=`make check 2>&1`

    if [ $? -ne 0 ]
    then
        echo "failed"
        echo "$tests_output"
        exit 1
    fi
    echo ""
done
