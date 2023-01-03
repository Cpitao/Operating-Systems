#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

const int RAISED_SIGNAL = SIGUSR1;
int NODEFER_COUNTER = 0;

int parse_arg(char* arg) {return atoi(arg);}

void info_handler(int signo, siginfo_t *si, void *uc);
void info_action();
void nodefer_handler();
void nodefer_action();
void resethand_handler();
void resethand_action();


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Incorrect usage (provide an option e.g. ./zad2 <1|2|3>\n");
		return 1;
	}

	int option = parse_arg(argv[1]);
	switch(option)
	{
		case 1:
			info_action();
			raise(RAISED_SIGNAL);
			break;
		case 2:
			nodefer_action();
			pid_t parent_pid = getpid();
			pid_t pid = fork();
			if (pid == 0)
			{
				for (int i=0; i < 5; i++)
				{
					kill(parent_pid, RAISED_SIGNAL);
					sleep(1);
				}
				return 0;
			}
			wait(0);
			printf("Received signals: %d\n", NODEFER_COUNTER);
			break;
		case 3:
			resethand_action();
			raise(RAISED_SIGNAL);
			sleep(1);
			raise(RAISED_SIGNAL);
			break;
		default:
			fprintf(stderr, "Invalid argument\n");
			return 1;
	}
	return 0;	
}

void info_action()
{
	struct sigaction sa;
	sa.sa_sigaction = info_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(RAISED_SIGNAL, &sa, NULL);
}

void info_handler(int signo, siginfo_t *si, void *uc)
{
	printf("SIGINFO handler:\n");
	printf("Signal number: %d\n", si->si_signo);
	printf("Caller PID: %d\n", (int)si->si_pid);
	printf("Caller real UID: %d\n", (int)si->si_uid);
	printf("Address which caused fault: %p\n", si->si_addr);
	printf("Errno value (may be 0): %d\n", si->si_errno);
}

void nodefer_action()
{
	struct sigaction sa;
	sa.sa_handler = nodefer_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NODEFER;
	sigaction(RAISED_SIGNAL, &sa, NULL);
}

void nodefer_handler(int signo)
{
	printf("Received signal %d\n", signo);
	printf("Sleeping for 5 seconds\n");
	NODEFER_COUNTER++;
	sleep(5);
	printf("Finished sleeping\n");
}

void resethand_action()
{
	struct sigaction sa;
	sa.sa_handler = resethand_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESETHAND;
	sigaction(RAISED_SIGNAL, &sa, NULL);
}

void resethand_handler(int signo)
{
	printf("Received signal %d\n", signo);
}

