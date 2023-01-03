#!/bin/bash

if (( $# != 1 )); then
	echo "Illegal number of arguments"
	exit;
fi

if [[ ! -w $1 && -f $1 ]]; then
	echo "Cannot write to specified file"
	exit;
fi

echo "Generating test files..."
./generate_files {1..10} {20..100..10} {1000..15000..1000}
echo "Testing..."
./zad2 -s 40 -f {1..10}.txt {20..100..10}.txt -r 0 -r 1 -r 2 -r 3 -r 4 -f {1000..15000..1000}.txt --nolog >> $1
echo "Cleaning up..."
rm {1..10}.txt* {20..100..10}.txt* {1000..15000..1000}.txt*
echo "Saved results to "$1
