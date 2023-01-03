#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <poll.h>
#include "server_handler.h"

void match_opponents(int skt, int p1, int p2, Game_t **games, Client_t **clients)
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
            if (clients[p1]->type == 0)
                sendto(skt, message_p1, strlen(message_p1), 0, (struct sockaddr*) &clients[p1]->in_src, sizeof(clients[p1]->in_src));
            else
                sendto(skt, message_p1, strlen(message_p1), 0, (struct sockaddr*) &clients[p1]->un_src, sizeof(clients[p1]->un_src));

            if (clients[p2]->type == 0)
                sendto(skt, message_p2, strlen(message_p2), 0, (struct sockaddr*) &clients[p2]->in_src, sizeof(clients[p2]->in_src));
            else
                sendto(skt, message_p2, strlen(message_p2), 0, (struct sockaddr*) &clients[p2]->un_src, sizeof(clients[p2]->un_src));
            
            char board_msg[255];
            sprint_board(board_msg, &games[i]->b);
            if (clients[p1]->type == 0)
                sendto(skt, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[p1]->in_src, sizeof(clients[p1]->in_src));
            else
                sendto(skt, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[p1]->un_src, sizeof(clients[p1]->un_src));

            if (clients[p2]->type == 0)
                sendto(skt, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[p2]->in_src, sizeof(clients[p2]->in_src));
            else
                sendto(skt, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[p2]->un_src, sizeof(clients[p2]->un_src));

            return;
        }
    }
}

int init_sockets(int *net_sock, int *uni_sock, int port, char* path)
{
    int status;
    struct sockaddr_in net_serv;
    memset(&net_serv, 0, sizeof net_serv);
    net_serv.sin_family = AF_INET;
    net_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    net_serv.sin_port = htons(port);

    status = bind(*net_sock, (struct sockaddr*) &net_serv, sizeof net_serv);
    if (status < 0)
    {
        perror("bind");
        return -1;
    }

    struct sockaddr_un uni_serv;
    memset(&uni_serv, 0, sizeof uni_serv);
    uni_serv.sun_family = AF_UNIX;
    strcpy(uni_serv.sun_path, path);

    status = bind(*uni_sock, (struct sockaddr*) &uni_serv, sizeof uni_serv);
    if (status < 0)
    {
        perror("bind");
        return -1;
    }

    return 0;
}

void handle_net_input(int *socket, Client_t **clients, Game_t **games, int* waiting_for_opponent)
{
    int status, len;
    char buffer[255] = "";
    struct sockaddr_in src;
    len = sizeof src;
    if ((status = recvfrom(*socket, buffer, sizeof(buffer)-1, 0, (struct sockaddr*) &src, (socklen_t*) &len)) == -1)
    {
        perror("recvfrom");
        return;
    }
    buffer[status] = '\0';
    char name[255] = "";
    strcpy(name, buffer + 1);
    if (buffer[0] == 'I') 
    {
        init_client(clients, &src, NULL, name, waiting_for_opponent, *socket, games);
        return;
    }


    int client_no=-1, game_no=-1;
    for (int i=0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL && clients[i]->in_src.sin_addr.s_addr == src.sin_addr.s_addr &&
            clients[i]->in_src.sin_port == src.sin_port)
        {
            client_no = i;
            break;
        }
    }
    for (int i=0; i < MAX_PLAYERS/2; i++)
    {
        if (games[i] != NULL && (games[i]->player1 == i || games[i]->player2 == i))
        {
            game_no = i;
            break;
        }
    }
    if (game_no == -1) return;

    if (strcmp(buffer, "SHUTDOWN") == 0)
    {
        shutdown_client(games[game_no]->player1, clients, 0, *socket, games);
        shutdown_client(games[game_no]->player2, clients, 0, *socket, games);
        end_game(game_no, games);
        return;
    }
    
    int n = atoi(buffer);
    if (n < 0 || n > BOARD_SIZE * BOARD_SIZE)
    {
        sendto(*socket, "Invalid field\n", 14, 0, (struct sockaddr*) &src, sizeof src);
        return;
    }
    if (games[game_no]->next_move != client_no)
    {
        sendto(*socket, "Wait for your move", 18, 0 ,(struct sockaddr*) &src, sizeof src);
        return;
    }
    int result;
    if (games[game_no]->player1 == client_no) result = make_move(&(games[game_no]->b), n-1, games[game_no]->assigned1);
    else result = make_move(&(games[game_no]->b), n-1, games[game_no]->assigned2);

    if (result)
    {
        sendto(*socket, "Field taken\n", 12, 0, (struct sockaddr*) &src, sizeof src);
        return; 
    }

    int other_player;
    if (games[game_no]->player1 == client_no)
        other_player = games[game_no]->player2;
    else other_player = games[game_no]->player1;
    games[game_no]->next_move = other_player;
    char* board_msg = calloc(255, 1);
    sprint_board(board_msg, &games[game_no]->b);
    sendto(*socket, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[client_no]->in_src, sizeof(clients[client_no]->in_src));
    sendto(*socket, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[other_player]->in_src, sizeof(clients[other_player]->in_src));
    free(board_msg);
    int win = check_win(&games[game_no]->b, n-1);
    if (win % 2)
    {
        sendto(*socket, "You won. Congratulations", 24, 0, (struct sockaddr*) &clients[client_no]->in_src, sizeof(clients[client_no]->in_src));
        sendto(*socket, "You lost. Better luck next time.", 33, 0, (struct sockaddr*) &clients[other_player]->in_src, sizeof(clients[other_player]->in_src));
        shutdown_client(client_no, clients, 0, *socket, games);
        end_game(game_no, games);
    }
    else if (win == 2)
    {
        sendto(*socket, "It's a draw!\n", 13, 0, (struct sockaddr*) &clients[client_no]->in_src, sizeof(clients[client_no]->in_src));
        sendto(*socket, "It's a draw!\n", 13, 0, (struct sockaddr*) &clients[other_player]->in_src, sizeof(clients[other_player]->in_src));
        shutdown_client(client_no, clients, 0, *socket, games);
        end_game(game_no, games);
    }
}

