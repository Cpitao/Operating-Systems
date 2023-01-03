all: no_nftw nftw
no_nftw:
	gcc -Wall -c zad3.c -o zad3_no_nftw.o -DNONFTW
	gcc -Wall zad3_no_nftw.o -o zad3_no_nftw

nftw:
	gcc -Wall -c zad3.c -o zad3_nftw.o
	gcc -Wall zad3_nftw.o -o zad3_nftw

clean:
	rm zad3_no_nftw.o zad3_no_nftw zad3_nftw.o zad3_nftw
