#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


void count_sys(char* filename, char c);
void count_lib(char* filename, char c);

clock_t start;
clock_t end;
void start_clock();
void end_clock(char* msg, char* filename, char letter);

int main(int argc, char** args)
{
	if (argc != 3)
	{
		fprintf(stderr, "Invalid number of arguments\n");
		return -1;
	}

	char c = args[1][0];
	char* filename = args[2];

	#ifdef LIB
	start_clock();
	count_lib(filename, c);
	end_clock("Library time: ", filename, c);
	#else
	start_clock();
	count_sys(filename, c);
	end_clock("System time: ", filename, c);
	#endif
}


void count_sys(char* filename, char c)
{
	int fd;
	if ((fd=open(filename, O_RDONLY)) < 0)
	{
		fprintf(stderr, "Unable to open file\n");
		return;
	}

	int occurrences = 0;
	int line_occ = 0;

	char ch;
	int line_flag = 0;
	while (read(fd, &ch, 1) == 1)
	{
		if (ch == c)
		{
			occurrences++;
			if (!line_flag) line_occ++;
			line_flag = 1;
		}

		if (ch == '\n') line_flag = 0;
	}

	close(fd);
	printf("%d character occurrences in %d lines\n", occurrences, line_occ);
}

void count_lib(char* filename, char c)
{
	FILE* fp;
	if ((fp=fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "Unable to open file\n");
		return;
	}

	int occurrences = 0;
	int line_occ = 0;

	char ch;
	int line_flag = 0;
	while (fread(&ch, 1, 1, fp) == 1)
	{
		if (ch == c)
		{
			occurrences++;
			if (!line_flag) line_occ++;
			line_flag = 1;
		}
		
		if (ch == '\n') line_flag = 0;
	}

	fclose(fp);
	printf("%d character occurrences in %d lines\n", occurrences, line_occ);
}

void start_clock()
{
	start = clock();
}

void end_clock(char* msg, char* filename, char letter)
{
	end = clock();
	FILE* fp = fopen("pomiar_zad_2.txt", "a+");
	if (fp == NULL)
	{
		fprintf(stderr, "Unable to open pomiar_zad_2.txt\n");
		return;
	}
	fprintf(fp, "%s%f for %s for '%c'\n", msg, (double)(end-start)/CLOCKS_PER_SEC, filename, letter);
	fclose(fp);
}
