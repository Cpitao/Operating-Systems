#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "server_game.h"

#define USAGE "./server <port number> <unix socket path>"

int start_net_socket(int port);
int start_unix_socket(char* path);
void run_server(int net_socket, int unix_socket);
int init_fds_set(fd_set* fds);
int init_client(int skt);
void shutdown_client(int i, char* message);
void handle_client(int i);
void signal_handler(int sig);
void match_opponents(int p1, int p2);
int max(int a, int b) 
{
    if (a >= b) return a;
    return b;
}

Client_t *clients[MAX_PLAYERS];
Game_t *games[MAX_PLAYERS/2];

int net_socket, unix_socket, running=1, waiting_for_opponent[MAX_PLAYERS];

int main(int argc, char** argv)
{
    srand(time(NULL));
    signal(SIGINT, signal_handler);
    if (argc != 3)
    {
        puts(USAGE);
        return 0;
    }
    int port = atoi(argv[1]);
    if (port <= 1024)
    {
        printf("Port should be >= 1024\n");
        return 0;
    }
    for (int i=0; i < MAX_PLAYERS; i++) clients[i] = NULL;
    for (int i=0; i < MAX_PLAYERS/2; i++) games[i] = NULL;
    for (int i=0; i < MAX_PLAYERS; i++) waiting_for_opponent[i] = 0;

    char* un_path = argv[2];
    
    net_socket = start_net_socket(port);
    if (net_socket == -1) 
    {
        for (int i=0; i < MAX_PLAYERS/2; i++) free(games[i]);
        return 0;
    }
    unix_socket = start_unix_socket(un_path);
    if (unix_socket == -1)
    {
        for (int i=0; i < MAX_PLAYERS/2; i++) free(games[i]);
        close(net_socket);
        return 0;
    }

    run_server(net_socket, unix_socket);

    close(net_socket);
    close(unix_socket);
    unlink(un_path);
    for (int i=0; i < MAX_PLAYERS/2; i++) 
    {
        if (games[i] != NULL)
            free(games[i]);
    }
    for (int i=0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL)
        {
            shutdown_client(i, "Server shutdown");
        }
    }
}

int start_net_socket(int port)
{
    int status, net_socket;

    struct sockaddr_in server;

    net_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (net_socket == -1)
    {
        perror("socket net");
        return -1;
    }

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    status = bind(net_socket, (struct sockaddr*) &server, sizeof server);
    if (status == -1)
    {
        perror("bind net");
        return -1;
    }

    status = listen(net_socket, 16);
    if (status == -1)
    {
        perror("listen");
        return -1;
    }

    return net_socket;
}

int start_unix_socket(char* path)
{
    int status, unix_socket;

    struct sockaddr_un server;

    unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unix_socket == -1)
    {
        perror("socket unix");
        return -1;
    }

    memset(&server, 0, sizeof server);
    server.sun_family = AF_UNIX;
    
    strcpy(server.sun_path, path);
    
    status = bind(unix_socket, (struct sockaddr*) &server, SUN_LEN(&server));
    if (status == -1)
    {
        perror("bind unix");
        return -1;
    }

    status = listen(unix_socket, 16);
    if (status == -1)
    {
        perror("listen");
        return -1;
    }

    return unix_socket;
}

void run_server(int net_socket, int unix_socket)
{
    fd_set rfds;
    int max_fd;
    int status;
    while (running)
    {
        max_fd = init_fds_set(&rfds);
        int retval = select(max_fd, &rfds, NULL, NULL, NULL);
        if (retval == -1)
        {
            return;
        }
        else if (retval)
        {
            if (FD_ISSET(net_socket, &rfds))
            {
                init_client(net_socket);
            }
            if (FD_ISSET(unix_socket, &rfds))
            {
                init_client(unix_socket);
            }
            for (int i=0; i < MAX_PLAYERS; i++)
            {
                if (clients[i] != NULL && FD_ISSET(clients[i]->fd, &rfds))
                {
                    handle_client(i);
                }
            }
        }
    }
}

int init_fds_set(fd_set* fds)
{
    int max_fd = 0;
    FD_ZERO(fds);
    FD_SET(net_socket, fds);
    FD_SET(unix_socket, fds);
    max_fd = max(net_socket, unix_socket);
    for (int i=0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL)
        {
            FD_SET(clients[i]->fd, fds);
            max_fd = max(max_fd, clients[i]->fd);
        }
    }

    return max_fd + 1;
}

/* return -1 if name taken, 0 if reached MAX CLIENTS, >0 on success */
int init_client(int skt)
{
    int status;
    struct sockadrr* client;
    char buffer[255];
    int length = sizeof client;
    for (int i=0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] == NULL)
        {
            clients[i] = calloc(1, sizeof(Client_t));
            clients[i]->fd = accept(skt, (struct sockaddr*) &clients[i]->client, &length);
            if (clients[i]->fd == -1)
            {
                perror("accept");
            }
            if ((status=read(clients[i]->fd, buffer, sizeof(buffer) - 1)) <= 0)
            {
                perror("read");
                shutdown_client(i, "Invalid name\n");
                return -1;
            }
            buffer[status] = '\0';
            int name_taken = 0;
            for (int j=0; j < MAX_PLAYERS; j++)
            {
                if (i == j) continue;
                if (clients[j] != NULL && strcmp(clients[j]->name, buffer) == 0) 
                {
                    name_taken = 1;
                    break;
                }
            }
            if (name_taken)
            {
                shutdown_client(i, "Name is already taken");
                return -1;
            }
            strcpy(clients[i]->name, buffer);
            for (int j=0; j < MAX_PLAYERS; j++)
            {
                if (waiting_for_opponent[j]) 
                {
                    match_opponents(i, j);
                    waiting_for_opponent[j] = 0;
                    return i;
                }
            }
            waiting_for_opponent[i] = 1;
            write(clients[i]->fd, "Waiting for opponent...", 24);
            return i;
        }
    }
    return 0;
}

