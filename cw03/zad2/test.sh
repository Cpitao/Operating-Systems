#!/bin/bash

for i in {1..11}
do
	for j in {1..12}
	do
		./zad2 10.0e-$i $j
	done
done
