all: server client

server:
	gcc -Wall -I./ -o server server_handler.c server.c -lrt

client:
	gcc -Wall -I./ -o client client_handler.c client.c -lrt

clean:
	rm client server
