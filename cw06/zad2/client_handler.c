#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "client_handler.h"


int init_client_queue(char* filename)
{
    struct mq_attr mqa;
    mqa.mq_flags = 0;
    mqa.mq_maxmsg = 10;
    mqa.mq_msgsize = MAX_MSG_LEN;
    mqa.mq_curmsgs = 0;

    pid_t pid = getpid();
    char *queue_name = calloc(strlen(filename) + 15, 1);

    sprintf(queue_name, "/%s%d", filename, pid);
    int mqd = mq_open(queue_name, O_CREAT | O_RDONLY, 0660, &mqa);

    free(queue_name);
    return mqd;
}

int notify_server(char* filename)
{
    pid_t pid = getpid();
    char *queue_name = calloc(strlen(filename) + 15, 1);

    sprintf(queue_name, "/%s%d", filename, pid);
    int server_mqd = mq_open("/server_queue", O_WRONLY);

    Message_t message = calloc(strlen(queue_name) + 10, 1);
    sprintf(message, "I %s", queue_name);

    int success = mq_send(server_mqd, message, strlen(message), INIT);
    free(queue_name);
    mq_close(server_mqd);

    return success;
}

int get_id(mqd_t mqd)
{
    int id;
    Message_t message = calloc(257, 1);
    mq_receive(mqd, message, 257, NULL);

    sscanf(message, "I %d", &id);
    printf("Got ID from the server: %d\n", id);
    return id;
}
