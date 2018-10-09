
#pragma once

#define BACKLOG     5
#define STDIN       0
#define TRUE        1
#define CMD_SIZE    100
#define BUFFER_SIZE 256
#define MSG_SIZE    256
#define BUFFER_SIZE 256
#define IP_SIZE     255


int tcp_client(int c_PORT);
int connect_to_host(char *server_ip, int server_port);
void c_processCMD(struct s_cmd * parse_cmd, int fd);
