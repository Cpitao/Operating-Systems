#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

int init_semaphores();
int prepare_pizza(shm_oven_t* shm_oven, shm_table_t* shm_table);

sem_t *in_oven, *oven_active, *table_free, *on_table, *table_active;

int main(void)
{
    srand(time(NULL));

    if (init_semaphores())
    {
        perror("init_semaphores");
        return 1;
    }

    int oven_id = shm_open(SHM_OVEN_NAME, O_RDWR, 0660);
    if (oven_id == -1)
    {
        perror("get_shm oven");
        return -1;
    }
    shm_oven_t* shm_oven = mmap(NULL, sizeof(shm_oven_t), PROT_READ | PROT_WRITE, MAP_SHARED, oven_id, 0);
    if (shm_oven == (void*) -1)
    {
        perror("mmap");
        return -1;
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
        if (prepare_pizza(shm_oven, shm_table)) 
        {
            perror("prepare_pizza");
            break;
        }
    }

    return 0;
}

int init_semaphores()
{
    in_oven = sem_open(IN_OVEN, 0);
    oven_active = sem_open(OVEN_ACTIVE, 0);
    on_table = sem_open(ON_TABLE, 0);
    table_active = sem_open(TABLE_ACTIVE, 0);
    table_free = sem_open(TABLE_FREE, 0);

    if (in_oven == SEM_FAILED || oven_active == SEM_FAILED || on_table == SEM_FAILED || table_active == SEM_FAILED
        || table_free == SEM_FAILED)
    {
        return -1;
    }

    return 0;
}

int prepare_pizza(shm_oven_t* shm_oven, shm_table_t* shm_table)
{
    struct timespec ts;

    // pizza type
    int n = rand() % 10;
    
    // making pizza ...
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_GREEN "(%d %ldms) Preparing pizza: \"%d\"" ANSI_RESET "\n",
            getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n);
    sleep((rand() % 2) + 1);

    /* PUT IN THE OVEN */
    // queue for putting in the oven
    if (sem_wait(in_oven) || sem_wait(oven_active)) return 1;
    if (shm_oven->oldest_element == -1) shm_oven->oldest_element = 0;
    shm_oven->oven[shm_oven->next_element] = n;
    shm_oven->next_element = (shm_oven->next_element + 1) % OVEN_SIZE;

    int pizzas_in_oven = (shm_oven->next_element - shm_oven->oldest_element + OVEN_SIZE) % OVEN_SIZE;
    if (pizzas_in_oven == 0) pizzas_in_oven = OVEN_SIZE;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_RED "(%d %ldms) Putting pizza \"%d\" in the oven. Pizzas in the oven: %d" ANSI_RESET "\n",
        getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n, pizzas_in_oven);

    // free oven
    if(sem_post(oven_active)) return 1;

    sleep((rand() % 2) + 4);

    /* TAKE OUT OF THE OVEN */
    // take out of the oven and put on the table
    if (sem_wait(oven_active)) return 1;

    // take out of the oven
    n = shm_oven->oven[shm_oven->oldest_element];
    shm_oven->oldest_element = (shm_oven->oldest_element + 1) % OVEN_SIZE;
    if (sem_post(oven_active) || sem_post(in_oven)) return 1;

    // put on the table
    if (sem_wait(table_free) || sem_wait(table_active)) return 1;
    if (shm_table->oldest_element == -1) shm_table->oldest_element = 0;
    shm_table->table[shm_table->next_element] = n;
    shm_table->next_element = (shm_table->next_element + 1) % TABLE_SIZE;

    // get number of pizzas in the oven and on the table
    pizzas_in_oven = (shm_oven->next_element - shm_oven->oldest_element + OVEN_SIZE) % OVEN_SIZE;
    int pizzas_on_table = (shm_table->next_element - shm_table->oldest_element + TABLE_SIZE - 1) % TABLE_SIZE;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf(ANSI_CYAN "(%d %ldms) Taking out pizza \"%d\". Pizzas in the oven: %d. Pizzas on the table: %d" ANSI_RESET "\n",
        getpid(), 1000 * ts.tv_sec + (int) (ts.tv_nsec / 1000000), n, pizzas_in_oven, pizzas_on_table);

    // fix semaphores
    if (sem_post(on_table) || sem_post(table_active)) return 1;


    return 0;
}