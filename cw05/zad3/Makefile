all: consumer producer

consumer:
	gcc -Wall konsument.c -o konsument

producer:
	gcc -Wall producent.c -o producent

test: all
	./run_test.sh

clean:
	rm producent konsument fifo out.txt
