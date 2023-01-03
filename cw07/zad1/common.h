#define OVEN_SIZE 5
#define TABLE_SIZE 5

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

const char TABLE_ID = 'T';
const char OVEN_ID = 'I';