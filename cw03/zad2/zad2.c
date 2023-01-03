#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>


struct timespec start, end;

void start_clock();
void end_clock(int processes, long double width);

long double min(long double x, long double y)
{
	if (x <= y)
		return x;
	return y;
}

int integrate(int i, long double start, long double end, long double width);

int main(int argc, char** argv)
{ 
	int processes;
	long double width;

	if (argc < 3)
	{
		printf("Number of processes: ");
		scanf("%d", &processes);

		printf("Interval width: ");
		scanf("%Lf", &width);
	}
	else
	{
		width = strtold(argv[1], NULL);
		processes = atoi(argv[2]);
	}
	start_clock();
	for (int i=0; i < processes; i++)
	{
		long double start = (long double) i / processes;
		long double end = (long double) (i + 1) / processes;

		pid_t pid = fork();
		if (pid == 0)
		{
			integrate(i, start, end, width);
			return 0;
		}
	}
	for (int i=0; i < processes; i++) wait(0);
	long double *partial_results = (long double*)calloc(processes, sizeof(long double));

	for (int i=0; i < processes; i++)
	{
		char* filename = (char*) calloc(16, 1);
		sprintf(filename, "w%d.txt", i);
		FILE* fp = fopen(filename, "r");

		if (fp == NULL)
		{
			fprintf(stderr, "Unable to open %s\n", filename);
			return -1;
		}
		free(filename);
		fscanf(fp, "%Lf", &partial_results[i]);
		fclose(fp);
	}
	long double result = 0;
	for (int i=0; i < processes; i++)
	{
		result += partial_results[i];
	}
	printf("Pi: %.20Lf\n", result);

	end_clock(processes, width);

	return 0;
}

int integrate(int i, long double start, long double end, long double width)
{
	long double right = min(start + width, end);
	long double left = start;
	long double area = 0;
	while (left < end)
	{
		area += (right - left) * (4 / (left * left + 1));
		left = right;
		right = min(right + width, end);
	}

	char* filename = (char*)calloc(16, 1);
	sprintf(filename, "w%d.txt", i);
	FILE* fp = fopen(filename, "w+");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot write to %s\n", filename);
		return -1;
	}
	free(filename);
	fprintf(fp, "%.40Lf\n", area);
	fclose(fp);
	return 0;
}

void start_clock()
{
	clock_gettime(CLOCK_REALTIME, &start);	
}

void end_clock(int processes, long double width)
{
	clock_gettime(CLOCK_REALTIME, &end);
	double total = (double)(end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_nsec)/1.0e9;
	printf("%f %d %.20Lf\n", total, processes, width);
	FILE* fp = fopen("raport2.txt", "a+");
	if (fp == NULL)
	{
		fprintf(stderr, "Unable to open raport2.txt for writing\n");
		return;
	}

	fprintf(fp, "%f %d %.20Lf\n", total, processes, width);
	fclose(fp);
}
