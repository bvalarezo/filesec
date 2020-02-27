#!/bin/bash

echo Test 1
in_file=$(find "$(pwd)" -name in1.txt)
out_file=$(find "$(pwd)" -name out1.txt)
cmp_file=$(find "$(pwd)" -name cmp1.txt)
pass=$(find "$(pwd)" -name pass.txt)
prog=$(find "$(pwd)" -name filesec)
echo $prog -e -p $pass $in_file $out_file
$prog -e -p $pass $in_file $out_file
if [ $? = 0 ] ; then
    echo "Return Code 0; Success"
else
    echo "Return Code $?; Failure!"
fi
echo $prog -d -p $pass $out_file $cmp_file
$prog -d -p $pass $out_file $cmp_file
if [ $? = 0 ] ; then
    echo "Return Code 0; Success!"
else
    echo "Return Code $?; Failure!"
fi

result=$(diff -y --brief $cmp_file $in_file)

if [ $? -eq 0 ]
then
        echo "Pass, encryption/decrytion works"
else
        echo "Fail, files are different"
        echo "$result"
fi
