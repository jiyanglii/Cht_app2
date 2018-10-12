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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "cmdTokenizer.h"
#include "tcp_server.h"

#ifndef __APPLE__
#include "../include/logger.h"
#else
#define cse4589_print_and_log printf
#endif

static struct s_client client_list[MAX_CLIENT] = {0};
static int client_count = 1;
static struct s_cmd input_cmd;

static int s_local_port = 0;

static fd_set master_list, watch_list;
static int sock_index = 0;
static int head_socket = 0;

int tcp_server(int s_PORT){

    int port, server_socket, selret, fdaccept=0, caddr_len;
    struct sockaddr_in server_addr, client_addr;

    /* Socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0); /* socket - create an endpoint for communication
                                                      AF_INET Internet domain sockets
                                                      0 causes socket() to use an unspecified default protocol appropriate for the requested socket type.*/

    if(server_socket < 0)
        perror("Cannot create socket");

    /* Fill up sockaddr_in struct */
    s_local_port = s_PORT;
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
//    printf("\nserver_socket: %d\n", server_socket);

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
//                        printf("\nI got: %s\n", cmd);

                        // process command
                        cmdTokenizer(cmd, &input_cmd);
                        processCMD(&input_cmd);

                        //Process PA1 commands here ...
                        bzero(&input_cmd, sizeof(struct s_cmd));
                        free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
//                        printf("\nLine %d: sock_index: %d\n", __LINE__, sock_index);
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&caddr_len);
                        if(fdaccept < 0){
                            perror("Accept failed.");
                        }
                        

                        //printf("\nRemote Host connected!\n");

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
//                        printf("\nLine %d: sock_index: %d\n", __LINE__, sock_index);
                        /* Initialize buffer to receieve response */
                        char *buffer= (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) < 0){
                            printf("Remote Host terminated connection!\n");
                            close(sock_index);

                            // Remove client from client list

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                            //Process incoming data from existing clients here ...

                            printf("\nClient sent me: %s\n", buffer);

                            int src_ip;
                            src_ip = client_addr.sin_addr.s_addr;
                            printf("\nClient send ip_address is: %d\n", src_ip);

                            cmdTokenizer(buffer, &input_cmd);
                            processCMD(&input_cmd);

                            //Process PA1 commands here ...
                            bzero(&input_cmd, sizeof(struct s_cmd));
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}

int forward(){

    int id_dst, id_src  = -1;
    char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
    memset(msg, '\0', MSG_SIZE);

    char dest_ip_str[INET_ADDRSTRLEN];

    memcpy(&dest_ip_str[0],input_cmd.arg0,sizeof(dest_ip_str));

    id_dst = find_client_by_ip(input_cmd.arg0);

    if(id_dst != -1){

        id_src = find_client_by_fd(sock_index);
        client_list[id_src].msg_sent++;

        memcpy(&dest_ip_str[0],client_list[id_src].ip_str,sizeof(dest_ip_str));

        input_cmd.arg0 = &dest_ip_str[0];

        msg = concatCMD(msg, &input_cmd);

        if (client_list[id_dst].status == LOGGED_IN){

            client_list[id_dst].msg_rev++;
            // When logged in, directly forward msg, find out incoming client by its FD

            if(send(client_list[id_dst].fd, msg, (strlen(msg)), 0) == strlen(msg))
                printf("Sent!\n");
            fflush(stdout);
        }
        else{
            // Client not logged in, buffer the msg
//            printf("Buffering msg!\n");

            // Find a spot in the buffer
            for(int j = 0; j < MAX_MSG_BUFFER; j++){
//                printf("%s\n", client_list[id_dst].buffer[j]);
                if(!client_list[id_dst].buffer[j]){
                    client_list[id_dst].buffer[j] = (char*) malloc(sizeof(char)*(strlen(msg)));
                    memcpy(client_list[id_dst].buffer[j], msg, sizeof(char)*(strlen(msg)));

//                    printf("Buffered msg: %s\n", client_list[id_dst].buffer[j]);

                    break;
                }
            }
        }


    }
    else{
        // Not client in the list
    }
    free(msg);
    return 0;
}


