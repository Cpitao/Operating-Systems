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

int init_semaphores();
int deliver_pizza(shm_table_t* shm_table);

sem_t *table_free, *on_table, *table_active;

int main(void)
{
    if (init_semaphores())
    {
        perror("init_semaphores");
        return 1;
    }

    int table_id = shm_open(SHM_TABLE_NAME, O_RDWR, 0660);
    if (table_id == -1)
    {
        perror("get_shm table");
        return -1;
    }
    shm_table_t* shm_table = mmap(NULL, sizeof(shm_table_t), PROT_READ | PROT_WRITE, MAP_SHARED, table_id, 0);
    if (shm_table == (void*) -1)
    {
        perror("mmap");
        return -1;
    }
    
    /* this will keep executing until an error occurs or process is sent a signal
    parent should send a signal to this process when it exits/is stopped for example with SIGINT */
    while (1)
    {
        if (deliver_pizza(shm_table))
        {
            perror("deliver_pizza");
            return 1;
        }
    }

    return 0;
}

int init_semaphores()
{
    table_free = sem_open(TABLE_FREE, 0);
    on_table = sem_open(ON_TABLE, 0);
    table_active = sem_open(TABLE_ACTIVE, 0);

    if(table_free == SEM_FAILED || on_table == SEM_FAILED || table_active == SEM_FAILED)
    {
        return -1;
    }

    return 0;
}

int deliver_pizza(shm_table_t* shm_table)
{
    struct timespec ts;

    /* take and deliver pizza */
    
    // wait until the table is not occupied and there is a pizza waiting
    if (sem_wait(on_table) || sem_wait(table_active)) return 1;

    int n = shm_table->table[shm_table->oldest_element];
    shm_table->oldest_element = (shm_table->oldest_element + 1) % TABLE_SIZE;

    int pizzas_on_table = (shm_table->next_element - shm_table->oldest_element + TABLE_SIZE) % TABLE_SIZE;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_YELLOW "(%d %ldms) Taking pizza \"%d\" for delivery. Pizzas on the table: %d" ANSI_RESET "\n", 
            getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n, pizzas_on_table);

    // return semaphore to previous state
    if (sem_post(table_active)) return 1;
    // free one place in free places on table semaphore
    if (sem_post(table_free)) return 1;

    sleep((rand() % 2) + 4);

    // give pizza
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_MAGENTA "(%d %ldms) Giving pizza %d" ANSI_RESET "\n",
            getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n);

    // return
    sleep((rand() % 2) + 4);
    return 0;
}