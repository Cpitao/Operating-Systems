#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "client_handler.h"


mqd_t mqd, serverd;
int id;
char* filename;


void atexit_func(void);
void signal_handler(int signal){atexit_func(); exit(0);}
void notification_handler(int signal);
void run_client(void);
void parse_input(char* input);

int main(int argc, char** argv)
{
    filename = argv[0] + 2;
    if (atexit(atexit_func))
    {
        fprintf(stderr, "Unable to set atexit func\n");
        return 1;
    }
    signal(2, signal_handler);
    signal(SIGUSR1, notification_handler);

    mqd = init_client_queue(filename);
    notify_server(filename);
    id = get_id(mqd);

    serverd = mq_open("/server_queue", O_WRONLY);

    run_client();
}

void run_client(void)
{
    struct sigevent se;
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGUSR1;
    se.sigev_value.sival_int = SIGUSR1;

    mq_notify(mqd, &se);
    while (1)
    {
        Message_t message = calloc(MAX_MSG_LEN, 1);
        char* input = calloc(MAX_MSG_LEN, 1);
        fgets(input, MAX_MSG_LEN, stdin);

        if (input[0] == '\0') 
        {
            free(message);
            free(input);
            continue;
        }
        
        parse_input(input);
        free(message);
        free(input);
    }
}

void notification_handler(int signal)
{
    Message_t message = calloc(MAX_MSG_LEN, 1);
    mq_receive(mqd, message, MAX_MSG_LEN, NULL);

    char* is_stop = calloc(MAX_MSG_LEN, 1);
    strncpy(is_stop, message, 4);

    if (strcmp(is_stop, "STOP") == 0)
    {
        free(is_stop);
        free(message);
        exit(0);
    }
    printf("--------------------\n");
    printf("%s", message);
    printf("--------------------\n");
    free(message);
    free(is_stop);

    struct sigevent se;
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGUSR1;
    se.sigev_value.sival_int = SIGUSR1;

    mq_notify(mqd, &se);
}

void parse_input(char* input)
{
    char mode[10];

    strncpy(mode, input, 4);
    if (strcmp(mode, "2ONE") == 0)
    {
        Message_t message = calloc(MAX_MSG_LEN, 1);
        int to, n;
        sscanf(input, "2ONE %d %n", &to, &n);
        sprintf(message, "1 %d %d %s", id, to, input + n);

        mq_send(serverd, message, strlen(message), TO_ONE);
        free(message);
    }
    else if (strcmp(mode, "2ALL") == 0)
    {
        int n;
        sscanf(input, "2ALL %n", &n);
        Message_t message = calloc(MAX_MSG_LEN, 1);
        sprintf(message, "A %d %s", id, input + n);

        mq_send(serverd, message, strlen(message), TO_ALL);
        free(message);
    }
    else if (strcmp(mode, "STOP") == 0)
    {
        exit(0);
    }
    else if (strcmp(mode, "LIST") == 0)
    {
        Message_t message = calloc(MAX_MSG_LEN, 1);
        sprintf(message, "L %d", id);

        mq_send(serverd, message, strlen(message), LIST);
        free(message);
    }
}

void atexit_func(void)
{
    Message_t message = calloc(256, 1);
    sprintf(message, "S %d", id);
    mq_send(serverd, message, strlen(message), STOP);
    free(message);
    mq_close(serverd);
    mq_close(mqd);

    char* queue_name = calloc(255, 1);
    sprintf(queue_name, "/%s%d", filename, getpid());
    mq_unlink(queue_name);
    free(queue_name);
}