/**

 */

#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include "cmdTokenizer.h"
#include "tcp_client.h"

#ifndef __APPLE__
#include "../include/logger.h"
#else
#define cse4589_print_and_log printf
#endif

static struct s_peers users[MAX_CLIENT] = {0};

static struct s_cmd input_cmd;
static uint8_t LOGIN_CMD = false; // This varible sets when recives a LOGIN cmd
static uint8_t LOGIN = false;     // This varible sets when the client is LOGGED IN
static uint8_t INIT_LOGIN  = true;

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
    //printf("\nserver: %d\n", server);

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

                        // printf("\nI got: %s\n", cmd);

                        // process command
                        cmdTokenizer(cmd, &input_cmd);
                        c_processCMD(&input_cmd, server);

                        // Process PA1 commands here ...
                        bzero(&input_cmd, sizeof(struct s_cmd));
                        free(cmd);
                    }
                    /* Read from existing server */
                    else if((sock_index == server) && (server != 0))
                    {
//                        printf("\nLine %d: sock_index: %d\n", __LINE__, sock_index);

                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                            //printf("Remote Host terminated connection!\n");

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                            INIT_LOGIN = true;
                        }
                        else {
                            printf("\nServer sent me: %s\n", buffer);

                            // process command
                            cmdTokenizer(buffer, &input_cmd);
                            c_processCMD_rev(&input_cmd, sock_index);

                            //Process PA1 commands here ...
                            bzero(&input_cmd, sizeof(struct s_cmd));

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
    remote_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(fdsocket, (struct sockaddr *)&remote_server_addr, (socklen_t)sizeof(remote_server_addr)) < 0 )
        perror("Bind failed");

//    printf("Client: local port %08d\n", local_port);

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr); //Convert IP addresses from human-readable to binary
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");

    getsockname(fdsocket, (struct sockaddr*)&remote_server_addr, (socklen_t *)sizeof(struct sockaddr_in));
//    printf("Client: local port %08d\n", htons(remote_server_addr.sin_port));

    return fdsocket;
}

int isValidIP(char * ip){

    char * token;
    char *_ip = (char*) malloc(sizeof(ip)*3);
    memcpy(_ip, ip, strlen(ip));

    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip, &(sa.sin_addr));

    if(result){
        token = strtok(_ip,".");

        if(strcmp(token, "127") == 0){
            result = 0;
        }
    }
    free(_ip);
    return result;
}

int isValidPort(int port){
    if(port == 1087)
        return 0;
    else
        return 1;
}

int isLoggedinUser(char * ip){
    for(int i=0; i<MAX_CLIENT; i++){
        if(strcmp(users[i].ip_str, ip) == 0){
            return 1;
        }
        else if(users[i].port_num == 0)
            break;
    }
    return 0;
}

void GetPrimaryIP(char *cmd) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock <0) {
      perror("Can not create socket!");
    }
    const char*         GoogleIp = "8.8.8.8";
    int                 GooglePort = 53;
    struct              sockaddr_in serv;
    unsigned    char    buffer[20];
    memset(&serv, 0, sizeof(serv));
    serv.sin_family      = AF_INET;
    serv.sin_addr.s_addr = inet_addr(GoogleIp);
    serv.sin_port        = htons(GooglePort);

//connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server
    if(connect(sock,(struct sockaddr*) &serv,sizeof(serv)) <0)
       perror("can not connect");
    else{
        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        if(getsockname(sock, (struct sockaddr *) &name, &namelen) <0)
            perror("can not get host name");

        if(inet_ntop(AF_INET, (const void *)&name.sin_addr, (char *)&buffer[0], 20) < 0) {
            printf("inet_ntop error");
        }
        else {
            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
            cse4589_print_and_log("IP:%s\n", buffer);
            cse4589_print_and_log("[%s:END]\n", cmd);
        }
        close(sock);
    }
}

