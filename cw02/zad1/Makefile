all: lib sys
lib:
	gcc -Wall -c zad1.c -DLIB -o zad1_lib.o
	gcc -Wall zad1_lib.o -o zad1_lib
sys:
	gcc -Wall -c zad1.c -o zad1_sys.o
	gcc -Wall zad1_sys.o -o zad1_sys

clean:
	rm zad1_lib.o zad1_sys.o zad1_lib zad1_sys

test: lib sys
	./tester.sh
	rm out.txt
	make clean