void init_client(Client_t **clients, struct sockaddr_in *net_sa, struct sockaddr_un *un_sa, char* name, int *waiting_for_opponent, int skt, Game_t **games)
{
    for (int i=0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] == NULL)
        {
            clients[i] = calloc(1, sizeof(Client_t));
            if (net_sa != NULL)
            {
                clients[i]->in_src = *net_sa;
                clients[i]->type = 0;
            }
            if (un_sa != NULL)
            {
                clients[i]->un_src = *un_sa;
                clients[i]->type = 1;
            }
            int name_taken = 0;
            for (int j=0; j < MAX_PLAYERS; j++)
            {
                if (i == j) continue;
                if (clients[j] != NULL && strcmp(clients[j]->name, name) == 0) 
                {
                    name_taken = 1;
                    break;
                }
            }
            if (name_taken)
            {
                shutdown_client(i, clients, clients[i]->type, skt, games);
                return;
            }
            strcpy(clients[i]->name, name);
            for (int j=0; j < MAX_PLAYERS; j++)
            {
                if (waiting_for_opponent[j]) 
                {
                    match_opponents(skt, i, j, games, clients);
                    waiting_for_opponent[j] = 0;
                    return;
                }
            }
            int status;
            waiting_for_opponent[i] = 1;
            char msg[255] = "Waiting for opponent...";
            if (clients[i]->type == 0)
                status = sendto(skt, msg, strlen(msg), 0, (struct sockaddr*) &clients[i]->in_src, sizeof(clients[i]->in_src));
            else status = sendto(skt, msg, strlen(msg), 0, (struct sockaddr*) &clients[i]->un_src, sizeof(clients[i]->un_src));

            return;
        }
    }
}

