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


#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define IP_SIZE 255


int connect_to_host(char *server_ip, int server_port);

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int tcp_client(int c_PORT){
    int server; //When choose Client only, how to combine the server from the internet outside
    char log_ip[255];
    char *client_ip = log_ip;
    
    fgets(log_ip, IP_SIZE, stdin);
    
    server = connect_to_host(client_ip, c_PORT);
    
    while (TRUE) {
        printf("\n[PA1-Client@CSE489/589]$ ");
        fflush(stdout);
        
        char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
        memset(msg, '\0', MSG_SIZE);
        if(fgets(msg, MSG_SIZE-1, stdin) == NULL)
            exit(-1);
        
        printf("\nSENDing to the remote server: %s(size:%d chars)", msg, strlen(msg));
        
        if(send(msg, server, strlen(msg), 0) == strlen(msg))
            printf("Done!/n");
        fflush(stdout);
    }

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