int login(){
    int id_src  = -1;

    id_src = find_client_by_fd(sock_index);

    if(id_src != -1){
        client_list[id_src].status = LOGGED_IN;

        //Send all buffered msg now!
        for(int j = 0; j < MAX_MSG_BUFFER; j++){
            if(client_list[id_src].buffer[j] != NULL){
                if(send(client_list[id_src].fd, client_list[id_src].buffer[j], (strlen(client_list[id_src].buffer[j])), 0) == strlen(client_list[id_src].buffer[j]))
                    printf("Buffered msg sent!\n");
                fflush(stdout);
                free(client_list[id_src].buffer[j]);
                client_list[id_src].buffer[j] = NULL;
                client_list[id_src].msg_rev++;
            }
            else{
                // Reach the end
                break;
            }
        }
    }
    else{
        // This case should NEVER happen!
        printf("Client not found! Creating new client!\n");
    }
    return 0;
}

int logout(){

    int id_src  = -1;

    id_src = find_client_by_fd(sock_index);

    if(id_src != -1){
        client_list[id_src].status = LOGGED_OUT;
    }
    else
        printf("Client not found!\n");
    return 0;
}

int find_client_by_ip(char * ip){

    int idx = -1;

    for(int i = 0; i<MAX_CLIENT; i++){

        if ((client_list[i].fd != 0) && (strcmp(client_list[i].ip_str, ip) == 0)){
//            printf("\nClient found!\n");

            idx = i;

            break;
        }
        else if(client_list[i].fd == 0){
            break;
        }
    }

    return idx;
}

int find_client_by_fd(int fd){
    int idx = -1;

    for(int i = 0; i<MAX_CLIENT; i++){

        if ((client_list[i].fd != 0) && (client_list[i].fd == fd)){
//            printf("\nClient found!\n");

            idx = i;

            break;
        }
        else if(client_list[i].fd == 0){
            break;
        }
    }

    return idx;
}

void remove_client_by_fd(int fd){

    int idx = find_client_by_fd(fd);

    // Wipe out the client
    memset(&client_list[idx], 0, sizeof(struct s_client));
    client_list[idx].fd = 0;

    // Re order the client list to fill in the hole
    for(int i = 0; i<(MAX_CLIENT - 1); i++){

        if ((client_list[i].fd == 0) && (client_list[i+1].fd != 0)){
            client_list[i] = client_list[i+1];
            //memcpy(&client_list[i], &client_list[i+1], sizeof(struct s_client));
            memset(&client_list[i+1], 0, sizeof(struct s_client));
            client_list[i+1].fd = 0;
        }
        else if ((client_list[i].fd == 0) && (client_list[i+1].fd == 0)){
            // Break when two consecutive clients are empty
            break;
        }
    }
}

