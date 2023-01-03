#include <mqueue.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define MAX_MSG_LEN 256

typedef enum {
    STOP = 1,
    LIST = 2,
    TO_ONE = 3,
    TO_ALL = 4,
    INIT = 5} Event_t;

typedef struct Client_t {
    int client_id;
    mqd_t q;
} Client_t;

typedef struct Clients_t {
    Client_t all_clients[MAX_CLIENTS];
    int clients_number;
} Clients_t;

typedef char* Message_t;
