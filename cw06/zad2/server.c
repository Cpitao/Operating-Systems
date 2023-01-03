#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "server_handler.h"


Clients_t clients;
mqd_t server_queue;

int parse(Message_t message);
void run_server();
void atexit_func(void);
void sig_handler(int signal) {atexit_func(); exit(0);}

int main(void)
{
    atexit(atexit_func);
    signal(2, sig_handler);
    for (int i=0; i < MAX_CLIENTS; i++)
    {
        clients.all_clients[i].client_id = -1;
        clients.all_clients[i].q = -1;
    }
    server_queue = init_server_queue();
    

    run_server();

    return 0;
}

int parse(Message_t message)
{
    int success = 0;
    Message_t cpy = calloc(MAX_MSG_LEN, 1);
    int from, to, n;
    char mode;

    time_t raw;
    struct tm* timeinfo;
    time(&raw);
    timeinfo = localtime(&raw);
    switch(message[0])
    {
        case '1':
            sscanf(message, "%c %d %d %n", &mode, &from, &to, &n);
            sprintf(cpy, "From %d at %s%s", from, asctime(timeinfo), message + n);
            success = success | msg_to_one(clients.all_clients[from], clients.all_clients[to],
                              cpy, 1);
            free(cpy);
            return success;
        case 'A':
            sscanf(message, "%c %d %n", &mode, &from, &n);
            sprintf(cpy, "From %d at %s%s", from, asctime(timeinfo), message + n);
            success = success | msg_to_all(clients.all_clients[from], clients, cpy);
            free(cpy);
            return success;
        case 'L':
            sscanf(message, "%c %d", &mode, &from);
            success = success | msg_client_list(clients, clients.all_clients[from]);
            free(cpy);
            return success;
        case 'S':
            sscanf(message, "%c %d", &mode, &from);
            success = success | client_stop(&clients, clients.all_clients[from]);
            free(cpy);
            return success;
        case 'I':
            sscanf(message, "%c %n", &mode, &n);
            strcpy(cpy, message + n);
            success = success | client_init(&clients, cpy);
            free(cpy);
            return success;
    }

    free(cpy);
    return -1;
}

void run_server()
{
    unsigned int prio;
    while (1)
    {
        Message_t message = calloc(MAX_MSG_LEN, 1);
        if (mq_receive(server_queue, message, MAX_MSG_LEN, &prio) < 0)
        {
            fprintf(stderr, "Error receiving message\n");
            continue;
        }

        parse(message);
        free(message);
    }
}

void atexit_func(void)
{

    /* send end message to all clients */
    for (int i=0; i < MAX_CLIENTS; i++)
    {
        if (clients.all_clients[i].client_id == -1) continue;

        send_end(clients.all_clients[i]);
    }
    
    mq_close(server_queue);
    mq_unlink("/server_queue");
}