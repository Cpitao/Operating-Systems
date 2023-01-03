#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "common.h"
#include "client_handler.h"


int key_seed, q_id, assigned_id;
pid_t pid;
void atexit_func(void);
void signal_handler(int signal){atexit_func(); exit(0);}

int main(void)
{
    if (atexit(atexit_func))
    {
        fprintf(stderr, "Unable to set atexit func\n");
        return 1;
    }
    signal(2, signal_handler);

    q_id = init_client_queue(&key_seed);
    notify_server(key_seed);
    assigned_id = get_id(q_id);

    if (assigned_id <= 0)
    {
        fprintf(stderr, "Failed to get ID from the server");
        return 1;
    }

    printf("Got id %d\n", assigned_id);

    pid = fork();
    if (fork() == 0)
    {
        while (1)
        {
            check_stdin(assigned_id);
            usleep(1000 * 100);
        }
    }
    while (1)
    {
        check_messages(q_id);
        usleep(1000 * 100);
    }
    
    // while(1)
    // {
    //     check_messages(q_id);
    //     check_stdin(assigned_id);
    //     // if (check_stdin(assigned_id))
    //     // {
    //     //     fprintf(stderr, "Error sending request to server\n");
    //     // }
    //     sleep(0.1);
    // }
    return 0;
}

void atexit_func(void)
{
    char* filename = calloc(10, 1);
    sprintf(filename, "%d", key_seed);
    remove(filename);
    free(filename);

    char *home;
    home = getenv("HOME");

    key_t server_key = ftok(home, 1);
    int server_q = msgget(server_key, 0600);
    Message_t message;
    sprintf(message.content, "%d", assigned_id);
    message.mtype = STOP;
    msgsnd(server_q, &message, MAX_MSG_LEN, 0);
    kill(pid, 2);
    msgctl(q_id, IPC_RMID, NULL);
}