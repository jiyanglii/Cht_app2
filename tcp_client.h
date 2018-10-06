
#pragma once

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define IP_SIZE 255


int tcp_client(int c_PORT);
int connect_to_host(char *server_ip, int server_port);