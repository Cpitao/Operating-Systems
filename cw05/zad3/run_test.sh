#!/bin/bash

./producent fifo 3 text.txt 5
./producent fifo 2 text2.txt 5
./konsument fifo out.txt 5
