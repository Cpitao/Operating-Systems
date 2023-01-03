#include <string.h>


#include "pgm_utils.h"

int read_int(FILE* fp)
{
    char c;
    while (fread(&c, 1, 1, fp) == 1)
    {
        if (c != ' ' && c != '\t' && c != '\n') break;
    }
    char* number = calloc(32, 1);
    number[0] = c;
    int num;

    while (fread(&c, 1, 1, fp) == 1)
    {
        if (c == ' ' || c == '\t' || c == '\n') break;
        number[strlen(number)] = c;
    }
    num = atoi(number);
    free(number);
    return num;
}

int load_pgma(FILE* fp, pgma_t *pgma)
{
    char* header_buff = calloc(255, 1);
    if (fread(header_buff, 1, 3, fp) < 3)
    {
        perror("fread");
        return -1;
    }

    if (strcmp(header_buff, "P2\n") != 0)
    {
        free(header_buff);
        fprintf(stderr, "Invalid file format\n");
        return -1;
    }
    free(header_buff);

    int width = read_int(fp);
    if (width <= 0)
    {
        fprintf(stderr, "Invalid width\n");
        return -1;
    }

    pgma->width = width;

    int height = read_int(fp);
    if (height <= 0)
    {
        fprintf(stderr, "Invalid height\n");
        return -1;
    }
    pgma->height = height;
    
    int M = read_int(fp);
    if (M <= 0)
    {
        fprintf(stderr, "Invalid M\n");
        return -1;
    }
    pgma->M = M;
    pgma->pixels = calloc(height, sizeof(int*));
    if (pgma->pixels == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        return -1;
    }
    for (int i=0; i < height; i++)
    {
        pgma->pixels[i] = calloc(width, sizeof(int));
        if (pgma->pixels[i] == NULL)
        {
            fprintf(stderr, "Error allocating memory\n");
            return -1;
        }
    }

    for (int i=0; i < height; i++)
    {
        for (int j=0; j < width; j++)
        {
            pgma->pixels[i][j] = read_int(fp);
            if (pgma->pixels[i][j] < 0)
            {
                fprintf(stderr, "Invalid pixel value\n");
                return -1;
            }
        }
    }

    return 0;
}

int save_pgma(FILE* fp, pgma_t *pgma)
{
    fprintf(fp, "P2\n%d %d\n%d\n", pgma->width, pgma->height, pgma->M);
    for (int i=0; i < pgma->height; i++)
    {
        for (int j=0; j < pgma->width; j++)
        {
            fprintf(fp, "%d ", pgma->pixels[i][j]);
        }
        fprintf(fp, "\n");
    }

    return 0;
}

void free_pgma(pgma_t *pgma)
{
    if (pgma->pixels == NULL) return;

    for (int i=0; i < pgma->height; i++)
    {
        if (pgma->pixels[i] != NULL)
            free(pgma->pixels[i]);
    }

    free(pgma->pixels);
}