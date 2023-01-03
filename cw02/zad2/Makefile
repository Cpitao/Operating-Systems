all: lib sys

lib:
	gcc -Wall -c zad2.c -DLIB -o zad2_lib.o
	gcc -Wall zad2_lib.o -o zad2_lib
sys:
	gcc -Wall -c zad2.c -o zad2_sys.o
	gcc -Wall zad2_sys.o -o zad2_sys

clean:
	rm zad2_sys.o zad2_sys zad2_lib.o zad2_lib

test: lib sys
	./tester.sh
