#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

#define BOARD_SIZE 3
#define MAX_PLAYERS 16

/* board, filled with x, o or ' ' (empty) */ 
typedef struct board {
    char values[BOARD_SIZE][BOARD_SIZE];
} board;

typedef struct Client_t {
    struct sockaddr_in in_src;
    struct sockaddr_un un_src;
    char name[255];
    int type;
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
void sprint_board(char* s, board* b);