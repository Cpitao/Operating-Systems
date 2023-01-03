#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "common.h"


#ifndef COOKS
#define COOKS 5
#endif

#ifndef DELIVERIES
#define DELIVERIES 3
#endif

int init_semaphores(int count, char id, int type);
int init_shm(size_t size, char id);
void kill_cooks(void);
void kill_deliveries(void);
void signal_handler(int sig);
void safe_exit();

int oven_sem, table_sem, is_empty_sem;
int oven, table;
int cooks[COOKS], deliveries[DELIVERIES];

int main(void)
{
    if (atexit(safe_exit))
    {
        perror("atexit");
        return 1;
    }
    signal(2, signal_handler);
    /* 
        semaphore 0 - oven/table access,
        semaphore 1 - oven/table free places
    */
    oven_sem = init_semaphores(2, OVEN_ID, 0);
    if (oven_sem == -1)
    {
        perror("init sem");
        return 1;
    }

    table_sem = init_semaphores(3, TABLE_ID, 1);
    if (table_sem == -1)
    {
        perror("init sem");
        return 1;
    }

    oven = init_shm(sizeof(shm_oven_t), OVEN_ID);
    shm_oven_t* oven_at = shmat(oven, NULL, 0);
    if (oven_at == (void*) -1) return -1;
    oven_at->oldest_element = 0;
    oven_at->next_element = 0;
    shmdt(oven_at);

    table = init_shm(sizeof(shm_table_t), TABLE_ID);
    shm_table_t* table_at = shmat(table, NULL, 0);
    if (table_at == (void*) -1) return -1;
    table_at->oldest_element = 0;
    table_at->next_element = 0;
    shmdt(table_at);

    // run all cooks with second delays to get different rand seed on each of them
    for (int i=0; i < COOKS; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp("./cook", "cook", NULL);
        }

        cooks[i] = pid;

        sleep(1); // to have different seed on the next process
    }

    for (int i=0; i < DELIVERIES; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp("./delivery", "delivery", NULL);
        }

        deliveries[i] = pid;

        sleep(1); // different seeds
    }

    for (int i=0; i < COOKS; i++) wait(0);
    for (int i=0; i < DELIVERIES; i++) wait(0);
    
    return 0;
}

int init_semaphores(int count, char id, int type)
{
    char* home = getenv("HOME");
    if (home == NULL) {
        perror("getenv");
        return -1;
    }

    key_t key = ftok(home, id);
    if (key == -1)
    {
        perror("ftok");
        return -1;
    }

    int sem = semget(key, count, IPC_CREAT | 0660);
    if (sem < 0) 
    {
        perror("semget");
        return -1;
    }

    if (count <= 1) return sem;

    union semun {
        int val;
    } arg;
    arg.val = 1;
    if (semctl(sem, 0, SETVAL, arg) == -1)
    {
        perror("semctl");
        return -1;
    }

    if (type == 0) arg.val = OVEN_SIZE;
    else arg.val = TABLE_SIZE;
    
    if (semctl(sem, 1, SETVAL, arg) == -1)
    {
        perror("semctl");
        return -1;
    }

    arg.val = 0;
    for (int i=2; i < count; i++)
    {
        if (semctl(sem, i, SETVAL, arg) == -1)
        {
            perror("semctl");
            return -1;
        }
    }


    return sem;
}

int init_shm(size_t size, char id)
{
    char* home = getenv("HOME");
    if (home == NULL)
    {
        perror("getenv");
        return -1;
    }

    key_t key = ftok(home, id);
    if (key == -1)
    {
        perror("ftok");
        return -1;
    }

    int shm = shmget(key, size, IPC_CREAT | 0660);
    if (shm == -1)
    {
        perror("shmget");
        return -1;
    }

    return shm;
}

void safe_exit()
{
    // kill child cook processes and delivery processes
    kill_cooks();
    kill_deliveries();

    // remove semaphores
    semctl(oven_sem, 0, IPC_RMID);
    semctl(table_sem, 0, IPC_RMID);

    // remove shared memory segments
    shmctl(oven, IPC_RMID, NULL);
    shmctl(table, IPC_RMID, NULL);
}

void signal_handler(int sig) {safe_exit();}

void kill_cooks()
{
    for (int i=0; i < COOKS; i++)
    {
        kill(cooks[i], 2);
    }
}

void kill_deliveries(void)
{
    for (int i=0; i < DELIVERIES; i++)
    {
        kill(deliveries[i], 2);
    }
}
