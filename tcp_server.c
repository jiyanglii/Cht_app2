/**
 * @server
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
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "cmdTokenizer.h"
#include "tcp_server.h"
#include "logger.h"

static struct s_client client_list[MAX_CLIENT] = {0};
static int client_count = 1;
static struct s_cmd input_cmd;

int tcp_server(int s_PORT){

    int port, server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
    struct sockaddr_in server_addr, client_addr;
    fd_set master_list, watch_list;

    /* Socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0); /* socket - create an endpoint for communication
                                                      AF_INET Internet domain sockets
                                                      0 causes socket() to use an unspecified default protocol appropriate for the requested socket type.*/

    if(server_socket < 0)
        perror("Cannot create socket");

    /* Fill up sockaddr_in struct */
    port = s_PORT;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY:  the socket accepts connections to all the IPs of the machine.
    server_addr.sin_port = htons(port);

    /* Bind */
    if(bind(server_socket, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr)) < 0 )

        perror("Bind failed");

    /* Listen */
    if(listen(server_socket, BACKLOG) < 0)
        perror("Unable to listen on port");

    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;
    printf("\nserver_socket: %d\n", server_socket);

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

                        printf("\nI got: %s\n", cmd);

                        // process command
                        cmdTokenizer(cmd, &input_cmd);
                        processCMD(&input_cmd);

                        //Process PA1 commands here ...

                        free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
                        printf("\nLine %d: sock_index: %d\n", __LINE__, sock_index);
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&caddr_len);
                        if(fdaccept < 0)
                            perror("Accept failed.");

                        printf("\nRemote Host connected!\n");

                        // Add new client to client list
                        if(new_client(fdaccept,(struct sockaddr *)&client_addr))
                            printf("\nAdd to client list failed!\n");

                        // Braodcast new client info

                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_socket) head_socket = fdaccept;
                    }
                    /* Read from existing clients */
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
                            //Process incoming data from existing clients here ...

                            printf("\nClient sent me: %s\n", buffer);
                            printf("ECHOing it back to the remote host ... ");

                            // Extract destination ip from the incoming msg
                            //dest_ip_str = buffer;

                            // Validate destination IP and
                            if(forward(buffer))
                                printf("Message forwarding failed\n");
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}

int forward(char * buffer){

    char dest_ip_str[INET_ADDRSTRLEN];
    cmdTokenizer(buffer, &input_cmd);

    memcpy(&dest_ip_str[0],input_cmd.cmd,sizeof(dest_ip_str));

    for(int i = 0; i<MAX_CLIENT; i++){

        if ((client_list[i].fd != 0) && (strcmp(client_list[i].ip_str, input_cmd.cmd) == 0) && (client_list[i].status == LOGGED_IN)){
            printf("\nClient found!\n");

            // Destination client exits in the client list, forward the msg
            if(send(client_list[i].fd, input_cmd.arg0, (strlen(input_cmd.arg0)), 0) == (strlen(input_cmd.arg0)))
                printf("Done!\n");
            fflush(stdout);
            break;
        }
        else if(client_list[i].fd == 0){
            break;
        }
    }
    return 0;

}

int new_client(int new_fd, struct sockaddr * client_sock){

    char s[INET_ADDRSTRLEN];
    struct sockaddr_in client_sock_in = *(struct sockaddr_in *)client_sock;

    for(int i = 0; i<MAX_CLIENT; i++)
    {
        if (client_list[i].fd == 0)
        {

            inet_ntop(client_sock->sa_family,get_in_addr(client_sock), s , sizeof(s));
            printf("server: got connection from ip %s\n", s);
            printf("server: got connection from port %08d\n", htons(client_sock_in.sin_port));


            printf("%d\n", *(uint32_t *)get_in_addr(client_sock));

            client_list[i].client_id = client_count;
            client_list[i].status = LOGGED_IN;
            client_list[i].ip = *(uint32_t *)get_in_addr(client_sock);
            memcpy(&client_list[i].ip_str[0],&s[0],sizeof(s));
            //client_list[i].ip_str = s;
            client_list[i].port_num = htons(client_sock_in.sin_port);
            client_list[i].fd = new_fd;
            client_list[i].client_info = client_sock_in;

            printf("getpeername:%d\n", getpeername(new_fd, client_sock, (socklen_t *)sizeof(struct sockaddr)));
            //printf("gethostname:%d\n", gethostname(fd, &client_sock, sizeof(client_sock)));

            client_count++;
            break;
        }
    }
    return 0;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void processCMD(struct s_cmd * parse_cmd){
    char *cmd = parse_cmd->cmd;


    if(strcmp(cmd, "IP") == 0){
                    // call ip();
    }
    //else if ......
    else{
        printf("Invalid command!\n");
    }
}
