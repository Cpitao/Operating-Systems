#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


void read_mails(int mode)
{
	if (mode == 1)
	{
		FILE* pp = popen("mail | tail -n+3 | head -n-1 | sort -k3", "w");
		fwrite("quit", 1, 5, pp);
		pclose(pp);
	}
	else if (mode == 2)
	{
		FILE* pp = popen("mail | tail -n+3 | head -n-1", "w");
		fwrite("quit", 1, 5, pp);
		pclose(pp);
	}
}

void send_mail(char* address, char* title, char* message)
{
	char* mail_exec = calloc(strlen(title) + strlen(address) + 5, 1);
	sprintf(mail_exec, "mail -s \"%s\" %s", title, address);
	FILE* pp = popen(mail_exec, "w");
	fputs(message, pp);
	pclose(pp);
}


int main(int argc, char** argv)
{
	if (argc == 2)
	{
		if (strcmp(argv[1], "nadawca") == 0) read_mails(1);
		else if (strcmp(argv[1], "data") == 0) read_mails(2);
		else
		{
			fprintf(stderr, "Wrong option specified");
			return 1;
		}
	}
	else if (argc == 4)
	{
		send_mail(argv[1], argv[2], argv[3]);
	}
	else
	{
		fprintf(stderr, "Invalid number of arguments (should be 1 or 3)");
		return 1;
	}

	return 0;
}
