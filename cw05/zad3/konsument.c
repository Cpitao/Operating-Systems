#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


int rewrite_all(FILE* fifod, char* filename, int N);

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Invalid number of arguments");
		return 1;
	}

	FILE* fifod = fopen(argv[1], "r");
	if (fifod == NULL)
	{
		fprintf(stderr, "Unable to open FIFO");
		return 1;
	}

	FILE* out_fp = fopen(argv[2], "w+");
	if (out_fp == NULL)
	{
		fprintf(stderr, "Unable to open output file");
		fclose(fifod);
		return 1;
	}
	fclose(out_fp);

	int N = atoi(argv[3]);
	if (N <= 0)
	{
		fprintf(stderr, "N must be a positive integer");
		return 1;
	}
	
	while (rewrite_all(fifod, argv[2], N));

	fclose(fifod);
}

int rewrite_all(FILE* fifod, char* filename, int N)
{
	int line;
	char* content = calloc(N+1, 1);
	int ret=0;
	// READ NUMBER OF LINE
	char* line_no = calloc(10, 1);
	int len = 0;
	char c;
	while(1) {
		if (fread(&c, 1, 1, fifod) == 1)
		{
			if (c == ' ')
				break;
			line_no[len] = c;
			len++;
		}
		else break;
	}
	line = atoi(line_no);
	if (line <= 0)
	{
		free(content);
		free(line_no);
		return 0;
	}
	free(line_no);

	// READ CONTENT
	if ((ret = fread(content, 1, N, fifod)) == 0)
	{
		fprintf(stderr, "Empty read");
		free(content);
		return 0;
	}

	// READ FILE CONTENT TO APPEND TO IT

	size_t file_size = 256;
	char* all_file = calloc(file_size, 1);
	len = 0;
	FILE* out_fd = fopen(filename, "r");
	if (out_fd == NULL)
	{
		fprintf(stderr, "Unable to open file");
		return 0;
	}
	fseek(out_fd, 0, SEEK_END);
	size_t size = ftell(out_fd);
	fseek(out_fd, 0, SEEK_SET);
	if (size > 0)
	{
		while (fread(&c, 1, 1, out_fd) > 0)
		{
			if (len == file_size - 1)
			{
				file_size*=2;
				all_file = realloc(all_file, file_size);
			}
			all_file[len] = c;
			len++;
		}
	}
	fclose(out_fd);
	// INSERT READ CONTENT IN THE RIGHT PLACE
	
	char* new_file = calloc(strlen(all_file) + strlen(content) + 5, 1);
	int new_lines = 0;
	int pos = 0;
	for (int i=0; i < len; i++)
	{
		if (new_lines == line) 
		{
			pos = i;
			break;
		}
		if (all_file[i] == '\n') new_lines++;
	}

	if (new_lines < line)
	{
		strcpy(new_file, all_file);
		for (int i=0; i < line - new_lines; i++) strcat(new_file, "\n");
		strcat(new_file, content);
	}
	else
	{
		while (pos < strlen(all_file) && all_file[pos] != '\n') pos++;
		char* before = calloc(pos + 5, 1);
		strncpy(before, all_file, pos);
		char* after = calloc(strlen(all_file) - pos + 5, 1);
		strncpy(after, all_file + pos, strlen(all_file) - pos);

		sprintf(new_file, "%s%s%s", before, content, after);
		free(before);
		free(after);
	}
	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "Unable to open file");
		free(new_file);
		free(all_file);
		free(content);
		return 0;
	}

	fputs(new_file, fp);
	fclose(fp);

	free(content);
	free(new_file);
	free(all_file);
	return ret;
}
