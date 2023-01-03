#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const int PARSED_SIGNAL = SIGUSR1;

int parse_arguments(int argc, char** argv);
void handler(int sig);
void mask();
void is_pending();


int main(int argc, char** argv)
{
	int option = parse_arguments(argc, argv);
	switch(option)
	{
		case -1:
			printf("Error parsing arguments!\n");
			return -1;
		case 1:
			signal(PARSED_SIGNAL, SIG_IGN);
			break;
		case 2:
			signal(PARSED_SIGNAL, handler);
			break;
		case 3:
			mask();
			break;
		case 4:
			mask();
			break;
		default:
			return -1;
	}

	printf("First raise in main\n");
	raise(PARSED_SIGNAL);

	if (option == 4)
		is_pending();

	pid_t pid;
	if ((pid = fork()) == -1)
	{
		printf("Error while forking\n");
		return -1;
	}
	if (pid == 0)
	{
		printf("Inside child process (fork)\n");
		if (option != 4)
		{
			printf("Second raise in child\n");
			raise(PARSED_SIGNAL);
		}
		else
			is_pending();
		printf("Exiting child process (fork)\n");
		return 0;
	}
	wait(0); // so as not to mix up logs from multiple processes together
		
	printf("Going to execl zad1_exec now\n");	
	execl("./zad1_exec", "zad1_exec", NULL);
	fprintf(stderr, "Unable to execl zad1_exec\n");
	
	return 0;
}

int parse_arguments(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "No argument given\n");
		return -1;
	}

	if (strcmp(argv[1], "ignore") == 0) return 1;
	else if (strcmp(argv[1], "handler") == 0) return 2;
	else if (strcmp(argv[1], "mask") == 0) return 3;
	else if (strcmp(argv[1], "pending") == 0) return 4;
	else return -1;
}

void handler(int sig)
{
	printf("Got signal %d\n", sig);
}

void mask()
{
	sigset_t sa_mask;
	sigemptyset(&sa_mask);
	sigaddset(&sa_mask, PARSED_SIGNAL);

	if(sigprocmask(SIG_BLOCK, &sa_mask, NULL) < 0)
	{
		fprintf(stderr, "Failed to set signal mask\n");
	}
}

void is_pending()
{
	sigset_t set;
	if (sigpending(&set) != 0)
	{
		fprintf(stderr, "Unable to read pending signals\n");
		return;
	}

	if (sigismember(&set, PARSED_SIGNAL))
		printf("Signal %d pending\n", PARSED_SIGNAL);
	else
		printf("Signal %d not pending\n", PARSED_SIGNAL);
}