void shutdown_client(int i, char* message)
{
    if (clients[i] == NULL) return;
    int opponent = -1;
    for (int i=0; i < MAX_PLAYERS/2; i++)
    {
        if (games[i] != NULL)
        {
            if (games[i]->player1 == i) 
            {
                opponent = games[i]->player2;
                free(games[i]);
                games[i] = NULL;
                break;
            }
            else if (games[i]->player2 == i) 
            {
                opponent = games[i]->player1;
                free(games[i]);
                games[i] = NULL;
                break;
            }
        }
    }
    char buffer[255];
    sprintf(buffer, "SHUTDOWN %s", message);

    write(clients[i]->fd, buffer, strlen(buffer));
    shutdown(clients[i]->fd, SHUT_RDWR);
    free(clients[i]);
    clients[i] = NULL;
    waiting_for_opponent[i] = 0;
    if (opponent != -1)
    {
        shutdown_client(opponent, "Game is over or opponent quit");
    }
}

void handle_client(int i)
{
    char buffer[255];
    int status;
    if ((status=read(clients[i]->fd, buffer, sizeof(buffer) - 1)) <= 0)
    {
        perror("read");
        return;
    }
    buffer[status] = '\0';
    if (strcmp(buffer, "SHUTDOWN") == 0) 
    {
        char message[255] = "Shutdown requested by client\n";
        shutdown_client(i, message);
        return;
    }

    if (waiting_for_opponent[i]) return;

    int game_number = -1;
    for (int j=0; i < MAX_PLAYERS/2; j++)
    {
        if (games[j] != NULL && (games[j]->player1 == i || games[j]->player2 == i))
        {
            game_number = j;
            break;
        }
    }

    if (game_number == -1)
    {
        char message[255] = "No matching game found for this client\n";
        shutdown_client(i, message);
        return;
    }
    int n = atoi(buffer);
    if (games[game_number]->next_move != i)
    {
        write(clients[i]->fd, "Wait for your opponent to make a move", 38);
        return;
    }
    if (n < 0 || n > BOARD_SIZE * BOARD_SIZE)
    {
        write(clients[i]->fd, "Invalid field", 14);
        return;
    }
    

    int result;
    if (games[game_number]->player1 == i) 
        result = make_move(&games[game_number]->b, n-1, games[game_number]->assigned1);
    else result = make_move(&games[game_number]->b, n-1, games[game_number]->assigned2);

    if (result)
    {
        write(clients[i]->fd, "Field already taken", 20);
        return;
    }

    /* change to other players move */
    int other_player;
    if (games[game_number]->player1 == i)
        other_player = games[game_number]->player2;
    else other_player = games[game_number]->player1;
    games[game_number]->next_move = other_player;

    char* board_msg = calloc(255, 1);
    sprint_board(board_msg, &games[game_number]->b);
    write(clients[i]->fd, board_msg, strlen(board_msg));
    write(clients[other_player]->fd, board_msg, strlen(board_msg));
    free(board_msg);
    int win = check_win(&games[game_number]->b, n-1);
    if (win % 2)
    {
        write(clients[i]->fd, "You won. Congratulations!", 26);
        write(clients[other_player]->fd, "You lost. Better luck next time.", 33);
        shutdown_client(i, "");
        shutdown_client(other_player, "");
    }
    else if (win == 2)
    {
        write(clients[i]->fd, "It's a draw!\n", 12);
        write(clients[other_player]->fd, "It's a draw!\n", 12);
        shutdown_client(i, "");
        shutdown_client(other_player, "");
    }

    return;
}

void signal_handler(int sig)
{
    running = 0;
}

void match_opponents(int p1, int p2)
{
    for (int i=0; i < MAX_PLAYERS/2; i++)
    {
        if (games[i] == NULL)
        {
            games[i] = calloc(1, sizeof(Game_t));
            int r = rand() % 2;
            if (r)
            {
                int buff = p1;
                p1 = p2;
                p2 = buff;
            }

            games[i]->player1 = p1;
            games[i]->player2 = p2;
            games[i]->assigned1 = 'o';
            games[i]->assigned2 = 'x';
            games[i]->next_move = p1;
            for (int j=0; j < BOARD_SIZE; j++)
            {
                for (int k=0; k < BOARD_SIZE; k++)
                    (games[i]->b).values[j][k] = ' ';
            }
            char message_p1[255];
            sprintf(message_p1, "Playing against %s. You are 'o'. Your move.\n", clients[p2]->name);
            char message_p2[255];
            sprintf(message_p2, "Playing against %s. You are 'x'. Your opponent starts.\n", clients[p1]->name);
            write(clients[p1]->fd, message_p1, strlen(message_p1));
            write(clients[p2]->fd, message_p2, strlen(message_p2));

            // send(clients[p1]->fd, "B", 1, MSG_MORE);
            // send(clients[p2]->fd, "B", 1, MSG_MORE);
            
            char board_msg[255];
            sprint_board(board_msg, &games[i]->b);
            send(clients[p1]->fd, board_msg, strlen(board_msg), 0);
            send(clients[p2]->fd, board_msg, strlen(board_msg), 0);
            return;
        }
    }
}
