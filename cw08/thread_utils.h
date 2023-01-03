#include <pthread.h>

typedef struct thread_data {
    pgma_t *in_pgma, *out_pgma; // output pgma
    int min_val, max_val; // min/max pixel values for thread if in numbers mode
    int min_col, max_col; // min/max column numbers if in blocks mode
} thread_data;

