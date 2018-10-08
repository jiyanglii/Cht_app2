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


#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include "cmdTokenizer.h"
#include "tcp_client.h"

static struct s_cmd input_cmd;

int tcp_client(int c_PORT){

    int head_socket, selret, sock_index;
    struct sockaddr_in server_addr, client_addr;
    fd_set master_list, watch_list;

    int server;
    char log_ip[255];
    char *client_ip = log_ip;

    fgets(log_ip, IP_SIZE, stdin);

    cmdTokenizer(client_ip, &input_cmd);

    server = connect_to_host(client_ip, c_PORT);

    if(server < 0)
        perror("Cannot create socket");

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the listening socket */
    FD_SET(server, &master_list);
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
            for(sock_index=0; sock_index<=head_socket; sock_index++){

                if(FD_ISSET(sock_index, &watch_list)){


                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                        char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

                        memset(cmd, '\0', CMD_SIZE);
                        if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
                            exit(-1);

                        //printf("\nI got: %s\n", cmd);

                        // process command
                        cmdTokenizer(cmd, &input_cmd);
                        c_processCMD(&input_cmd, server);

                        //Process PA1 commands here ...

                        free(cmd);
                    }
                    /* Read from existing server */
                    else{
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
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr); //Convert IP addresses to human-readable form and back
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");

    return fdsocket;
}


void c_processCMD(struct s_cmd * parse_cmd, int fd){
    char *cmd = parse_cmd->cmd;


    if(strcmp(cmd, "IP") == 0){
        char *host_ip;
        char *host_name;
        struct hostent *host_addr;
        struct in_addr **addr_list;
        
        if (gethostname(host_name, IP_SIZE) == 0) {
            host_addr = gethostbyname(host_name);
            addr_list = (struct in_addr **)host_addr->h_addr_list;
            host_ip = inet_ntoa(*addr_list[0]);
            printf("[%s:SUCCESS]\n", cmd);
            printf("IP:%s\n", host_ip);
            printf("[%s:END]\n",cmd);
            fflush(stdout);
        }
        else{
            printf("[%s:ERROR]\n",cmd);
            printf("[%s:END]\n",cmd);
        }
    }
    else if(strcmp(cmd, "AUTHOR") == 0){
        const char my_ubit_name = "jiyangli";
        printf("I, %s, have read and understood the course academic integrity policy.\n", my_ubit_name);
        fflush(stdout);
    }
    else if(strcmp(cmd, "SEND") == 0){
        printf("SEND cmd received\n");

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
    else{
        printf("Invalid command!\n");
    }
}
