#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void** create_array(int n)
{
	void** arr = (void**)calloc(n, sizeof(void*));

	if (arr == NULL)
	{
		fprintf(stderr, "Array allocation failed");
		return NULL;
	}

	return arr;
}


extern void file_stats(char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error opening %s", filename);
		return;
	}

	char c;
	int words = 0;
	int word_flag = 0;
	int lines = 0;
	int characters = 0;
	while ((c = fgetc(fp)) != EOF)
	{
		characters++;
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
		{
			if (c == '\n') lines++;
			if (word_flag) words++;
			word_flag = 0;
		}
		else
		{
			word_flag = 1;
		}
	}

	fclose(fp);

	char* out_name = malloc((strlen(filename) + 20) * sizeof(char));
	if (out_name == NULL)
	{
		fprintf(stderr, "Array allocation failed\n");
	}
	strcpy(out_name, filename);
	strcat(out_name, "_stats.tmp");
	
	FILE* out = fopen(out_name, "w+");
	free(out_name);

	fprintf(out, "%d lines\n", lines);
	fprintf(out, "%d words\n", words);
	fprintf(out, "%d characters\n", characters);

	fclose(out);
}

extern int save_results_to_array(void** array, int array_len, char* results_filename)
{
	FILE* fp = fopen(results_filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error opening file %s\n", results_filename);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);

	fseek(fp, 0, SEEK_SET);
	char* content = calloc(file_size+1, 1);
	if (content == NULL)
	{
		fprintf(stderr, "Results array allocation failed\n");
		return -1;
	}

	if (fread(content, 1, file_size, fp) == 0)
	{
		fprintf(stderr, "Invalid read\n");
		return -1;
	}
	
	fclose(fp);
	
	int index = -1;
	for (int i=0; i < array_len; ++i)
	{
		if (array[i] == NULL)
		{
			index = i;
			break;
		}
	}
	
	if (index == -1)
	{
		fprintf(stderr, "Pointer array is full\n");
		free(content);
		return -1;
	}

	array[index] = content;

	return index;
}

extern void free_index(void** array, int index)
{
	if (array == NULL) return;
	free(array[index]);
	array[index] = NULL;
}