void handle_uni_input(int *socket, Client_t **clients, Game_t **games, int* waiting_for_opponent)
{
    int status, len;
    char buffer[255] = "";
    struct sockaddr_un src;
    len = sizeof(src);
    if ((status = recvfrom(*socket, buffer, sizeof(buffer)-1, 0, (struct sockaddr*) &src, (socklen_t*) &len)) == -1)
    {
        perror("recvfrom");
        return;
    }
    buffer[status] = '\0';

    char name[255] = "";
    strcpy(name, buffer + 1);
    if (buffer[0] == 'I') 
    {
        init_client(clients, NULL, &src, name, waiting_for_opponent, *socket, games);
        return;
    }

    int client_no, game_no=-1;
    for (int i=0; i < MAX_PLAYERS; i++)
    {
        if (clients[i] != NULL && strcmp(clients[i]->un_src.sun_path, src.sun_path) == 0)
        {
            client_no = i;
            break;
        }
    }
    for (int i=0; i < MAX_PLAYERS/2; i++)
    {
        if (games[i] != NULL && (games[i]->player1 == i || games[i]->player2 == i))
        {
            game_no = i;
            break;
        }
    }

    if (strcmp(buffer, "SHUTDOWN") == 0)
    {
        shutdown_client(games[game_no]->player1, clients, 1, *socket, games);
        shutdown_client(games[game_no]->player2, clients, 1, *socket, games);
        end_game(game_no, games);
        return;
    }
    if (game_no == -1) return;
    int n = atoi(buffer);
    if (n < 0 || n > BOARD_SIZE * BOARD_SIZE)
    {
        sendto(*socket, "Invalid field\n", 14, 0, (struct sockaddr*) &src, sizeof src);
    }
    int result;
    if (games[game_no]->player1 == client_no) 
    {
        result = make_move(&(games[game_no]->b), n-1, games[game_no]->assigned1);
    }
    else result = make_move(&(games[game_no]->b), n-1, games[game_no]->assigned2);

    if (result)
    {
        sendto(*socket, "Field taken\n", 12, 0, (struct sockaddr*) &src, sizeof src);
        return;
    }

    int other_player;
    if (games[game_no]->player1 == client_no)
        other_player = games[game_no]->player2;
    else other_player = games[game_no]->player1;
    games[game_no]->next_move = other_player;

    char* board_msg = calloc(255, 1);
    sprint_board(board_msg, &games[game_no]->b);
    sendto(*socket, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[client_no]->un_src, sizeof(clients[client_no]->un_src));
    sendto(*socket, board_msg, strlen(board_msg), 0, (struct sockaddr*) &clients[other_player]->un_src, sizeof(clients[other_player]->un_src));
    free(board_msg);
    int win = check_win(&games[game_no]->b, n-1);
    if (win % 2)
    {
        sendto(*socket, "You won. Congratulations", 24, 0, (struct sockaddr*) &clients[client_no]->un_src, sizeof(clients[client_no]->un_src));
        sendto(*socket, "You lost. Better luck next time.", 33, 0, (struct sockaddr*) &clients[other_player]->un_src, sizeof(clients[other_player]->un_src));
        shutdown_client(client_no, clients, 1, *socket, games);
        shutdown_client(other_player, clients, 1, *socket, games);
        end_game(game_no, games);
    }
    else if (win == 2)
    {
        sendto(*socket, "It's a draw!\n", 12, 1, (struct sockaddr*) &clients[client_no]->un_src, sizeof(clients[client_no]->un_src));
        sendto(*socket, "It's a draw!\n", 12, 1, (struct sockaddr*) &clients[other_player]->un_src, sizeof(clients[other_player]->un_src));
        shutdown_client(client_no, clients, 1, *socket, games);
        shutdown_client(other_player, clients, 1, *socket, games);
        end_game(game_no, games);
    }
}

void end_game(int game_no, Game_t **games)
{
    if (games[game_no] == NULL) return;
    free(games[game_no]);
    games[game_no] = NULL;
}

void shutdown_client(int i, Client_t **clients, int type, int socket, Game_t **games)
{
    if (clients[i] == NULL) return;
    if (type == 0)
    {
        sendto(socket, "SHUTDOWN", 8, 0, (struct sockaddr*) &clients[i]->in_src, sizeof(clients[i]->in_src));
    }
    else
    {
        sendto(socket, "SHUTDOWN", 8, 0, (struct sockaddr*) &clients[i]->un_src, sizeof(clients[i]->un_src));
    }
    free(clients[i]);
    clients[i] = NULL;
    for (int j=0; j < MAX_PLAYERS/2; j++)
    {
        if (games[j] != NULL && (games[j]->player1 == i || games[j]->player2 == i))
        {
            if (games[j]->player1 == i && clients[games[j]->player2] != NULL)
                shutdown_client(games[j]->player2, clients, clients[games[j]->player2]->type, socket, games);
            else if (clients[games[j]->player1] != NULL)
                shutdown_client(games[j]->player1, clients, clients[games[j]->player1]->type, socket, games);
            break;
        }
    }
}

void run_server(int *net_sock, int *uni_sock, int *running, Client_t **clients, Game_t **games, int *waiting_for_opponent)
{
    while (*running)
    {
        struct pollfd pfds[2];
        pfds[0].fd = *net_sock;
        pfds[0].events = POLLIN;
        pfds[1].fd = *uni_sock;
        pfds[1].events = POLLIN;

        int retval = poll(pfds, 2, -1);
        if (retval == -1)
        {
            return;
        }
        else if (retval)
        {
            if (pfds[0].revents == POLLIN) handle_net_input(&pfds[0].fd, clients, games, waiting_for_opponent);
            if (pfds[1].revents == POLLIN) handle_uni_input(&pfds[1].fd, clients, games, waiting_for_opponent);
        }
    }
}