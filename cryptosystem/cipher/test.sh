#!/bin/bash

for (( i = 1; i < $1; i++ ))
do
	rm input.txt
	base64 /dev/urandom | head -c 1000 > input.txt
	./a.out 123 input.txt out.txt
	python RC4.py 123 out.txt out1.txt
	if cmp -s input.txt out1.txt ;
	then
       		echo "True"
	else
       		echo "False"
	fi
done
