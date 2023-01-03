RESULTS="raport2.txt"
CFLAGS=

all: zad2

test: zad2 generate_files
	./test_zad2.sh $(RESULTS) && echo "\nArguments used: $(CFLAGS)\n" >> $(RESULTS)

zad2: libzad1.a zad2.o
	gcc $(CFLAGS) zad2.o -lzad1 -L ../zad1 -o zad2

libzad1.a:
	cd ../zad1/ && make libzad1.a

zad2.o:
	gcc -Wall -c zad2.c

generate_files:
	gcc -Wall file_generator.c -o generate_files

clean:
	rm zad2.o zad2 generate_files
