#include <string.h>
#include <time.h>

#include "pgm_utils.h"
#include "thread_utils.h"


int div_ceil(int x, int y)
{
    if (x % y != 0) return x / y + 1;
    return x / y;
}
int min(int x, int y) {if (x < y) return x; return y;}

int get_args(int argc, char** argv);
double** run_numbers(pgma_t *in_pgma, pgma_t *out_pgma);
double** run_blocks(pgma_t *in_pgma, pgma_t *out_pgma);
void* thread_numbers(void *in_data);
void* thread_blocks(void *in_data);

int threads, mode;
char *input_name, *output_name;

int main(int argc, char** argv)
{
    if (get_args(argc, argv))
    {
        fprintf(stderr, "Error parsing arguments\n");
        return 1;
    }

    FILE* inp = fopen(input_name, "r");
    if (inp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", input_name);
        return 1;
    }
    FILE* outp = fopen(output_name, "w+");
    if (outp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", output_name);
        return 1;
    }

    pgma_t in_pgma;
    if (load_pgma(inp, &in_pgma))
    {
        fprintf(stderr, "Error loading pgma\n");
        free_pgma(&in_pgma);
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    pgma_t out_pgma;
    out_pgma.height = in_pgma.height;
    out_pgma.width = in_pgma.width;
    out_pgma.M = in_pgma.M;
    out_pgma.pixels = calloc(in_pgma.height, sizeof(int*));
    for (int i=0; i < in_pgma.height; i++)
    {
        out_pgma.pixels[i] = calloc(in_pgma.width, sizeof(int));
    }
    double** times;

    if (mode == 0) times = run_numbers(&in_pgma, &out_pgma);
    else times = run_blocks(&in_pgma, &out_pgma);

    clock_gettime(CLOCK_REALTIME, &end);
    double total_time = (double)(end.tv_sec - start.tv_sec) * 1.0e6 +
                        (double)(end.tv_nsec - start.tv_nsec) / 1000;

    FILE* times_file = fopen("times.txt", "a+");
    if (times_file == NULL)
    {
        fprintf(stderr, "Unable to open times.txt");
    }
    fprintf(times_file, "Mode: %s\nThreads: %d\nTotal time: %ds%dus\n", argv[2], threads, (int) (total_time/1.0e6),
            (int) (total_time - (int)(total_time/1.0e6)*1.0e6));
    for (int i=0; i < threads; i++)
    {
        fprintf(times_file, "Thread #%d: %ds%dus\n", i, (int)(*times[i] / 1.0e6),
                (int)(*times[i] - (int)(*times[i] / 1.0e6)*1.0e6));
        free(times[i]);
    }
    fprintf(times_file, "\n\n");
    free(times);
    fclose(times_file);

    if (save_pgma(outp, &out_pgma)) 
    {
        fprintf(stderr, "Error saving pgma\n");
        return 1;
    }
    free_pgma(&in_pgma);
    free_pgma(&out_pgma);

    fclose(inp);
    fclose(outp);
    return 0;
}

int get_args(int argc, char** argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }

    threads = atoi(argv[1]);
    if (threads <= 0) 
    {
        fprintf(stderr, "Threads number must be a positive integer\n");
        return 1;
    }

    if (strcmp(argv[2], "numbers") == 0) mode = 0;
    else if (strcmp(argv[2], "blocks") == 0) mode = 1;
    else
    {
        fprintf(stderr, "Mode must be \"numbers\" or \"blocks\"");
        return 1;
    }

    input_name = argv[3];
    output_name = argv[4];

    return 0;
}

