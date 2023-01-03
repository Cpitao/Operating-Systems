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
int mode;
int finish = 0;

void usr1_handler(int signo, siginfo_t *si, void *uc);
void usr2_handler(int signo, siginfo_t *si, void *uc);

void block_other();
void set_usr1_action();
void set_usr2_action();

int main(int argc, char** argv)
{
	printf("%d\n", (int)getpid());
	block_other();
	set_usr1_action();
	set_usr2_action();
		
	while(!finish)
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
	pid_t sender_pid = si->si_pid;
	switch(si->si_code)
	{
		case SI_USER:
			kill(sender_pid, BASE_SIGNAL);
			break;
		case SI_QUEUE:;
			union sigval value;
			value.sival_int = RECEIVED_SIGNALS;
			sigqueue(sender_pid, BASE_SIGNAL, value);
			break;
	}
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
	pid_t sender_pid = si->si_pid;
	switch(si->si_code)
	{
		case SI_USER:
			kill(sender_pid, END_SIGNAL);
			break;
		case SI_QUEUE:;
			union sigval value;
			value.sival_int = RECEIVED_SIGNALS+1;
			sigqueue(sender_pid, END_SIGNAL, value);
			break;
	}
	printf("Catcher received %d signals\n", RECEIVED_SIGNALS);
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
