#!/bin/sh
#
# this script generates c code, to call every test method in a file
#

file=$1

#
# find all static functions and strip them from anything but name
#
#       static void options_log_allowed(void)
#
# will result in
#
#       options_log_allowed
#
funs=`grep -e "static void .*\(void\)" $file | cut -d' ' -f3 | rev | cut -c7- | rev`

#
# add mt_run(); to every function and print result
#
while read -r line
do
    echo "mt_run($line);"
done <<< "$funs"