double** run_numbers(pgma_t *in_pgma, pgma_t *out_pgma)
{
    // split numbers equally - from 0 to (M+1)/threads-1, 255/threads+1 to (255/threads)*2 etc.
    pthread_t* thread_ids = calloc(threads, sizeof(pthread_t));
    thread_data* args = calloc(threads, sizeof(thread_data));

    int n = (in_pgma->M + 1) % threads;
    for (int i=0; i < n; i++)
    {
        args[i].in_pgma = in_pgma;
        args[i].out_pgma = out_pgma;
        args[i].min_val = ((in_pgma->M + 1) / threads + 1) * i;
        args[i].max_val = ((in_pgma->M + 1) / threads + 1) * (i + 1) - 1;

        if (pthread_create(&thread_ids[i], NULL, thread_numbers, &args[i]))
        {
            free(args);
            free(thread_ids);
            fprintf(stderr, "Error starting new thread\n");
            return NULL;
        }
    }

    for (int i=n; i < threads; i++)
    {
        args[i].in_pgma = in_pgma;
        args[i].out_pgma = out_pgma;
        args[i].min_val = ((in_pgma->M + 1) / threads + 1) * n + 
                          ((in_pgma->M + 1) / threads) * (i - n);
        args[i].max_val = args[i].min_val + (in_pgma->M + 1) / threads - 1;

        if (pthread_create(&thread_ids[i], NULL, thread_numbers, &args[i]))
        {
            fprintf(stderr, "Error starting new thread\n");
            return NULL;
        }
    }

    double** times = malloc(threads * sizeof(double));
    for (int i=0; i < threads; i++)
    {
        pthread_join(thread_ids[i], (void**) &times[i]);
    }
    free(args);
    free(thread_ids);
    return times;
}

void* thread_numbers(void* in_data)
{
    // measure time
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    // actual work
    thread_data* data = (thread_data*) in_data;
    for (int i=0; i < data->in_pgma->height; i++)
    {
        for (int j=0; j < data->in_pgma->width; j++)
        {
            if (data->in_pgma->pixels[i][j] <= data->max_val && data->in_pgma->pixels[i][j] >= data->min_val)
                data->out_pgma->pixels[i][j] = (data->in_pgma->M - data->in_pgma->pixels[i][j]);
        }
    }

    // return pointer to measured time (as double)
    clock_gettime(CLOCK_REALTIME, &end);

    double *usp = malloc(sizeof(double));
    if (usp == NULL) return NULL;
    *usp = (double)(end.tv_sec - start.tv_sec) * 1.0e6 +
                (double)(end.tv_nsec - start.tv_nsec) / 1000;

    return usp;
}

double** run_blocks(pgma_t *in_pgma, pgma_t *out_pgma)
{
    pthread_t* thread_ids = calloc(threads, sizeof(pthread_t));
    thread_data* args = calloc(threads, sizeof(thread_data));

    for (int i=0; i < threads; i++)
    {
        args[i].in_pgma = in_pgma;
        args[i].out_pgma = out_pgma;
        args[i].min_col = i * div_ceil(in_pgma->width, threads);
        args[i].max_col = min((i+1) * div_ceil(in_pgma->width, threads) - 1, in_pgma->width - 1);

        if (pthread_create(&thread_ids[i], NULL, thread_blocks, &args[i]))
        {
            free(args);
            free(thread_ids);
            fprintf(stderr, "Error starting new thread\n");
            return NULL;
        }
    }

    double** times = malloc(threads * sizeof(double));
    for (int i=0; i < threads; i++)
    {
        pthread_join(thread_ids[i], (void**) &times[i]);
    }
    free(args);
    free(thread_ids);
    
    return times;
}

void* thread_blocks(void* in_data)
{
    // measure time
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    // actual work

    thread_data* data = (thread_data*) in_data;
    for (int i=data->min_col; i <= data->max_col; i++)
    {
        for (int j=0; j < data->in_pgma->height; j++)
        {
            data->out_pgma->pixels[i][j] = data->in_pgma->M - data->in_pgma->pixels[i][j];
        }
    }

    // return pointer to measured time (as double)
    clock_gettime(CLOCK_REALTIME, &end);

    double *usp = malloc(sizeof(double));
    if (usp == NULL) return NULL;
    *usp = (double)(end.tv_sec - start.tv_sec) * 1.0e6 +
                (double)(end.tv_nsec - start.tv_nsec) / 1000;

    return usp;
}