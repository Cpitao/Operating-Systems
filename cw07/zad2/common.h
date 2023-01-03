#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

// shared memory names
#define SHM_OVEN_NAME "/shm_oven"
#define SHM_TABLE_NAME "/shm_table"

#define OVEN_SIZE 5
#define TABLE_SIZE 5

// semaphore names
const char* OVEN_ACTIVE = "/oven_active";
const char* IN_OVEN = "/in_oven";
const char* TABLE_FREE = "/table_free";
const char* ON_TABLE = "/on_table";
const char* TABLE_ACTIVE = "/table_active";

typedef struct shm_oven_t {
    int oven[OVEN_SIZE];
    int next_element;
    int oldest_element;
} shm_oven_t;

typedef struct shm_table_t {
    int table[TABLE_SIZE];
    int next_element;
    int oldest_element;
} shm_table_t;
