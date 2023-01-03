#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

int parse_directory(char *path, char *target_chain, int max_depth);
int find_in_file(char *path, char *target_chain);
int is_text_file(char *file);

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Invalid number of arguments!\n (Expected 3, got %d)", argc-1);
		return -1;
	}

	char *path = argv[1];
	char *target_chain = argv[2];
	int max_depth = atoi(argv[3]);
	
	parse_directory(path, target_chain, max_depth);
	return 0;
}

int parse_directory(char *path, char *target_chain, int max_depth)
{
	DIR *dp = opendir(path);
        if (dp == NULL)
        {
        	fprintf(stderr, "Unable to open directory %s\n", path);
		return -1;
	}
	struct dirent de;
	struct dirent *dep = &de;
	struct stat buff;
	struct stat *buffp = &buff;
	int children_processes = 0;
	while ((dep = readdir(dp)) != NULL)
	{
		char *filename = calloc(255, 1);
		sprintf(filename, "%s/%s", path, dep->d_name);
		if (lstat(filename, buffp) != 0)
		{
			continue;
		}
		if (is_text_file(filename))
		{
			if(find_in_file(filename, target_chain))
			{
				printf("PID: %d\nPath: %s\n", (int)getpid(), filename);
			}
		}
		else if (max_depth > 0 && S_ISDIR(buffp->st_mode) && strcmp(dep->d_name, ".") != 0 && strcmp(dep->d_name, "..") != 0)
		{
			children_processes++;
			pid_t pid = fork();
			if (pid == 0)
			{
				parse_directory(filename, target_chain, max_depth-1);
				return 0;
			}
		}
		free(filename);
	}
	for (int i=0; i < children_processes; i++) wait(0);
	return 0;

}

int find_in_file(char *path, char *target_chain)
{
	size_t n = strlen(target_chain);
	char *s = calloc(n + 1, 1);
	FILE *fp = fopen(path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Unable to open %s\n", path);
		return -1;
	}
	while (fread(s, 1, n, fp) == n)
	{
		if (strcmp(s, target_chain) == 0)
		{
			free(s);
			fclose(fp);
			return 1;
		}

		fseek(fp, 1-n, SEEK_CUR);
	}
	free(s);
	fclose(fp);
	return 0;
}

int is_text_file(char *file)
{
	FILE *fp;
	char *out = calloc(10, 1);
	char *command = calloc(1024, 1);
	sprintf(command, "file %s | grep \"ASCII text\" | wc -l", file);
	fp = popen(command, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Unable to execute command\n");
		return 0;
	}
	free(command);
	fread(out, 1, 9, fp);
	int result = atoi(out);
	pclose(fp);
	return result;
}
