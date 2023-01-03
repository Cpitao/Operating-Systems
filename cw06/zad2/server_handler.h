#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "common.h"


mqd_t init_server_queue();

int client_init(Clients_t *clients, char* name);

int msg_to_one(Client_t sender, Client_t recipient, Message_t message, int log);

int msg_to_all(Client_t sender, Clients_t clients, Message_t message);

int msg_client_list(Clients_t clients, Client_t receiver);

int client_stop(Clients_t *clients, Client_t client);

int send_end(Client_t client);

int log_event(Event_t event, Message_t message, Client_t client);