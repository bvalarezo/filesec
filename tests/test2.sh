#!/bin/bash

echo Test 2 
in_file=$(find "$(pwd)" -name in1.txt)
out_file=$(find "$(pwd)" -name out1.txt)
cmp_file=$(find "$(pwd)" -name cmp1.txt)
pass=$(find "$(pwd)" -name pass.txt)
prog=$(find "$(pwd)" -name filesec)
echo $prog -e $in_file $out_file
$prog -e - -
if [ $? = 1 ] ; then
    echo "Return Code 1; Program should error on this invalid usage"
else
    echo "Return Code $?; Failure!"
fi
echo $prog -e -e -e 
$prog -e -e -e
if [ $? = 1 ] ; then
    echo "Return Code 1; Program should error on this invalid usage"
else
    echo "Return Code $?; Failure!"
fi

