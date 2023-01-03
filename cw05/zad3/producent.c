#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>


int write_all(FILE* fifod, FILE* in_fd, int line, int N);

int main(int argc, char** argv)
{
	if (argc != 5)
	{
		fprintf(stderr, "Invalid number of arguments");
	}

	FILE* fifod = fopen(argv[1], "a");
	if (fifod == NULL)
	{
		printf("No FIFO %s, creating one\n", argv[1]);
		if (mkfifo(argv[1], S_IRWXU | S_IFIFO) != 0)
		{
			fprintf(stderr, "Unable to create FIFO");
			return 1;
		}

		fifod = fopen(argv[1], "w");
	}

	FILE* in_fd = fopen(argv[3], "r");
	if (in_fd == NULL)
	{
		fprintf(stderr, "Unable to open input file");
		fclose(fifod);
		return 1;
	}

	int line = atoi(argv[2]);
	if (line <= 0)
	{
		fprintf(stderr, "line number must be a positive integer");
		fclose(fifod);
		fclose(in_fd);
		return 1;
	}

	int N = atoi(argv[4]);
	if (N <= 0)
	{
		fprintf(stderr, "N must be a positive integer");
		fclose(fifod);
		fclose(in_fd);
		return 1;
	}

	while(write_all(fifod, in_fd, line, N));

	//fclose(fifod);
	fclose(in_fd);

	return 0;
}


int write_all(FILE* fifod, FILE* in_fd, int line, int N)
{
	char* next_write = calloc(N+10, 1);
	sprintf(next_write, "%d ", line);
	srand(time(NULL));
	int ret = 0;
	if((ret = fread(next_write + strlen(next_write), 1, N, in_fd)) == N)
	{
		sleep(rand() % 2 + 1);
		fwrite(next_write, 1, strlen(next_write), fifod);
	}

	free(next_write);

	return ret;
}
