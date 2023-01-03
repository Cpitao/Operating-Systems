#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>

int init_client_queue();

int notify_server(int i);

int get_id(int q_id);

void check_messages(int q_id);

int check_stdin(int assigned_id);