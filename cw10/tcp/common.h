#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#define BOARD_SIZE 3

/* board, filled with x, o or ' ' (empty) */ 
typedef struct board {
    char values[BOARD_SIZE][BOARD_SIZE];
} board;

void sprint_board(char* s, board* b);