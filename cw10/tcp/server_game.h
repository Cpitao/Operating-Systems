#include "common.h"

#define MAX_PLAYERS 16

typedef struct Client_t {
    int fd;
    struct sockaddr_in client;
    char name[255];
} Client_t;

typedef struct Game_t {
    int player1, player2;
    char assigned1, assigned2;
    board b;
    int next_move;
} Game_t;

void init_board(board* b);
int make_move(board* b, int i, char c);
int check_win(board* b, int move);