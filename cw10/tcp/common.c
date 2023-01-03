#include "common.h"

void sprint_board(char* s, board* b)
{
    if (s == NULL) return;
    if (b == NULL) return;
    for (int j=0; j < 2*BOARD_SIZE - 1; j++) strcat(s, "-");
    strcat(s, "\n");
    for (int i=0; i < BOARD_SIZE; i++)
    {
        for (int j=0; j < BOARD_SIZE-1; j++)
        {
            char buffer[5] = "";
            printf("%c", b->values[i][j]);
            if(b->values[i][j] != ' ')
                sprintf(buffer, "%c|", b->values[i][j]);
            else sprintf(buffer, "%d|", i * BOARD_SIZE + j + 1);
            strcat(s, buffer);
        }
        char buffer[5]="";
        if(b->values[i][BOARD_SIZE-1] != ' ')
                sprintf(buffer, "%c\n", b->values[i][BOARD_SIZE - 1]);
            else sprintf(buffer, "%d\n", i * BOARD_SIZE + BOARD_SIZE);
        strcat(s, buffer);
        for (int j=0; j < 2*BOARD_SIZE - 1; j++)
        {
            strcat(s, "-");
        }
        strcat(s, "\n");
    }
    strcat(s, "\n");
}