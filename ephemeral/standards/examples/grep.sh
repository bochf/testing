#!/bin/ksh93

# Simple case of detecting and handling multiple lines.
# Replaces grep '.*pattern.*' $input_file 

typeset input_file=lorem_ipsum

typeset line

while IFS='' read line
do
    if [[ "${line}" == *ullamco* ]]
    then
        print -- "${line}"
    fi
done < "$input_file"


