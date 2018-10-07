/**
 * @client
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file contains the client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

#include "tcp_client.h"
#include "tcp_server.h"

#define TRUE 1
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
            printf("[%s:SUCCESS]\n", *argv);

            tcp_client(atoi(argv[2]));
        }
        else if(strcmp("s", argv[1]) == 0) {
            printf("[%s:SUCCESS]\n", *argv);

            tcp_server(atoi(argv[2]));
        }
        else
            printf("PLEASE INPUT c/s\n");
    }
}


