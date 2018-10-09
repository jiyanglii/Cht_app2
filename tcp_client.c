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

#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "cmdTokenizer.h"
#include "tcp_client.h"

static struct s_cmd input_cmd;
static uint8_t LOGIN_CMD = false; // This varible sets when recives a LOGIN cmd
static uint8_t LOGIN = false;     // This varible sets when the client is LOGGED IN

static fd_set master_list, watch_list;
static int server = 0;
static int head_socket = 0;

static int local_port;

int tcp_client(int c_PORT){

    int selret, sock_index;
    struct sockaddr_in server_addr, client_addr;

    char log_ip[255];
    char *client_ip = log_ip;

    local_port = c_PORT;

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server;
    printf("\nserver: %d\n", server);

    while(TRUE){
        memcpy(&watch_list, &master_list, sizeof(master_list));

        //printf("\n[PA1-Server@CSE489/589]$ ");
        //fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if(selret > 0){
            /* Loop through socket descriptors to check which ones are ready */
            for(sock_index = 0; sock_index<=head_socket; sock_index++){
                printf("TEST PRINT sock_index: %d\n", sock_index);

                if(FD_ISSET(sock_index, &watch_list)){


                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                        char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

                        memset(cmd, '\0', CMD_SIZE);
                        if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
                            exit(-1);

                        printf("\nI got: %s\n", cmd);

                        // process command
                        cmdTokenizer(cmd, &input_cmd);
                        c_processCMD(&input_cmd, server);

                        //Process PA1 commands here ...
                        bzero(&input_cmd, sizeof(struct s_cmd));
                        free(cmd);
                    }
                    /* Read from existing server */
                    else if((sock_index == server) && (server != 0))
                    {
                        printf("\nLine %d: sock_index: %d\n", __LINE__, sock_index);

                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) < 0){
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");

                            // Remove client from client list

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                            printf("\nServer sent me: %s\n", buffer);
                            fflush(stdout);
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}


int connect_to_host(char *server_ip, int server_port)
{
    int fdsocket, len;
    struct sockaddr_in remote_server_addr;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdsocket < 0)
        perror("Failed to create socket");

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    remote_server_addr.sin_port = htons(local_port);
    printf("TEST PRINT remote_server_addr.sin_port: %d\n", remote_server_addr.sin_port);
    printf("TEST PRINT local_port: %d\n", local_port);

    remote_server_addr.sin_addr.s_addr = htons(INADDR_ANY);

    if(bind(fdsocket, (struct sockaddr *)&remote_server_addr, (socklen_t)sizeof(remote_server_addr)) < 0 )
        perror("Bind failed");

    printf("Client: local port %08d\n", local_port);

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr); //Convert IP addresses from human-readable to binary
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");

    getsockname(fdsocket, (struct sockaddr*)&remote_server_addr, (socklen_t *)sizeof(struct sockaddr_in));
    printf("Client: local port %08d\n", htons(remote_server_addr.sin_port));

    return fdsocket;
}

void c_processCMD(struct s_cmd * parse_cmd, int fd){
    char *cmd = parse_cmd->cmd;

    if(strcmp(cmd, "IP") == 0){
                    // call ip();
    }
    else if(strcmp(cmd, "LOGIN") == 0)
    {
        printf("LOGIN cmd recieved\n");

        if(LOGIN == true){
            printf("Client already logged in, please log out first!\n");
        }
        else{
            server = connect_to_host(parse_cmd->arg0, atoi(parse_cmd->arg1));
            if(server < 0)
                perror("Cannot create socket");
            else{
                /* Register the listening socket */
                FD_SET(server, &master_list);
                if(server > head_socket) head_socket = server;
                LOGIN = true;
            }
        }
    }
    else if(strcmp(cmd, "LOGOUT") == 0){
        printf("LOGOUT cmd recieved\n");
        if(LOGIN == false){
            printf("Client not logged in, please log in first!\n");
        }
        else{
                if(send(fd, LOGOUT, (strlen(LOGOUT)), 0) == strlen(LOGOUT))
                    printf("Sent!\n");
                LOGIN = false;
        }
    }
    else if((strcmp(cmd, "SEND") == 0) && (parse_cmd->arg_num == 2)){ // For cmds with args, check arg number before accessing it to ensure security
        printf("SEND cmd revieved\n");
        char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
        memset(msg, '\0', MSG_SIZE);

        strcpy(msg, parse_cmd->arg0);
        strcat(msg, " ");
        strcat(msg, parse_cmd->arg1);

        if(send(fd, msg, (strlen(msg)), 0) == strlen(msg))
            printf("Sent!\n");
        fflush(stdout);
        free(msg);
    }
    else if(strcmp(cmd, "PORT") == 0){
        printf("PORT:%d\n", local_port);
    }
    else{
        printf("Invalid command!\n");
    }
}
