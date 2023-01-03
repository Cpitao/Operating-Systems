#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "server_handler.h"


const char LOG_FILE[] = "logs.txt";

mqd_t init_server_queue()
{
    struct mq_attr mqa;
    mqa.mq_flags = 0;
    mqa.mq_maxmsg = 10;
    mqa.mq_msgsize = MAX_MSG_LEN;
    mqa.mq_curmsgs = 0;

    const char* name = "/server_queue";
    mqd_t mqd = mq_open(name, O_CREAT | O_RDONLY, 0660, &mqa);

    return mqd;
}

int log_event(Event_t event, Message_t message, Client_t client)
{
    FILE* fp = fopen(LOG_FILE, "a+");
    if (fp == NULL) return -1;
    int success = 0;
    char* new_log = calloc(500, 1);
    time_t raw;
    struct tm* timeinfo;
    time(&raw);
    timeinfo = localtime(&raw);

    switch(event)
    {
        case INIT:
            sprintf(new_log, "Client named %s (id %d) sent INIT at %s---------------", message, 
                    client.client_id, asctime(timeinfo));
            success = fprintf(fp, "%s\n", new_log) < 0;
            break;
        case TO_ONE:
            sprintf(new_log, "Client %d 2ONE at %s%s\n---------------", client.client_id, asctime(timeinfo),
                    message);
            success = fprintf(fp, "%s\n", new_log) < 0;
            break;
        case TO_ALL:
            sprintf(new_log, "Client %d 2ALL at %s%s\n---------------", client.client_id, asctime(timeinfo),
                    message);
            success = fprintf(fp, "%s\n", new_log) < 0;
            break;
        case LIST:
            sprintf(new_log, "Client %d LIST at %s---------------", client.client_id, asctime(timeinfo));
            success = fprintf(fp, "%s\n", new_log) < 0;
            break;
        case STOP:
            sprintf(new_log, "Client %d STOP at %s---------------", client.client_id, asctime(timeinfo));
            success = fprintf(fp, "%s\n",  new_log) < 0;
            break;
    }
    free(new_log);
    return success;
}

int client_init(Clients_t *clients, char* name)
{
    for (int i=0; i < MAX_CLIENTS; i++)
    {
        // if position is free - send this id to the client
        if ((*clients).all_clients[i].client_id == -1)
        {
            mqd_t mqd = mq_open(name, O_WRONLY);

            Message_t message = calloc(MAX_MSG_LEN, 1);
            sprintf(message, "I %d", i);

            if (mq_send(mqd, message, strlen(message), 0))
            {
                fprintf(stderr, "Unable to communicate with new client\n");
                perror("mq_send");
                free(message);
                return -1;
            }

            (*clients).all_clients[i].client_id = i;
            (*clients).all_clients[i].q = mqd;

            log_event(INIT, name, (*clients).all_clients[i]);
            free(message);
            (*clients).clients_number++;
            return 0;
        }
    }

    return -1;
}

int msg_to_one(Client_t sender, Client_t recipient, Message_t message, int do_log)
{
    int success = mq_send(recipient.q, message, strlen(message), TO_ONE);

    if (do_log) success = success | log_event(TO_ONE, message, sender);

    return success;
}

int msg_to_all(Client_t sender, Clients_t clients, Message_t message)
{
    int success = log_event(TO_ALL, message, sender);

    for (int i=0; i < MAX_CLIENTS; i++)
    {
        if (clients.all_clients[i].client_id == -1) continue;
        success = success | msg_to_one(sender, clients.all_clients[i], message, 0);
    }

    return success;
}

int msg_client_list(Clients_t clients, Client_t receiver)
{
    int success = log_event(LIST, NULL, receiver);

    Message_t message = calloc(MAX_MSG_LEN, 1);
    for (int i=0; i < MAX_CLIENTS; i++)
    {
        if (clients.all_clients[i].client_id == -1) continue;

        char new_part[23];
        sprintf(new_part, "%d\n", clients.all_clients[i].client_id);

        strcat(message, new_part);
    }

    success = mq_send(receiver.q, message, strlen(message), LIST) | success;
    free(message);

    return success;
}

int client_stop(Clients_t *clients, Client_t client)
{
    log_event(STOP, NULL, client);
    if (mq_close(client.q))
    {
        fprintf(stderr, "Error closing queue for client %d", client.client_id);
        return -1;
    }

    (*clients).all_clients[client.client_id].client_id = -1;
    (*clients).all_clients[client.client_id].q = -1;
    (*clients).clients_number--;

    return 0;
}

int send_end(Client_t client)
{
    Message_t message = calloc(10, 1);
    sprintf(message, "STOP ");
    
    int success = mq_send(client.q, message, strlen(message), STOP) | mq_close(client.q);

    free(message);
    return success;
}