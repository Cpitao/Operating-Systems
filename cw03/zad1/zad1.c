#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	int n;
	if (argc < 2)
	{
		printf("n: ");
		scanf("%d", &n);
	}
	else n = atoi(argv[1]);
	pid_t pid;
	for (int i=0; i < n; i++)
	{
		pid = fork();
		if (pid == 0)
		{
			
			printf("Child PID: %d\n", (int)getpid());
			return 0;
		}
	}

	return 0;
}
