all: libzad1.a libzad1.so

# Static library
libzad1.a: zad1_stat.o
	ar rcs libzad1.a zad1_stat.o

zad1_stat.o:
	gcc -Wall -c zad1.c -o zad1_stat.o

clean_stat:
	rm zad1_stat.o main.o libzad1.a

# Shared library
libzad1.so: zad1_shared.o
	gcc -shared zad1_shared.o -o libzad1.so

zad1_shared.o:
	gcc -Wall -fPIC -c zad1.c -o zad1_shared.o

clean_shared:
	rm zad1_shared.o libzad1.so

clean:
	rm zad1_stat.o zad1_shared.o libzad1.so libzad1.a
