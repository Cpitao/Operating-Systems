#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>


#ifndef RTSIG
const int BASE_SIGNAL = SIGUSR1;
const int END_SIGNAL = SIGUSR2;
#else
const int BASE_SIGNAL = 34;
const int END_SIGNAL = 35;
#endif

int RECEIVED_SIGNALS = 0;
int mode, signals_count, finish=0;

void usr1_handler(int signo, siginfo_t *si, void *uc);
void usr2_handler(int signo, siginfo_t *si, void *uc);

void block_other();
void set_usr1_action();
void set_usr2_action();

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		fprintf(stderr, "Wrong number of arguments\n");
		return 1;
	}
	pid_t catcher_pid = atoi(argv[1]);
	signals_count = atoi(argv[2]);
	mode = atoi(argv[3]);

	for (int i=0; i < signals_count; i++)
	{
		switch(mode)
		{
			case 1:
				kill(catcher_pid, BASE_SIGNAL);
				break;
			case 2:;
				union sigval val;
				sigqueue(catcher_pid, BASE_SIGNAL, val);
				break;
		}
	}

	block_other();
	set_usr1_action();
	set_usr2_action();
	
	// sigset_t accepted_signals;
	// sigemptyset(&accepted_signals);
	// sigaddset(&accepted_signals, BASE_SIGNAL);
	// sigaddset(&accepted_signals, END_SIGNAL);

	switch(mode)
	{
		case 1:
			if(kill(catcher_pid, END_SIGNAL) == -1)
			{
				printf("Unable to send signal\n");
			}
			break;
		case 2:;
			union sigval val;
			sigqueue(catcher_pid, END_SIGNAL, val);
			break;
	}
	while (!finish)
	{
		sigset_t accepted_signals;
		sigfillset(&accepted_signals);
		sigdelset(&accepted_signals, BASE_SIGNAL);
		sigdelset(&accepted_signals, END_SIGNAL);
		set_usr1_action();
		set_usr2_action();
		sigsuspend(&accepted_signals);
	}
}

void block_other()
{
	sigset_t blocked;
	sigfillset(&blocked);
	sigdelset(&blocked, BASE_SIGNAL);
	sigdelset(&blocked, END_SIGNAL);

	sigprocmask(SIG_BLOCK, &blocked, NULL);
}

void usr1_handler(int signo, siginfo_t *si, void *uc)
{
	RECEIVED_SIGNALS++;
	if (mode == 2)
		printf("(Sender) Signal with info: %d received from catcher\n", si->si_value.sival_int);
}

void set_usr1_action()
{
	struct sigaction sa;
	sa.sa_sigaction = usr1_handler;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, END_SIGNAL);
	sa.sa_flags = SA_SIGINFO;
	sigaction(BASE_SIGNAL, &sa, NULL);
}

void usr2_handler(int signo, siginfo_t *si, void *uc)
{
	printf("Sender received %d signals back\n", RECEIVED_SIGNALS);
	printf("Expected (sent) %d signals\n", signals_count);
	finish = 1;
}

void set_usr2_action()
{
	struct sigaction sa;
	sa.sa_sigaction = usr2_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(END_SIGNAL, &sa, NULL);
}
