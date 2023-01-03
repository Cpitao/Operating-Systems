#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

int get_shm(char id);
int get_sem(char id);
int prepare_pizza(int oven_sem, int table_sem, shm_oven_t* shm_oven, shm_table_t* shm_table);

int main(void)
{
    srand(time(NULL));

    int oven_sem = get_sem(OVEN_ID);
    if (oven_sem == -1)
    {
        perror("get_sem oven");
        return -1;
    }

    int table_sem = get_sem(TABLE_ID);
    if (table_sem == -1)
    {
        perror("get_sem table");
        return -1;
    }

    int oven_id = get_shm(OVEN_ID);
    if (oven_id == -1)
    {
        perror("get_shm oven");
        return -1;
    }
    shm_oven_t* shm_oven = (shm_oven_t*) shmat(oven_id, NULL, 0);
    if (shm_oven == (void*) -1)
    {
        perror("shmat");
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
        if (prepare_pizza(oven_sem, table_sem, shm_oven, shm_table)) break;
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

int prepare_pizza(int oven_sem, int table_sem, shm_oven_t* shm_oven, shm_table_t* shm_table)
{
    struct timespec ts;

    // pizza type
    int n = rand() % 10;
    
    // making pizza ...
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_GREEN "(%d %ldms) Preparing pizza: \"%d\"" ANSI_RESET "\n",
            getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n);
    sleep((rand() % 2) + 1);

    // queue for putting in the oven
    struct sembuf ops[3];
    ops[0].sem_num = 0;
    ops[0].sem_op = -1;
    ops[0].sem_flg = SEM_UNDO;
    ops[1].sem_num = 1;
    ops[1].sem_op = -1;
    ops[1].sem_flg = SEM_UNDO;
    ops[2].sem_num = 2;
    ops[2].sem_flg = SEM_UNDO;

    int op = semop(oven_sem, ops, 2);
    if (op)
    {
        perror("semop");
        return -1;
    }
    
    // put in the oven
    shm_oven->oven[shm_oven->next_element] = n;
    shm_oven->next_element = (shm_oven->next_element + 1) % OVEN_SIZE;
    if (shm_oven->oldest_element == -1) shm_oven->oldest_element = 0;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    int in_oven = (shm_oven->next_element - shm_oven->oldest_element + OVEN_SIZE) % OVEN_SIZE;
    if (in_oven == 0) in_oven = OVEN_SIZE; // there can never be 0 pizzas in the oven at the moment of writing this message
    printf(ANSI_RED "(%d %ldms) Putting pizza \"%d\" in the oven. Pizzas in the oven: %d" ANSI_RESET "\n",
        getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n, in_oven);
    
    // free place in the oven queue
    ops[0].sem_op = 1;
    op = semop(oven_sem, ops, 1);
    if (op)
    {
        perror("semop");
        return -1;
    }

    sleep((rand() % 2) + 4);

    // take out of the oven
    ops[0].sem_op = -1;
    ops[1].sem_op = 1;
    op = semop(oven_sem, ops, 2);
    if (op)
    {
        perror("semop");
        return -1;
    }

    n = shm_oven->oven[shm_oven->oldest_element];
    shm_oven->oven[shm_oven->oldest_element] = -1;
    shm_oven->oldest_element = (shm_oven->oldest_element + 1) % OVEN_SIZE;

    // wait until the table is free and active
    ops[0].sem_op = -1;
    ops[1].sem_op = -1;
    ops[2].sem_op = 1;
    op = semop(table_sem, ops, 3);
    if (op)
    {
        perror("semop");
        return -1;
    }

    // put the pizza on the table
    shm_table->table[shm_table->next_element] = n;
    shm_table->next_element = (shm_table->next_element + 1) % TABLE_SIZE;
    int on_table = (shm_table->next_element - shm_table->oldest_element + TABLE_SIZE) % TABLE_SIZE;
    if (on_table == 0) on_table = TABLE_SIZE; // the table can never be empty at this point
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_CYAN "(%d %ldms) Taking out pizza \"%d\". Pizzas in the oven: %d. Pizzas on the table: %d" ANSI_RESET "\n",
        getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n,
        (shm_oven->next_element - shm_oven->oldest_element + OVEN_SIZE) % OVEN_SIZE, // there can never be full pizzas at this point 
        on_table);

    // free the oven (assumed that taking pizza out of the oven and putting it on the table should be atomic)
    // ^ otherwise there might be some incosistency in the output, where we take out a pizza, put some other
    // inside and put the first one on the table (and only then print the output)
    ops[0].sem_op = 1;
    op = semop(oven_sem, ops, 1);
    if (op)
    {
        perror("semop");
        return -1;
    }

    // activate the table back
    ops[0].sem_op = 1;
    op = semop(table_sem, ops, 1);
    
    return 0;
}
