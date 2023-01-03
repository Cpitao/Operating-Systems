CFLAGS=

all: zad2_static zad2_shared zad2_dynamic

# Static
zad2_static: libzad1_stat.a zad2.o
	gcc $(CFLAGS) zad2.o libzad1_stat.a -o zad2_static

zad2.o:
	gcc -c ../zad2/zad2.c

libzad1_stat.a:
	gcc $(CFLAGS) -c ../zad1/zad1.c -o zad1_stat.o
	ar rcs libzad1_stat.a zad1_stat.o

# Shared
zad2_shared: libzad1_shared.so
	gcc $(CFLAGS) ../zad2/zad2.c -Wl,-rpath=. -lzad1_shared -L ./ -o zad2_shared

libzad1_shared.so: zad1_shared.o
	gcc $(CFLAGS) -fPIC -shared zad1_shared.o -o libzad1_shared.so

zad1_shared.o:
	gcc $(CFLAGS) -c -fPIC ../zad1/zad1.c -o zad1_shared.o
	
zad2_dynamic: libzad1_shared.so
	gcc $(CFLAGS) ../zad2/zad2.c -Wl,-rpath=. -lzad1_shared -ldl -L ./ -o zad2_dynamic -DDYNAMIC

test_a: all generate_files
	./test_zad3a.sh raport3a.txt ./zad2_static
	./test_zad3a.sh raport3a.txt ./zad2_shared
	./test_zad3a.sh raport3a.txt ./zad2_dynamic
	make clean

test_b:
	for opt in -O -O1 -O2 -O3 -O0 -Os ; do\
		make generate_files;\
		make CFLAGS=$$opt;\
		./test_zad3b.sh raport3b.txt ./zad2_static $$opt;\
		./test_zad3b.sh raport3b.txt ./zad2_shared $$opt;\
		./test_zad3b.sh raport3b.txt ./zad2_dynamic $$opt;\
		make clean;\
	done

generate_files:
	gcc file_generator.c -o generate_files

clean:
	rm zad* lib* generate_files
