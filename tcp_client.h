
#pragma once

#define BACKLOG     5
#define STDIN       0
#define CMD_SIZE    512
#define BUFFER_SIZE 512
#define MSG_SIZE    512

#define IP_SIZE     255
#define LOGOUT      "LOGOUT"
#define EXIT        "EXIT"

#ifndef MAX_CLIENT
#define MAX_CLIENT  0xff
#endif

#ifndef TRUE
#define TRUE        true
#endif

struct s_peers{
    int id;
    char host_name[35];
    char ip_str[INET6_ADDRSTRLEN];
    int port_num;
};

int listening();
int tcp_client(int c_PORT);
int connect_to_host(char *server_ip, int server_port);
int isValidIP(char * ip);

struct s_cmd;
void c_processCMD(struct s_cmd * parse_cmd, int fd);
void c_processCMD_rev(struct s_cmd * parse_cmd, int fd);
void GetPrimaryIP(char *cmd);
