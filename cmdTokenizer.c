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

void cmdTokenizer(char *input, struct s_cmd * parse_cmd){

    // parse input cmd, initialize new buffer for cmd and each arges, and pass the pointers to parse_cmd as returned parsed cmd
    char *token;
    const char delim = ' ';

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

#ifdef DEBUG
        printf("Line %d: %s\n",  __LINE__, parse_cmd->arg0);
#endif
   }

    if( token != NULL ) {

        token = strtok(NULL, &delim);
        parse_cmd->arg1 = token;

#ifdef DEBUG
        printf("Line %d: %s\n",  __LINE__, parse_cmd->arg1);
#endif
   }

}