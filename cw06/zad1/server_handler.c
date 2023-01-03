#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>


#include "server_handler.h"

const char LOG_FILE[] = "logs.txt";

int init_server_queue()
{
    char *home;
    if ((home = getenv("HOME")) == NULL) {
        fprintf(stderr, "Unable to get HOME var\n");
        return -1;
    }

    // get key for new server queue
    key_t key = ftok(home, 1);

    // create queue
    int q_id = msgget(key, IPC_CREAT | 0660);

    return q_id;
}

char* get_event_name(Event_t event)
{
    switch(event)
    {
        case STOP: return "STOP";
        case LIST: return "LIST";
        case TO_ONE: return "2ONE";
        case TO_ALL: return "2ALL";
        case INIT: return "INIT";
    }

    return "";
}

int log_event(Message_t message, Client_t client)
{
    FILE* fp = fopen(LOG_FILE, "a+");
    if (fp == NULL)
    {
        return -1;
    }

    time_t raw;
    struct tm* timeinfo;
    time(&raw);
    timeinfo = localtime(&raw);
    // log time
    if (fputs(asctime(timeinfo), fp) < 0) {
        fclose(fp);
        return -1;
    }
    char client_id[25] = "";
    sprintf(client_id, "Sent by: %d\n", client.client_id);
    if (fputs(client_id, fp) < 0)
    {
        fputs("Error writing client name\n", fp);
        fclose(fp);
        return -1;
    }

    if (fputs(get_event_name(message.mtype), fp) < 0)
    {
        fputs("Error writing event type\n", fp);
        fclose(fp);
        return -1;
    }
    fputs(" ", fp);
    // log message
    if (fputs(message.content, fp) < 0) {
        fputs("Error writing message\n", fp);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

int client_init(Clients_t clients, key_t key)
{
    printf("Got INIT request\n");
    for (int i=1; i < MAX_CLIENTS; i++)
    {
        if (clients.all_clients[i].client_id == -1)
        {
            Client_t client;
            client.client_id = i;
            client.q_id = msgget(key, 0660);

            clients.all_clients[i] = client;

            // send message to the new client with his ID
            Message_t message;
            sprintf(message.content, "%d", i);
            message.mtype = 10;

            return msgsnd(client.q_id, &message, MAX_MSG_LEN, 0);
        }
    }
    // send failure message
    Message_t message;
    message.mtype = 10;
    sprintf(message.content, "-1");
    int q_id = msgget(key, 0660);
    msgsnd(q_id, &message, MAX_MSG_LEN, 0);
    return -1;
}

int msg_to_one(Client_t sender, Client_t recipient, Message_t message, int do_log)
{
    int success;
    if (do_log) success = log_event(message, sender);
    time_t raw;
    struct tm* timeinfo;
    time(&raw);
    timeinfo = localtime(&raw);
    char new_string[2 * MAX_MSG_LEN];
    sprintf(new_string, "FROM: %d at %s%s",
            sender.client_id, asctime(timeinfo), message.content);
    Message_t new_message;
    new_message.mtype = recipient.client_id;
    strcpy(new_message.content, new_string);

    success = success | msgsnd(recipient.q_id, &new_message, MAX_MSG_LEN, 0);
    return success;
}

int msg_to_all(Client_t sender, Clients_t clients, Message_t message)
{
    int success = 0;
    success = success | log_event(message, sender);
    for (int i=0; i < MAX_CLIENTS; i++)
    {
        if (clients.all_clients[i].client_id == -1) continue;
        success = success | msg_to_one(sender, clients.all_clients[i], message, 0);
    }

    return success;
}

int msg_client_list(Clients_t clients, Client_t receiver)
{
    Message_t message;
    message.mtype = receiver.client_id;
    message.content[0] = '\0';
    for (int i=1; i < MAX_CLIENTS; i++)
    {
        if (clients.all_clients[i].client_id == -1) continue;

        char client_id[10] = "";
        sprintf(client_id, "%d", clients.all_clients[i].client_id);
        strcat(message.content, client_id);
        strcat(message.content, "\n");
    }

    return msgsnd(receiver.q_id, &message, MAX_MSG_LEN, 0);
}

int client_stop(Clients_t clients, Client_t client)
{
    clients.all_clients[client.client_id].client_id = -1;
    clients.all_clients[client.client_id].q_id = -1;

    clients.clients_number--;
    return 0;
}