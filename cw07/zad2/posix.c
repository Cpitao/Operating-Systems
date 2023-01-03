#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "common.h"


#ifndef COOKS
#define COOKS 4
#endif

#ifndef DELIVERIES
#define DELIVERIES 4
#endif

int init_semaphores(int count, char id, int type);
int init_shm(size_t size, char id);
void kill_cooks(void);
void kill_deliveries(void);
void signal_handler(int sig);
void safe_exit();

sem_t *in_oven, *oven_active, *table_free, *on_table, *table_active;
int cooks[COOKS], deliveries[DELIVERIES];

int main(void)
{
    for (int i=0; i < COOKS; i++) cooks[i] = -1;
    for (int i=0; i < DELIVERIES; i++) deliveries[i] = -1;

    if (atexit(safe_exit))
    {
        perror("atexit");
        return 1;
    }
    signal(2, signal_handler);
    
    in_oven = sem_open(IN_OVEN, O_CREAT, 0660, OVEN_SIZE);
    oven_active = sem_open(OVEN_ACTIVE, O_CREAT, 0660, 1);
    table_free = sem_open(TABLE_FREE, O_CREAT, 0660, TABLE_SIZE);
    on_table = sem_open(ON_TABLE, O_CREAT, 0660, 0);
    table_active = sem_open(TABLE_ACTIVE, O_CREAT, 0660, 1);

    if (in_oven == SEM_FAILED || oven_active == SEM_FAILED || table_free == SEM_FAILED || 
        on_table == SEM_FAILED || table_active == SEM_FAILED)
    {
        perror("sem_open");
        return 1;
    }

    int shm_oven = shm_open(SHM_OVEN_NAME, O_CREAT | O_RDWR, 0660);
    if (shm_oven == -1)
    {
        perror("shm_open");
        return 1;
    }
    if (ftruncate(shm_oven, sizeof(shm_oven_t)))
    {
        perror("ftruncate");
        return 1;
    }
    shm_oven_t* shm_ovenp = mmap(NULL, sizeof(shm_oven_t), PROT_WRITE, MAP_SHARED, shm_oven, 0);
    if (shm_ovenp == (void*) -1) return 1;
    shm_ovenp->next_element = 0;
    shm_ovenp->oldest_element = -1;
    munmap(shm_ovenp, sizeof(shm_oven_t));

    int shm_table = shm_open(SHM_TABLE_NAME, O_CREAT | O_RDWR, 0660);
    if (shm_table == -1)
    {
        perror("shm_open");
        return 1;
    }
    if (ftruncate(shm_table, sizeof(shm_table_t)))
    {
        perror("ftruncate");
        return 1;
    }
    shm_table_t* shm_tablep = mmap(NULL, sizeof(shm_table_t), PROT_WRITE, MAP_SHARED, shm_table, 0);
    if (shm_tablep == (void*) -1) return 1;
    shm_tablep->next_element = 0;
    shm_tablep->oldest_element = -1;
    munmap(shm_tablep, sizeof(shm_table_t));

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

void safe_exit()
{
    // kill child cook processes and delivery processes
    kill_cooks();
    kill_deliveries();
    
    // close and remove semaphores
    sem_close(in_oven);
    sem_close(oven_active);
    sem_close(table_free);
    sem_close(on_table);
    sem_close(table_active);
    sem_unlink(IN_OVEN);
    sem_unlink(OVEN_ACTIVE);
    sem_unlink(TABLE_FREE);
    sem_unlink(ON_TABLE);
    sem_unlink(TABLE_ACTIVE);

    // remove shared memory segments
    shm_unlink(SHM_OVEN_NAME);
    shm_unlink(SHM_TABLE_NAME);
}

void signal_handler(int sig) {safe_exit();}

void kill_cooks()
{
    for (int i=0; i < COOKS; i++)
    {
        if (cooks[i] == -1) continue;
        kill(cooks[i], 2);
    }
}

void kill_deliveries(void)
{
    for (int i=0; i < DELIVERIES; i++)
    {
        if (deliveries[i] == -1) continue;
        kill(deliveries[i], 2);
    }
}