#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#include <time.h>


void nonftw_browse(char* dirname, int flag, int*reg,int*dirs,int*chr,int*blk,int*fifo,int*link,int*socket);
void nftw_browse(char* dirname);
int parse_nftw_file(const char* pathname, const struct stat* buffer, int info, struct FTW* fsp);

static int reg=0,dirs=0,chr=0,blk=0,fifo=0,lnk=0,socket=0;

int main(int argc, char** args)
{
	if (argc < 2)
	{
		fprintf(stderr, "Directory not specified\n");
		return -1;
	}

	char* dirname = args[1];
	#ifdef NONFTW
	nonftw_browse(dirname, 0, &reg,&dirs,&chr,&blk,&fifo,&lnk,&socket);
	#else
	nftw_browse(dirname);
	#endif
	return 0;
}

void nonftw_browse(char* dirname, int flag, int *reg, int*dirs, int*chr, int*blk, int*fifo, int*link, int*socket)
{
	DIR* dir;
	struct dirent de;
	struct dirent *dp = &de;

	if ((dir = opendir(dirname)) == NULL)
	{
		fprintf(stderr, "Unable to open %s\n", dirname);
		return;
	}
	//int reg=0, dirs=0, chr=0, blk=0, fifo=0, link=0, socket=0;

	struct stat t;
	struct stat *s = &t;
	while ((dp = readdir(dir)) != NULL)
	{
		if (!strcmp(dp->d_name, "..")) continue;
		char *filepath = (char*) calloc(256, 1);
		strcpy(filepath, dirname);
		strcat(filepath, "/");
		strcat(filepath, dp->d_name);

		filepath = realpath(filepath, NULL);

		printf("-----------------------------------\n");
		printf("File: %s\n", dp->d_name);
		printf("Absolute path: %s\n", filepath);

		if (stat(filepath, s) != 0)
		{
			fprintf(stderr, "Unable to retrieve %s info\n", dp->d_name);
			continue;
		}

		printf("Hardlinks count: %lu\n", s->st_nlink);
		printf("Size in bytes: %lu\n", s->st_size);
		printf("Last access time: %s", ctime(&s->st_atime));
		printf("Last modification time: %s", ctime(&s->st_mtime));
		if (S_ISREG(s->st_mode))
		{
			printf("Regular file\n");
			(*reg)++;
		}
		else if (S_ISDIR(s->st_mode))
		{
			printf("Directory\n");
			(*dirs)++;
			if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
			{
				printf("%s\n", dp->d_name);
				nonftw_browse(filepath, 1, reg, dirs, chr, blk, fifo, link, socket);
			}
			else if (flag) (*dirs)--;
		}
		else if (S_ISCHR(s->st_mode))
		{
			printf("Char device\n");
			(*chr)++;
		}
		else if (S_ISBLK(s->st_mode))
		{
			printf("Block device\n");
			(*blk)++;
		}
		else if (S_ISFIFO(s->st_mode))
		{
			printf("Fifo\n");
			(*fifo)++;
		}
		else if (S_ISSOCK(s->st_mode))
		{
			printf("Socket\n");
			(*socket)++;
		}
		else if (S_ISLNK(s->st_mode))
		{
			printf("Symlink\n");
			(*link)++;
		}
		free(filepath);
	}
	if (!flag)
	{
	printf("----------------------------------\n");
	printf("Regular files: %d\nDirectories: %d\nCharacter devices: %d\nBlock devices: %d\nFifo: %d\nSockets: %d\nSymlinks: %d\n", *reg, *dirs, *chr, *blk, *fifo, *socket, *link);
	}
	return;
}

void nftw_browse(char* dirname)
{
	nftw(dirname, &parse_nftw_file, 100, FTW_PHYS);
	printf("Regular files: %d\nDirectories: %d\nCharacter devices: %d\nBlock devices: %d\nFifo: %d\nSockets: %d\nSymlinks: %d\n", reg, dirs, chr, blk, fifo, socket, lnk);
}

int parse_nftw_file(const char* pathname, const struct stat* buffer, int info, struct FTW* fsp)
{
	printf("File path: %s\n", pathname);
	printf("Hard links: %lu\n", buffer->st_nlink);
	printf("Size in bytes: %lu\n", buffer->st_size);
	printf("Last access time: %s\n", ctime(&buffer->st_atime));
	printf("Last modification time: %s\n", ctime(&buffer->st_mtime));
	if (S_ISREG(buffer->st_mode))
	{
		printf("Regular file\n");
		reg++;
	}
	else if (S_ISDIR(buffer->st_mode))
	{
		printf("Directory\n");
		dirs++;
	}
	else if (S_ISCHR(buffer->st_mode))
	{
		printf("Character device\n");
		chr++;
	}
	else if (S_ISBLK(buffer->st_mode))
	{
		printf("Block device\n");
		blk++;
	}
	else if (S_ISFIFO(buffer->st_mode))
	{
		printf("Fifo\n");
		fifo++;
	}
	else if (S_ISSOCK(buffer->st_mode))
	{
		printf("Socket\n");
		socket++;
	}
	else if (S_ISLNK(buffer->st_mode))
	{
		printf("Symbolic link\n");
		lnk++;
	}
	
	return 0;
}
