#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <dlfcn.h>
#ifndef DYNAMIC
#include "../zad1/zad1.h"
#endif

#ifdef DYNAMIC

typedef void** (*create_array_f) ();
typedef void (*file_stats_f) ();
typedef int (*save_results_to_array_f) ();
typedef void (*free_index_f) ();

create_array_f create_array;
file_stats_f file_stats;
save_results_to_array_f save_results_to_array;
free_index_f free_index;

#endif

void start();
void end(char* operation, char* info);

clock_t real_clock_start;
clock_t real_clock_end;
clock_t clock_start;
clock_t clock_end;
struct tms cpu_start;
struct tms cpu_end;

int parse(int argc, char** args, int log);

int main(int argc, char** args)
{
	#ifdef DYNAMIC

	void* handle = dlopen("libzad1_shared.so", RTLD_LAZY);
	if (!handle) 
	{
		fprintf(stderr, "Cannot load library");
		return -1;
	}

	create_array = (void** (*)())dlsym(handle, "create_array");
	file_stats = (void (*)())dlsym(handle, "file_stats");
	save_results_to_array = (int (*)())dlsym(handle, "save_results_to_array");
	free_index = (void (*)())dlsym(handle, "free_index");

	#endif

	int log = 1;
	for (int i=0; i < argc; i++)
	{
		if (strcmp(args[i], "--nolog") == 0)
		{
			log = 0;
			break;
		}
	}
	
	if (parse(argc, args, log) == 0 && log)
	{
		printf("Program executed successfully\n");
	}

	#ifdef DYNAMIC
	dlclose(handle);
	#endif

	return 0;
}

int parse(int argc, char** args, int log)
{
	int elements_count = -1;
	void** main_array = NULL;
	
	printf("%12s %12s %12s %12s %25s\n", "OPERATION", "REAL_TIME[s]", "USER_TIME[s]", "SYSTEM_TIME[s]", "ADDITIONAL_INFO");
	for (int i=1; i < argc; i++)
	{
		if (strcmp(args[i], "-s") == 0)
		{
				
			start();
			

			if (elements_count != -1)
			{
				fprintf(stderr, "Array count cannot be declared twice (invalid -s usage)\n");
				for (int i=0; i < elements_count; i++)
				{
					free(main_array[i]);
				}
				free(main_array);
				return -1;
			}
			
			i++;
			if (i >= argc)
			{
				fprintf(stderr, "No argument for -s\n");
				return -1;
			}
			elements_count = strtol(args[i], NULL, 10);
			if (elements_count < 1)
			{
				fprintf(stderr, "Array elements count must be a positive integer (invalid -s usage)\n");
				return -1;
			}
			main_array = create_array(elements_count);

			if (log && main_array != NULL)
				printf("Allocated %d elements array successfully!\n", elements_count);
			
			char* info = calloc(25, 1);
			sprintf(info, "%d", elements_count);
			strcat(info, " elements");
			end("create", info);
			free(info);
		}
		else if (strcmp(args[i], "-r") == 0)
		{
			start();
			i++;
			if (i >= argc)
			{
				fprintf(stderr, "No argument for -r\n");
				if (main_array != NULL)
				{
					for (int i=0; i < elements_count; i++)
						free(main_array[i]);
					free(main_array[i]);
				}
				return -1;
			}

			if (main_array == NULL)
			{
				fprintf(stderr, "Trying to remove element from uninitialised array\n");
				return -1;
			}
			int index = strtol(args[i], NULL, 10);
		       	
			if (elements_count <= index || index < 0 || (index == 0 && args[i][0] != '0'))
			{
				fprintf(stderr, "Index out of range for -r\n");
			}
			else 
			{
				free_index(main_array, index);
				if (log)
				{
					printf("Freed index %d\n", index);
				}
			}
			char* info = calloc(25, 1);
			sprintf(info, "index %d", index);
			end("remove", info);
			free(info);
		}
		else if (strcmp(args[i], "-f") == 0)
		{
			i++;
			if (main_array == NULL)
			{
				fprintf(stderr, "Attempt to write to uninitialized array\n");
				return -1;
			}
			while (i < argc && strcmp(args[i], "-r") && strcmp(args[i], "-s") && strcmp(args[i], "--nolog") && strcmp(args[i], "-f"))
			{
				start();

				file_stats(args[i]);

				end("count", args[i]);

				char* out_name = (char*)calloc(255, 1);
				strcpy(out_name, args[i]);
				strcat(out_name, "_stats.tmp");

				start();

				int index = save_results_to_array(main_array, elements_count, out_name);

				end("save", args[i]);
				free(out_name);
				if (index == -1)
				{
					fprintf(stderr, "Error trying to save results to array\n");
				}
				else if (log)
				{
					printf("Saved results to array, they are:\n");
					printf("%s\n", (char*)main_array[index]); 
				}
				i++;
			}
			i--;
		}
	}
	if (main_array != NULL)
	{
		for (int i=0; i < elements_count; i++)
		{
			free(main_array[i]);
		}
		free(main_array);
	}

	return 0;
}

void start()
{
	real_clock_start = clock();
	clock_start = times(&cpu_start);
}

void end(char* operation, char* info)
{
	real_clock_end = clock();
	clock_end = times(&cpu_end);
	printf("%12s %12f %12f %12f %25s\n", operation, 
			((double)(real_clock_end - real_clock_start)) / CLOCKS_PER_SEC,
		       	((double)(cpu_end.tms_utime - cpu_start.tms_utime)) / CLOCKS_PER_SEC,
			((double)(cpu_end.tms_stime - cpu_start.tms_stime)) / CLOCKS_PER_SEC,
			info);
}