void c_list(){
    for(int i=0; i<MAX_CLIENT; i++){
        if(users[i].id != 0){
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", users[i].id, users[i].host_name, users[i].ip_str, users[i].port_num);
        }
        else{
            break;
        }
    }
}

void update_user_list(struct s_cmd * parse_cmd){
    int i = 0;
    char * token;
    char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
    memset(msg, '\0', sizeof(char)*MSG_SIZE);

    if(parse_cmd->arg_num == 0)
        return;

    strcat(msg, parse_cmd->arg0);
    if(parse_cmd->arg_num >= 2){
        strcat(msg, " ");
        strcat(msg, parse_cmd->arg1);
    }

    token = strtok(msg, " ");

    memset(&users[0], 0, sizeof(struct s_peers)*MAX_CLIENT);

    while(token && (i < MAX_CLIENT)){

        users[i].id = atoi(token);

        token = (strtok(NULL, " "));
        if(token)
            strcpy(users[i].host_name, token);
        else
            break;

        token = (strtok(NULL, " "));
        if(token)
            strcpy(users[i].ip_str, token);
        else
            break;

        token = (strtok(NULL, " "));
        if(token)
            users[i].port_num = atoi(token);
        else
            break;

        token = strtok(NULL, " ");
        i++;
    }

    free(msg);
}

void c_processCMD_rev(struct s_cmd * parse_cmd, int fd){
    char *cmd = parse_cmd->cmd;

    if((strcmp(cmd, "SEND") == 0) && (parse_cmd->arg_num >= 2)){
        cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
        cse4589_print_and_log("msg from:%s\n[msg]:%s\n", parse_cmd->arg0, parse_cmd->arg1);
        cse4589_print_and_log("[%s:END]\n", "RECEIVED");
    }
    else if(strcmp(cmd, "REFRESH") == 0){
        update_user_list(parse_cmd);
    }

}


