#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

const int PARSED_SIGNAL = SIGUSR1;

void handler(int sig);
void is_pending();

int main(int argc, char** argv)
{
	printf("Inside exec program\n");
	is_pending();
	printf("Raising inside exec\n");
	raise(PARSED_SIGNAL);
	return 0;
}

void handler(int sig)
{
	printf("Got signal %d\n", sig);
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
