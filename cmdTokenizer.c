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

#define DEBUG

void cmdTokenizer(char *input, struct s_cmd * parse_cmd){

    // parse input cmd, initialize new buffer for cmd and each arges, and pass the pointers to parse_cmd as returned parsed cmd
    char *token;
    const char delim = ' ';

    parse_cmd->arg_num = 0;

    /* get the first token */
    token = strtok(input, &delim);
    parse_cmd->cmd = token;

#ifdef DEBUG
    printf("Line %d: %s\n",  __LINE__, parse_cmd->cmd);
#endif

    /* walk through other tokens */
    if( token != NULL ) {

        token = strtok(NULL, &delim);
        parse_cmd->arg0 = token;

        if(parse_cmd->arg0)
            parse_cmd->arg_num ++;

#ifdef DEBUG
        printf("Line %d: %s\n",  __LINE__, parse_cmd->arg0);
#endif
   }

    if( token != NULL ) {

        token = strtok(NULL, &delim);
        parse_cmd->arg1 = token;

        if(parse_cmd->arg1)
            parse_cmd->arg_num ++;

#ifdef DEBUG
        printf("Line %d: %s\n",  __LINE__, parse_cmd->arg1);
#endif
   }

#ifdef DEBUG
    printf("Line %d: Number of atgs in this cmd: %d\n",  __LINE__, parse_cmd->arg_num);
#endif

}