int new_client(int new_fd, struct sockaddr * client_sock){

    char s[INET_ADDRSTRLEN];
    struct sockaddr_in client_sock_in = *(struct sockaddr_in *)client_sock;


    for(int i = 0; i<MAX_CLIENT; i++)
    {
        if (client_list[i].fd == 0)
        {

            inet_ntop(client_sock->sa_family,get_in_addr(client_sock), s , sizeof(s));
//            printf("server: got connection from ip %s\n", s);
//            printf("server: got connection from port %08d\n", htons(client_sock_in.sin_port));
            getnameinfo((struct sockaddr *)&client_sock_in, sizeof(client_sock_in), client_list[i].host_name, sizeof(client_list[i].host_name), NULL, 0, 0);

            //printf("host: %s\n", client_list[i].host_name);
            //printf("service: %s\n", service);


            //printf("%d\n", *(uint32_t *)get_in_addr(client_sock));

            client_list[i].client_id = client_count;
            client_list[i].status = LOGGED_IN;
            client_list[i].ip = *(uint32_t *)get_in_addr(client_sock);
            memcpy(&client_list[i].ip_str[0],&s[0],sizeof(s));
            //client_list[i].ip_str = s;
            client_list[i].port_num = htons(client_sock_in.sin_port);
            client_list[i].fd = new_fd;
            client_list[i].client_info = client_sock_in;

            client_list[i].msg_rev = 0;
            client_list[i].msg_sent = 0;

            //printf("getpeername:%d\n", getpeername(new_fd, client_sock, (socklen_t *)sizeof(struct sockaddr)));
            //printf("gethostname:%d\n", gethostname(fd, &client_sock, sizeof(client_sock)));

            for(int j = 0; j < MAX_MSG_BUFFER; j++){
                client_list[i].buffer[j] = NULL;
                }

            client_count++;
            break;
        }
    }

    // Re-order client list
    client_list_sort();

    return 0;
}

void statistics(){
    for(int i = 0; i<MAX_CLIENT; i++){

        if (client_list[i].fd != 0){
            //printf("\nClient found!\n");
            if(client_list[i].status)
                cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i+1, client_list[i].host_name, client_list[i].msg_sent, client_list[i].msg_rev, loggedin);
            else
                cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i+1, client_list[i].host_name, client_list[i].msg_sent, client_list[i].msg_rev, loggedout);
        }
        else if(client_list[i].fd == 0){
            break;
        }
    }
}

void broadcast(struct s_cmd * cmd) {
    char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);

    memset(msg, '\0', sizeof(char)*MSG_SIZE);
    memcpy(msg, cmd->arg0, strlen(cmd->arg0));

    if(cmd->arg_num >= 2){
        strcat(msg, " ");
        strcat(msg, cmd->arg1);
    }

    for(int i = 0; i < MAX_CLIENT; i++) {
        if((client_list[i].fd != 0) && (client_list[i].fd != sock_index)) {
            send(client_list[i].fd, msg, (strlen(msg)),0);
        }
        else {
            break;
        }
    }
    free(msg);
}

void refresh() {
    char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
//    char info[255] = "the ip is:";

    for(int i = 0; i < MAX_CLIENT; i++) {
        if((client_list[i].fd == sock_index)&&(client_list[i].fd == sock_index)) {
           for(int j = 0; j < MAX_CLIENT; j++) {
                   strcat(msg,"the ip is:");
                   strcat(msg,"client_list[i].ip");
                   strcat(msg,"the ip port is:");
                   strcat(msg,"client_list[i].port_num");
               if(client_list[j].fd != 0) {
                  send(client_list[i].fd, msg, (strlen(msg)),0);
               }
           }
           printf("REFRESH DONE!\n");
        }
        else {
            break;
        }
    }

    free(msg);
}

void sort(int arr_a[], uint32_t arr_b[], int len) {
   int      tmp;
   uint32_t tmp_c;
   for(int i = 0; i < len -1; i++) {
       for(int j =0; j < len -1 -i; j++) {
           if(arr_a[j] > arr_a[j + 1]) {
              tmp        = arr_a[j];
              arr_a[j]   = arr_a[j +1];
              arr_a[j+1] = tmp;

              tmp_c      = arr_b[j];
              arr_b[j]   = arr_b[j +1];
              arr_b[j+1] = tmp_c;
           }
       }
   }
}

void client_swap(struct s_client *a, struct s_client *b){
    struct s_client temp = {0};
    temp = *a;
    *a = *b;
    *b = temp;
}

