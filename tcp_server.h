
#pragma once

#define BACKLOG     5
#define STDIN       0
#define TRUE        1
#define CMD_SIZE    100
#define BUFFER_SIZE 256
#define MSG_SIZE    256
#define MAX_CLIENT  0xff

#define LOGGED_IN       1
#define LOGGED_OUT      0


struct s_client{
    int client_id;
    char status;
    char host_name[20];
    uint32_t ip;
    char ip_str[INET6_ADDRSTRLEN];
    int port_num;
    int msg_rev;
    int msg_sent;
    int fd;
    struct sockaddr_in client_info;
};

int tcp_server(int s_PORT);
int forward();
int new_client(int fd, struct sockaddr*);
void *get_in_addr(struct sockaddr *sa);
void processCMD(struct s_cmd * parse_cmd);


int find_client_by_ip(char * ip);
int find_client_by_fd(int fd);