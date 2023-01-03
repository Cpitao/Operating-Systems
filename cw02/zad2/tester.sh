#!/bin/bash

for i in {1000..20000..1000}
do
	for ((j=1;j<=i;j++))
	do
		echo -e $j"\n" >> $i.txt
	done
	./zad2_lib 1 $i.txt
	./zad2_sys 1 $i.txt
	rm $i.txt
done
