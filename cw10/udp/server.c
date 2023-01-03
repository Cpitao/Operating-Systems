#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include "server_handler.h"

#define USAGE "./server <port> <path>"

void signal_handler(int sig);

Client_t *clients[MAX_PLAYERS];
Game_t *games[MAX_PLAYERS/2];
int running = 1;
int waiting_for_opponent[MAX_PLAYERS];

int main(int argc, char **argv)
{
    signal(SIGINT, signal_handler);
    for (int i=0; i < MAX_PLAYERS; i++) clients[i] = NULL;
    for (int i=0; i < MAX_PLAYERS/2; i++) games[i] = NULL;
    for (int i=0; i < MAX_PLAYERS; i++) waiting_for_opponent[i] = 0;
    if (argc != 3)
    {
        printf(USAGE);
        return 0;
    }

    int port = atoi(argv[1]);
    if (port <= 1024)
    {
        fprintf(stderr, "Port number must be >= 1024\n");
        return 0;
    }

    char* path = argv[2];

    int net_sock = socket(AF_INET, SOCK_DGRAM, 0);
    int uni_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (init_sockets(&net_sock, &uni_sock, port, path))
    {
        perror("init sockets");
        return 0;
    }
    run_server(&net_sock, &uni_sock, &running, clients, games, waiting_for_opponent);



    for (int i=0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL)
        {
            if (clients[i]->type == 0)
                shutdown_client(i, clients, clients[i]->type, net_sock, games);
            else
                shutdown_client(i, clients, clients[i]->type, uni_sock, games);
        }
    }
    for (int i=0; i < MAX_PLAYERS/2; i++) 
    {
        if (games[i] != NULL)
            free(games[i]);
    }
    close(net_sock);
    close(uni_sock);
    unlink(path);
}

void signal_handler(int sig)
{
    running = 0;
}
