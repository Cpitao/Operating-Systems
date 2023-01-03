#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** args)
{
	for (int i=1; i < argc; i++)
	{
		int lines = atoi(args[i]);
		char* filename = (char*) calloc(100, 1);
		strcat(filename, args[i]);
		strcat(filename, ".txt");

		FILE* fp = fopen(filename, "w+");
		if (fp == NULL) continue;

		for (int j=0; j < lines; j++)
		{
			for (int k=0; k < j; k++)
				fprintf(fp, "*");
			fprintf(fp, "\n");
		}
	}
}
