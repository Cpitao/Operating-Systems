#include <stdio.h>
#include <stdlib.h>

typedef struct pgma_t {
    int width, height;
    int M;
    int **pixels;
} pgma_t;

/* save pgma to passed address and return 0 on success */
int read_int(FILE* fp);

int load_pgma(FILE* fp, pgma_t *pgma);

int save_pgma(FILE* fp, pgma_t *pgma);

void free_pgma(pgma_t *pgma);