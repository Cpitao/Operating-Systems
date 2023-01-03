#!/bin/bash

if (( $# != 3 )); then
	echo "Illegal number of arguments"
	exit;
fi

if [[ ! -w $1 && -f $1 ]]; then
	echo "Cannot write to specified file"
	exit;
fi

if [[ ! -f $2 ]]; then
	echo "Cannot find $2"
	exit;
fi

echo "Generating test files..."
./generate_files {10000..15000..1000}
echo "Testing..."
echo "Testing for version $2 and option $3" >> $1
$2 -s 20 -f {10000..15000..1000}.txt -r 0 -r 1 -r 2 -r 3 -r 4 -f {10000..15000..1000}.txt --nolog >> $1
echo "Cleaning up..."
rm {10000..15000..1000}.txt*
echo "Saved results to "$1
