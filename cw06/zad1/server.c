#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "server_handler.h"


Clients_t clients;

int parse(Message_t message);
void atexit_func(void);
void sig_handler(int signal) {atexit_func(); exit(0);}

int main(void)
{
    int server_queue = init_server_queue();
    clients.all_clients = (Client_t*) malloc(sizeof(Client_t) * MAX_CLIENTS);
    clients.clients_number = 0;
    for (int i=0; i < MAX_CLIENTS; i++) 
    {
        Client_t new_client;
        new_client.client_id = -1;
        new_client.q_id = -1;
        clients.all_clients[i] = new_client;
    }

    atexit(atexit_func);
    signal(2, sig_handler);

    while (1)
    {
        Message_t message;
        for (int i=STOP; i <= INIT; i++)
        {
            if (msgrcv(server_queue, &message, MAX_MSG_LEN, i, IPC_NOWAIT) > 0)
            {
                if (parse(message) != 0)
                {
                    fprintf(stderr, "Error occurred while handling request\n");
                }
                break;
            }
        }

        sleep(1);
    }

    free(clients.all_clients);
}

int parse(Message_t message)
{
    int from, to, n, i;
    char new_content[MAX_MSG_LEN];

    switch(message.mtype)
    {
        case STOP:
            if (sscanf(message.content, "%d", &from) != 1) return -1;
            return client_stop(clients, clients.all_clients[from]);
        case LIST: 
            if (!sscanf(message.content, "%d %n", &from, &n)) return -1;
            strcpy(new_content, message.content + n);
            strcpy(message.content, new_content);
            log_event(message, clients.all_clients[from]);
            return msg_client_list(clients, clients.all_clients[from]);
        case TO_ONE: 
            if (!sscanf(message.content, "%d %d %n", &from, &to, &n)) return -1;
            strcpy(new_content, message.content + n);
            strcpy(message.content, new_content);
            return msg_to_one(clients.all_clients[from], clients.all_clients[to], message, 1);
        case TO_ALL:
            if (!sscanf(message.content, "%d %n", &from, &n)) return -1;
            strcpy(new_content, message.content + n);
            strcpy(message.content, new_content);
            return msg_to_all(clients.all_clients[from], clients, message);
        case INIT:
            i = atoi(message.content);
            if (i < 0) return -1;

            char *home;
            if ((home = getenv("HOME")) == NULL) {
                fprintf(stderr, "Unable to get HOME var\n");
                return -1;
            }

            key_t key = ftok(home, i);
            if (key < 0) return -1;
            return client_init(clients, key);
    }

    return 1;
}

void atexit_func(void)
{
    char* home = getenv("HOME");
    key_t key = ftok(home, 1);
    int q_id = msgget(key, 0600);
    msgctl(q_id, IPC_RMID, NULL);
    Message_t message;
    message.mtype = 999;
    for (int i=0; i < MAX_CLIENTS; i++)
    {
        if (clients.all_clients[i].client_id != -1)
        {
            msgsnd(clients.all_clients[i].q_id, &message, MAX_MSG_LEN, 0);
        }
    }
}
