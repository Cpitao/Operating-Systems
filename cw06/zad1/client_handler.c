#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include "common.h"

int init_client_queue(int *i)
{
    char *home;
    if ((home = getenv("HOME")) == NULL) {
        fprintf(stderr, "Unable to get HOME var\n");
        return -1;
    }

    FILE* fp;
    char* filename = calloc(10, 1);
    (*i)=2;
    sprintf(filename, "%d", *i);
    while ((fp = fopen(filename, "r")))
    {
        (*i)++;
        sprintf(filename, "%d", *i);
        fclose(fp);
    }
    // create new file to let other clients know that int is taken
    fp = fopen(filename, "w+");
    fclose(fp);
    free(filename);
    // get key for new client queue
    key_t key = ftok(home, *i);

    // create queue
    int q_id = msgget(key, IPC_CREAT | 0660);

    return q_id;
}

int notify_server(int i)
{
    char *home;
    if ((home = getenv("HOME")) == NULL) {
        printf("er");
        fflush(stdout);
        fprintf(stderr, "Unable to get HOME var\n");
        return -1;
    }
    key_t server_key = ftok(home, 1);
    int server_q_id = msgget(server_key, 0600);

    Message_t message;
    message.mtype = INIT;
    sprintf(message.content, "%d", i);
    return msgsnd(server_q_id, &message, MAX_MSG_LEN, 0);
}

int get_id(int q_id)
{
    Message_t message;
    message.content[0] = '\0';
    if (msgrcv(q_id, &message, MAX_MSG_LEN, 10, 0) < 0) return -1;
    return atoi(message.content);
}

void check_messages(int q_id)
{
    Message_t message;
    while (msgrcv(q_id, &message, MAX_MSG_LEN, 0, IPC_NOWAIT) > 0)
    {
        if (message.mtype == 999) exit(0);
        printf("%s\n", message.content);
    }
}

int check_stdin(int assigned_id)
{
    char buffer[255];

    fgets(buffer, 255, stdin);
    if (buffer[0] == '\0') return 0;
    char mode[10];
    int n;
    if(!sscanf(buffer, "%s %n", mode, &n)) return 1;
    int send_mode;
    if (strcmp(mode, "2ONE") == 0) send_mode = TO_ONE;
    else if (strcmp(mode, "LIST") == 0) send_mode = LIST;
    else if (strcmp(mode, "2ALL") == 0) send_mode = TO_ALL;
    else if (strcmp(mode, "STOP") == 0) send_mode = STOP;
    else return 1;

    Message_t message;
    message.mtype = send_mode;
    sprintf(message.content, "%d %s", assigned_id, buffer+n);

    char *home;
    if ((home = getenv("HOME")) == NULL) {
        fprintf(stderr, "Unable to get HOME var\n");
        return -1;
    }
    key_t server_key = ftok(home, 1);
    int server_q = msgget(server_key, 0600);
    
    return msgsnd(server_q, &message, MAX_MSG_LEN, 0);
}
