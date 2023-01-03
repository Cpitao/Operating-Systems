#!/bin/bash

for i in {1000..20000..1000}
do
	for ((j=1;j<=i;j++))
	do
		echo -e $j"\n" >> $i.txt
	done
	./zad1_lib $i.txt out.txt
	./zad1_sys $i.txt out.txt
	rm $i.txt
done
