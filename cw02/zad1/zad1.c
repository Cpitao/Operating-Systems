#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <time.h>


void get_args(int argc, char** args, char* in_file, char* out_file);
void copy_content_sys(char* in_file, char* out_file);
void copy_content_lib(char* in_file, char* out_file);

clock_t start;
clock_t end;
void start_clock();
void end_clock(char* msg, char* file);

int main(int argc, char** args)
{
	char* in_file = calloc(255, 1);
	char* out_file = calloc(255, 1);

	get_args(argc, args, in_file, out_file);
	
	#ifdef LIB
	printf("LIB\n");
	start_clock();
	copy_content_lib(in_file, out_file);
	end_clock("Library timer: ", in_file);
	#else
	start_clock();
	copy_content_sys(in_file, out_file);
	end_clock("System timer: ", in_file);
	printf("SYS\n");
	#endif
	free(in_file);
	free(out_file);

	return 0;
}


void get_args(int argc, char** args, char* in_file, char* out_file)
{
	if (argc < 3)
	{
		printf("Input file name:");
		scanf("%s", in_file);
		printf("Output file name:");
		scanf("%s", out_file);
	}
	else
	{
		in_file = strcpy(in_file, args[1]);
		out_file = strcpy(out_file, args[2]);
	}
}

void copy_content_sys(char* in_file, char* out_file)
{
	int fd = open(in_file, O_RDONLY);
	int outd = open(out_file, O_WRONLY | O_CREAT, 0644);
	if (fd == -1)
	{
		fprintf(stderr, "Unable to open %s\n", in_file);
		return;
	}

	if (outd == -1)
	{
		fprintf(stderr, "Unable to open %s\n", out_file);
		return;
	}

	char* line = calloc(256, 1);
	char c;
	int flag = 0;
	int length = 0;
	while(read(fd, &c, 1) == 1)
	{
		if (!isspace(c))
			flag = 1;
		line[length] = c;
		line[length+1] = '\0';
		length++;
		if (c == '\n')
		{
			if (flag)
			{
				write(outd, line, length);
			}
			line[0] = '\0';
			length = 0;
			flag = 0;
		}
	}
	free(line);
	close(outd);
	close(fd);
}

void copy_content_lib(char* in_file, char* out_file)
{
	FILE* finp = fopen(in_file, "r");
	FILE* foutp = fopen(out_file, "w+");

	if (finp == NULL || foutp == NULL)
	{
		fprintf(stderr, "Error opening file\n");
		return;
	}	
	
	char* line = calloc(256, 1);
	char c;
	int flag = 0;
	int length = 0;
	while (fread(&c, 1, 1, finp) == 1)
	{
		if (!isspace(c))
			flag = 1;
		line[length] = c;
		line[length+1] = '\0';
		length++;
		if (c == '\n')
		{
			if (flag)
			{
				fwrite(line, 1, length, foutp);
			}
			line[0] = '\0';
			length = 0;
			flag = 0;
		}
	}

	free(line);
	fclose(finp);
	fclose(foutp);
}

void start_clock()
{
	start = clock();
}

void end_clock(char* msg, char* file)
{
	end = clock();
	FILE* fp = fopen("pomiar_zad_1.txt", "a+");
	if (fp != NULL)
		fprintf(fp, "%s%f for %s\n", msg, (double)(end-start) / CLOCKS_PER_SEC, file);
	else
	{
		printf("Unable to open pomiar_zad_1.txt for writing");
	}
}