void client_list_sort(){
    bool swapped = false;

    // Max O(n*n)
    for(int j = 0; j<MAX_CLIENT*MAX_CLIENT; j++){
        for(int i = 0; i<(MAX_CLIENT - 1); i++){
            if ((client_list[i].port_num > client_list[i+1].port_num) &&
                (client_list[i].fd != 0) && (client_list[i+1].fd != 0))
            {
                client_swap(&client_list[i], &client_list[i+1]);
                swapped = true;
            }
            else if ((client_list[i].fd == 0) || (client_list[i+1].fd == 0))
                break;
        }
        if(swapped)
            swapped = false;
        else
            break;
    }
}


void list(){

    for(int i = 0; i<MAX_CLIENT; i++){

        if ((client_list[i].fd != 0) && (client_list[i].status == LOGGED_IN)){
            //printf("\nClient found!\n");
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i+1, client_list[i].host_name, client_list[i].ip_str, client_list[i].port_num);
        }
        else if(client_list[i].fd == 0){
            break;
        }
    }
}

/*
void list() {
    int       arr_a[MAX_CLIENT-1];
    uint32_t  arr_b[MAX_CLIENT-1];
    int       arr_c[MAX_CLIENT-1];
    int count = 0;
    for(int i = 0; i < MAX_CLIENT; i++) {
        if(client_list[i].fd != 0) {
           arr_a[i] = client_list[i].port_num;
           arr_b[i] = client_list[i].ip_str;
           arr_c[i] = i;
           count++;
        }
        else{
            sort(arr_a,arr_b,count);
            for(int k = 0; k < count; k++) {
                cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", k, "hostname", arr_b[k], arr_a[k]);
                //printf("num_sequence:%d, ip_addr:%04x, ip_port:%d\n",k,arr_b[k],arr_a[k]);
            }
            break;
        }
    }
}*/

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void processCMD(struct s_cmd * parse_cmd){
    char *cmd    = parse_cmd->cmd;
    char *buffer = parse_cmd->arg0;

    if(strcmp(cmd, "IP") == 0){
        GetPrimaryIP(cmd); // call ip();
    }
    else if(strcmp(cmd, "AUTHOR") == 0){
        const char* your_ubit_name = "jiyangli and yincheng";
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("I,%s,have read and understood the course academic integrity policy.\n",your_ubit_name);
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
    else if(strcmp(cmd, "PORT") == 0){
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("PORT:%d\n", s_local_port);
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
    else if (strcmp(cmd, "LOGOUT") == 0){
        logout();
    }
    else if (strcmp(cmd, "LOGIN") == 0){
        // Here handles when a client already in the list, but logged out, re log in here
        // New client case is handles elsewhere
        login();
    }
    else if (strcmp(cmd, "SEND") == 0){
        // Validate destination IP and
        if(forward())
            printf("Message forwarding failed\n");
    }
    else if (strcmp(cmd, EXIT) == 0){
        if(sock_index == STDIN){
            // EXIT server
            for(int i = 3; i<=head_socket; i++){
                shutdown(i, SHUT_WR);
                usleep(20);
                close(i);
                FD_CLR(i, &master_list);
            }
            head_socket = 2;
            exit(0);
        }
        else{ // A client is trying to EXIT
            // Do log out first
            logout();
            FD_CLR(sock_index, &master_list);
            // Remove client from client list
            remove_client_by_fd(sock_index);
        }
    }
    else if (strcmp(cmd, "LIST") == 0){
        // Validate destination IP and
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        list();
        cse4589_print_and_log("[%s:END]\n", cmd);

    }
    else if (strcmp(cmd, "BROADCAST") == 0){
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        broadcast(parse_cmd);
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
    else if (strcmp(cmd, "REFRESH") == 0){
        printf("refresh step 0\n");
        refresh();
    }
    else if (strcmp(cmd, "STATISTICS") == 0){
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        statistics();
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
    else if(strcmp(cmd, "") == 0){
        // This handles empty cmd, do nothing and no error
        printf("\n");
    }
    else{
        cse4589_print_and_log("[%s:ERROR]\n", cmd);
        printf("Invalid command!\n");
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
}

