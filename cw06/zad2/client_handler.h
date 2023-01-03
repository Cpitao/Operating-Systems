#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "common.h"


int init_client_queue(char* filename);

int notify_server(char* filename);

int get_id(mqd_t mqd);
