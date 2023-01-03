#include "game.h"

void match_opponents(int skt, int p1, int p2, Game_t **games, Client_t **clients);
int init_sockets(int *net_sock, int *uni_sock, int port, char* path);
void run_server(int *net_sock, int *uni_sock, int *running, Client_t **clients, Game_t **games, int *waiting_for_opponent);
void handle_net_input(int *socket, Client_t **clients, Game_t **games, int* waiting_for_opponent);
void handle_uni_input(int *socket, Client_t **clients, Game_t **games, int* waiting_for_opponent);
void shutdown_client(int i, Client_t **clients, int type, int socket, Game_t **games);
void end_game(int game_no, Game_t **games);
void init_client(Client_t **clients, struct sockaddr_in *net_sa, struct sockaddr_un *un_sa, char* name, int *waiting_for_opponent, int skt, Game_t **games);