all: server client

server:
	gcc -Wall -I./ -o server server_handler.c server.c

client:
	gcc -Wall -I./ -o client client_handler.c client.c

clean:
	rm client server
