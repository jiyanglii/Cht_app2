/**

 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

#include "tcp_client.h"
#include "tcp_server.h"

#ifndef __APPLE__
#include "../include/logger.h"
#else
#define cse4589_print_and_log printf
#endif

#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define IP_SIZE 255

int main(int argc, char **argv){

    if(argc != 3) {
        printf("[%s:ERROR]\n", *argv);
    }
    else
    {
        if (strcmp("c", argv[1]) == 0) {
            cse4589_print_and_log("[%s:SUCCESS]\n", *argv);

            tcp_client(atoi(argv[2]));
        }
        else if(strcmp("s", argv[1]) == 0) {
            cse4589_print_and_log("[%s:SUCCESS]\n", *argv);

            tcp_server(atoi(argv[2]));
        }
        else
            printf("PLEASE INPUT c/s\n");
    }
}


