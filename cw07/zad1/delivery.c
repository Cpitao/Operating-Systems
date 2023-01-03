#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define ANSI_YELLOW "\x1b[33m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_RESET   "\x1b[0m"

int get_shm(char id);
int get_sem(char id);
int deliver_pizza(int table_sem, shm_table_t* shm_table);

int main(void)
{
    int table_sem = get_sem(TABLE_ID);
    if (table_sem == -1)
    {
        perror("get_sem table");
        return -1;
    }

    int table_id = get_shm(TABLE_ID);
    if (table_id == -1)
    {
        perror("get_shm table");
        return -1;
    }
    shm_table_t* shm_table = (shm_table_t*) shmat(table_id, NULL, 0);
    if (shm_table == (void*) -1)
    {
        perror("shmat");
        return -1;
    }
    
    /* this will keep executing until an error occurs or process is sent a signal
    parent should send a signal to this process when it exits/is stopped for example with SIGINT */
    while (1)
    {
        if (deliver_pizza(table_sem, shm_table)) break;
    }

    return 0;
}

int get_shm(char id)
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

    return shmget(key, 0, 0);
}

int get_sem(char id)
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

    return semget(key, 0, 0);
}

int deliver_pizza(int table_sem, shm_table_t* shm_table)
{
    struct timespec ts;

    struct sembuf ops[3];
    ops[0].sem_flg = SEM_UNDO;
    ops[0].sem_num = 0;
    ops[0].sem_op = -1;
    ops[1].sem_flg = SEM_UNDO;
    ops[1].sem_num = 1;
    ops[1].sem_op = 1;
    ops[2].sem_flg = SEM_UNDO;
    ops[2].sem_num = 2;
    ops[2].sem_op = -1;

    // take and deliver pizza
    int op = semop(table_sem, ops, 3);
    if (op)
    {
        perror("semop");
        return -1;
    }

    int n = shm_table->table[shm_table->oldest_element];
    shm_table->table[shm_table->oldest_element] = -1;
    shm_table->oldest_element = (shm_table->oldest_element + 1) % TABLE_SIZE;

    int on_table = (shm_table->next_element - shm_table->oldest_element + TABLE_SIZE) % TABLE_SIZE;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_YELLOW "(%d %ldms) Taking pizza \"%d\" for delivery. Pizzas on the table: %d" ANSI_RESET "\n", 
            getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n, on_table);

    ops[0].sem_op = 1;
    op = semop(table_sem, ops, 1);
    if (op)
    {
        perror("semop");
        return -1;
    }
    sleep((rand() % 2) + 4);

    // give pizza
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_MAGENTA "(%d %ldms) Giving pizza %d" ANSI_RESET "\n",
            getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n);

    // return
    sleep((rand() % 2) + 4);
    return 0;
}
