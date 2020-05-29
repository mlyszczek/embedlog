#!/bin/sh

make clean
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze -o pvs-studio.log --disableLicenseExpirationCheck
plog-converter -a "GA:1,2;64:1;OP:1,2,3;CS:1;MISRA:1,2" -t tasklist \
    -o report.tasks pvs-studio.log
