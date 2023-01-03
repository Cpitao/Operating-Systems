#include <sys/signal.h>
#include "common.h"

#define USAGE "./client <unique name> <connection type> <server ip> <server port>"

#define UNIX_CONNECT 0
#define IP_CONNECT 1

void start_unix_client(char* name, char* path);
void start_network_client(char* name, char* path, int port);
void run_client(void);
void signal_handler(int sig);
void atexit_func(void) {signal_handler(0);}

int sock;


int main(int argc, char** argv)
{
    signal(SIGINT, signal_handler);
    atexit(atexit_func);
    /* argument parsing */
    if (argc != 4 && argc != 5)
    {
        printf(USAGE);
        return 0;
    }
    char name[255];
    strcpy(name, argv[1]);
    int connection_type;
    if (strcmp(argv[2], "unix") == 0) connection_type = UNIX_CONNECT;
    else if (strcmp(argv[2], "net") == 0) connection_type = IP_CONNECT;
    else 
    {
        fprintf(stderr, "Invalid connection type\n" USAGE);
        return 0;
    }
    char address[255]; // either server IP or unix path
    strcpy(address, argv[3]);
    int port;
    if (connection_type == IP_CONNECT)
    {
        if (argc != 5)
        {
            fprintf(stderr, "You must specify port with network connection type\n");
            return 0;
        }
        port = atoi(argv[4]);
        if (port <= 0)
        {
            fprintf(stderr, "Invalid port number\n");
            return 0;
        }
        start_network_client(name, address, port);
    }
    else if (connection_type == UNIX_CONNECT) start_unix_client(name, address);

    run_client();
}

void start_unix_client(char* name, char* path)
{
    int status;
    struct sockaddr_un server;

    char buffer[255];
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket");
        return;
    }

    memset(&server, 0, sizeof server);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, path);

    status = connect(sock, (struct sockaddr*)&server, sizeof server);
    if (status < 0)
    {
        perror("connect");
        return;
    }

    status = write(sock, name, strlen(name));
    if (status <= 0)
    {
        perror("write");
        return;
    }

    status = read(sock, buffer, sizeof(buffer)-1);
    buffer[status] = '\0';
    printf("%s\n", buffer);
    return;
}

void start_network_client(char* name, char* address, int port)
{
    int status;
    struct sockaddr_in server;

    char buffer[255];
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket");
        return;
    }

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address);
    
    status = connect(sock, (struct sockaddr*) &server, sizeof server);
    if (status < 0)
    {
        perror("connect");
        return;
    }

    status = write(sock, name, strlen(name));
    if (status <= 0)
    {
        perror("write");
        return;
    }

    status = read(sock, buffer, sizeof(buffer) - 1);
    buffer[status] = '\0';
    printf("%s\n", buffer);

    return;
}

void signal_handler(int sig)
{
    if (sig == -1) return;
    write(sock, "SHUTDOWN", 8);
    char buffer[255];
    read(sock, buffer, 255);
    printf("%s\n", buffer);
    close(sock);
    exit(0);
}

void run_client(void)
{
    int status;
    board b;
    while (1)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(sock, &rfds);
        int retval = select(sock + 1, &rfds, NULL, NULL, NULL);

        if (retval == -1)
        {
            perror("select");
            return;
        }
        else if (retval)
        {
            if (FD_ISSET(sock, &rfds)) // msg from server
            {
                char buffer[255];
                if ((status=read(sock, buffer, sizeof(buffer) - 1)) <= 0)
                {
                    perror("read");
                    return;
                }
                buffer[status] = '\0';
                char is_shutdown[9];
                strncpy(is_shutdown, buffer, 8);
                is_shutdown[9] = '\0';
                if (strcmp(is_shutdown, "SHUTDOWN") == 0)
                {
                    printf("%s\n", buffer);
                    return;
                }

                printf("%s\n", buffer);
            }
            if (FD_ISSET(0, &rfds)) // stdin
            {
                char buffer[255];
                if ((status=read(0, buffer, 255)) <= 0)
                {
                    perror("read");
                    return;
                }
                buffer[status] = '\0';
                status = write(sock, buffer, strlen(buffer));
                if (status <= 0)
                {
                    perror("write");
                    return;
                }
            }
        }
    }
}