void c_processCMD(struct s_cmd * parse_cmd, int fd){
    char *cmd = parse_cmd->cmd;

    if(strcmp(cmd, "IP") == 0){
      GetPrimaryIP(cmd); // call ip();
    }
    else if(strcmp(cmd, "AUTHOR") == 0){
      const char* your_ubit_name = "jiyangli";
      cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
      cse4589_print_and_log("I,%s,have read and understood the course academic integrity policy.\n",your_ubit_name);
      cse4589_print_and_log("[%s:END]\n", cmd);
    }
    else if (strcmp(cmd, "LIST") == 0){
        // Validate destination IP and
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        c_list();
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
    else if((strcmp(cmd, "LOGIN") == 0) && (parse_cmd->arg_num >= 2))
    {
        if(!(isValidIP(parse_cmd->arg0) && isValidPort(atoi(parse_cmd->arg1)))){
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
            cse4589_print_and_log("[%s:END]\n", cmd);
            return;
        }
        if(INIT_LOGIN){
            // This is ran by start up, first to establish connection to server
            server = connect_to_host(parse_cmd->arg0, atoi(parse_cmd->arg1));

            if(server < 0){
                cse4589_print_and_log("[%s:ERROR]\n", cmd);
                perror("Cannot create socket");
                cse4589_print_and_log("[%s:END]\n", cmd);
            }
            else{
                cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                /* Register the listening socket */
                FD_SET(server, &master_list);
                if(server > head_socket) head_socket = server;
                LOGIN = true;
                INIT_LOGIN = false;
                cse4589_print_and_log("[%s:END]\n", cmd);
            }
        }
        else if(LOGIN == true){
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
            printf("Client already logged in, please log out first!\n");
            cse4589_print_and_log("[%s:END]\n", cmd);
        }
        else if(LOGIN == false){
            // Re log in after logged out
            char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
            memset(msg, '\0', MSG_SIZE);

            msg = concatCMD(msg, parse_cmd);
            //printf("ConcatCMD %s\n", msg);

            if(send(fd, msg, (strlen(msg)), 0) == strlen(msg)){
                cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                cse4589_print_and_log("[%s:END]\n", cmd);
                LOGIN = true;
            }
            fflush(stdout);
            free(msg);
        }
    }
    else if(strcmp(cmd, "LOGOUT") == 0){
        if(LOGIN == false){
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
            printf("Client not logged in, please log in first!\n");
            cse4589_print_and_log("[%s:END]\n", cmd);
        }
        
        else{
                if(send(fd, LOGOUT, (strlen(LOGOUT)), 0) == strlen(LOGOUT)){
                    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                    LOGIN = false;
                    cse4589_print_and_log("[%s:END]\n", cmd);
                }
        }
    }
    else if((strcmp(cmd, "SEND") == 0) && (parse_cmd->arg_num >= 2)){
        if(!(isValidIP(parse_cmd->arg0) && isLoggedinUser(parse_cmd->arg0))){
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
            cse4589_print_and_log("[%s:END]\n", cmd);
            return;
        }

        char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
        memset(msg, '\0', MSG_SIZE);

        msg = concatCMD(msg, parse_cmd);
//        printf("ConcatCMD %s\n", msg);

        if(send(fd, msg, (strlen(msg)), 0) == strlen(msg))
//            printf("Sent!\n");
            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        else{
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
        }
        cse4589_print_and_log("[%s:END]\n", cmd);
//        fflush(stdout);
        free(msg);
    }
    else if(((strcmp(cmd, "BROADCAST") == 0) && (parse_cmd->arg_num >= 1))
            ||((strcmp(cmd, "REFRESH") == 0) && (parse_cmd->arg_num >= 0))){
        // For cmds with args, check arg number before accessing it to ensure security, add BROADCAST function

        char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
        memset(msg, '\0', MSG_SIZE);

        msg = concatCMD(msg, parse_cmd);
//        printf("ConcatCMD %s\n", msg);

        if(send(fd, msg, (strlen(msg)), 0) == strlen(msg)){
            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
            cse4589_print_and_log("[%s:END]\n", cmd);
        }
//            printf("Sent!\n");
        else{
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
            cse4589_print_and_log("[%s:END]\n", cmd);
        }
//        fflush(stdout);
        free(msg);
    }
    else if(strcmp(cmd, "PORT") == 0){
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("PORT:%d\n", local_port);
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
    else if(strcmp(cmd, EXIT) == 0){
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
        cse4589_print_and_log("[%s:END]\n", cmd);
        
        if(LOGIN == false){
                send(fd, LOGOUT, (strlen(LOGOUT)), 0);
                send(fd, EXIT, (strlen(EXIT)), 0);
                usleep(100);
                LOGIN = false;
                exit(0);
        }
        else{
                send(fd, EXIT, (strlen(EXIT)), 0);
                LOGIN = false;
                exit(0);
        }
    }
    else if(strcmp(cmd, "") == 0){
        // This handles empty cmd, do nothing and no error
        printf("\n");
    }
    else if(((strcmp(cmd, "BLOCK") == 0) && (parse_cmd->arg_num == 1)) ||
        ((strcmp(cmd, "UNBLOCK") == 0) && (parse_cmd->arg_num == 1))){
        if(!isValidIP(parse_cmd->arg0)){
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
            cse4589_print_and_log("[%s:END]\n", cmd);
            return;
        }

        char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
        memset(msg, '\0', MSG_SIZE);
        msg = concatCMD(msg, parse_cmd);

        if(send(fd, msg, (strlen(msg)), 0) == strlen(msg)){
            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
            cse4589_print_and_log("[%s:END]\n", cmd);
        }
        else{
            cse4589_print_and_log("[%s:ERROR]\n", cmd);
            cse4589_print_and_log("[%s:END]\n", cmd);
        }
        free(msg);
    }
    else{
        cse4589_print_and_log("[%s:ERROR]\n", cmd);
        printf("Invalid command!\n");
        cse4589_print_and_log("[%s:END]\n", cmd);
    }
}
