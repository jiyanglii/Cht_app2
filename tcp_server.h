
#pragma once

#ifndef TRUE
#define TRUE        true
#endif

#define BACKLOG     5
#define STDIN       0
#define CMD_SIZE    100
#define BUFFER_SIZE 256
#define MSG_SIZE    256
#define MAX_CLIENT  0xff
#define MAX_MSG_BUFFER 100

#define LOGOUT      "LOGOUT"
#define EXIT        "EXIT"

#define LOGGED_IN       1
#define LOGGED_OUT      0

#define loggedin        "logged-in"
#define loggedout       "logged-out"

struct s_client{
    int client_id;
    bool status;
    char host_name[35];
    uint32_t ip;
    char ip_str[INET6_ADDRSTRLEN];
    int port_num;
    int msg_rev;
    int msg_sent;
    int fd;
    struct sockaddr_in client_info;
    char *buffer[MAX_MSG_BUFFER];
    int block_by[MAX_CLIENT];
};

int tcp_server(int s_PORT);
int forward();
int logout();
int new_client(int fd, struct sockaddr*);
void *get_in_addr(struct sockaddr *sa);
void client_list_sort();

struct s_cmd;
void processCMD(struct s_cmd * parse_cmd);
void GetPrimaryIP(char *cmd);

int find_client_by_ip(char * ip);
int find_client_by_fd(int fd);
void remove_client_by_fd(int fd);
