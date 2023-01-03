#define MAX_MSG_LEN 254
#define MAX_CLIENTS 10

typedef enum {
    STOP = 1,
    LIST = 2,
    TO_ONE = 3,
    TO_ALL = 4,
    INIT = 5} Event_t;

typedef struct Client_t {
    int client_id;
    int q_id;
} Client_t;

typedef struct Clients_t {
    Client_t* all_clients;
    int clients_number;
} Clients_t;

typedef struct Message_t {
    long mtype;
    char content[MAX_MSG_LEN];
} Message_t;
