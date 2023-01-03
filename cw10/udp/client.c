#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include "game.h"

#define UNIX_CONNECT 0
#define IP_CONNECT 1
#define USAGE "./client <unique name> <connection type> <server ip> <server port>"


void start_unix_client(char* name, char* path);
void start_network_client(char* name, char* path, int port);
void run_client(void);
void signal_handler(int sig);
void atexit_func(void) {signal_handler(0);}

int sock, connection_type;
struct sockaddr_in in_srv;
struct sockaddr_un un_srv;

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
    int status, len, byteln;
    struct sockaddr_un client;

    char buffer[255];
    sprintf(buffer, "I%s", name);
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        perror("socket");
        return;
    }

    memset(&un_srv, 0, sizeof un_srv);
    un_srv.sun_family = AF_UNIX;
    strcpy(un_srv.sun_path, path);
    status = bind(sock, (struct sockaddr*) &un_srv, sizeof(sa_family_t));
    if (status < 0)
    {
        perror("bind");
        return;
    }
    printf("ABC");fflush(stdout);

    status = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*) &un_srv, sizeof un_srv);
    if (status <= 0)
    {
        perror("write1");
        return;
    }

    len = sizeof client;
    byteln = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    buffer[byteln] = '\0';
    printf("%s\n", buffer);
    return;
}

void start_network_client(char* name, char* address, int port)
{
    int status, len, byteln;
    struct sockaddr_in client;

    char buffer[255]="";
    sprintf(buffer, "I%s", name);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        perror("socket");
        return;
    }

    memset(&in_srv, 0, sizeof in_srv);
    in_srv.sin_family = AF_INET;
    in_srv.sin_port = htons(port);
    in_srv.sin_addr.s_addr = inet_addr(address);
    status = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*) &in_srv, sizeof in_srv);
    if (status <= 0)
    {
        perror("write");
        return;
    }

    len = sizeof client;
    byteln = recvfrom(sock, buffer, sizeof buffer, 0, NULL, NULL);
    buffer[byteln] = '\0';
    printf("%s\n", buffer);
    return;
}

void signal_handler(int sig)
{
    if (sig == -1) return;
    if (connection_type == UNIX_CONNECT)
        sendto(sock, "SHUTDOWN", 8, 0, (struct sockaddr*) &un_srv, sizeof un_srv);
    else
        sendto(sock, "SHUTDOWN", 8, 0, (struct sockaddr*) &in_srv, sizeof in_srv);
    char buffer[255];
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
        struct pollfd pfds[2];
        pfds[0].fd = 0;
        pfds[0].events = POLLIN;
        pfds[1].fd = sock;
        pfds[1].events = POLLIN;
        int retval = poll(pfds, 2, -1);

        if (retval == -1)
        {
            perror("select");
            return;
        }
        else if (retval)
        {
            if (pfds[1].revents == POLLIN) // msg from server
            {
                char buffer[255];
                if ((status=recvfrom(sock, buffer, sizeof(buffer) - 1, 0, NULL, NULL)) <= 0)
                {
                    perror("recvfrom");
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
            if (pfds[0].revents == POLLIN) // stdin
            {
                char buffer[255];
                if ((status=read(0, buffer, 254)) <= 0)
                {
                    perror("read");
                    return;
                }
                buffer[status] = '\0';
                if (connection_type == UNIX_CONNECT)
                    status = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*) &un_srv, sizeof un_srv);
                else
                    status = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*) &in_srv, sizeof in_srv);
                if (status <= 0)
                {
                    perror("write");
                    return;
                }
            }
        }
    }
}