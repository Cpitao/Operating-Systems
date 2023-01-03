#include "server_game.h"

void init_board(board* b)
{
    for (int i=0; i < BOARD_SIZE; i++)
    {
        for (int j=0; j < BOARD_SIZE; j++)
            b->values[i][j] = ' ';
    }
}

int make_move(board* b, int i, char c)
{
    if (b == NULL) return 1;
    /* if the field is already taken */
    if (b->values[i / BOARD_SIZE][i % BOARD_SIZE] != ' ') return -1;

    b->values[i / BOARD_SIZE][i % BOARD_SIZE] = c;

    return 0;
}

/* checks win conditions after a move is made
   knowing what move was made allows to check less conditions */
int check_win(board* b, int move)
{
    int f1=1, f2=1, f3=0, f4=0;
    // column / row
    for (int i=0; i < BOARD_SIZE-1; i++)
    {
        if (b->values[i][move % BOARD_SIZE] != b->values[i+1][move % BOARD_SIZE]) f1 = 0;
        if (b->values[move / BOARD_SIZE][i] != b->values[move / BOARD_SIZE][i + 1]) f2 = 0;
    }

    // main diagonal
    if (move % BOARD_SIZE == move / BOARD_SIZE)
    {
        f3 = 1;
        for (int i=0; i < BOARD_SIZE-1; i++)
        {
            if (b->values[i][i] != b->values[i+1][i+1]) 
            {
                f3 = 0;
                break;
            }
        }
    }

    // backwards diagonal
    if (move % BOARD_SIZE == BOARD_SIZE - move / BOARD_SIZE - 1)
    {
        f4 = 1;
        for (int i=0; i < BOARD_SIZE-1; i++)
        {
            if (b->values[i][BOARD_SIZE - i - 1] != b->values[i+1][BOARD_SIZE - i - 2])
            {
                f4 = 0;
                break;
            }
        }
    }
    
    int board_full = 2;
    for (int i=0; i < BOARD_SIZE; i++)
    {
        for (int j=0; j < BOARD_SIZE; j++)
        {
            if (b->values[i][j] == ' ') 
            {
                board_full = 0;
                break;
            }
        }
    }
    return f1 | f2 | f3 | f4 | board_full;
}
