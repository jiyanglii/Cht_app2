#include <ctype.h>
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

//#define DEBUG

void cmdTokenizer(char *input, struct s_cmd * parse_cmd){

    // parse input cmd, initialize new buffer for cmd and each arges, and pass the pointers to parse_cmd as returned parsed cmd
    char *token;
    int input_size = strlen(input);
    int char_count = 0;
    const char delim[2] = " ";

    parse_cmd->arg_num = 0;

    /* get the first token */
    token = strtok(input, delim);
    parse_cmd->cmd = token;

#ifdef DEBUG
    printf("Line %d: %s\n",  __LINE__, parse_cmd->cmd);
#endif

    /* walk through other tokens */
    if(token) {
        char_count += strlen(parse_cmd->cmd) + 1;

        token = strtok(NULL, delim);
        parse_cmd->arg0 = token;

        if(parse_cmd->arg0){
            char_count += strlen(parse_cmd->arg0) + 1;
            parse_cmd->arg_num ++;
        }

#ifdef DEBUG
        printf("Line %d: %s\n",  __LINE__, parse_cmd->arg0);
#endif
   }

    if(char_count < input_size){
        parse_cmd->arg1 = (char *)(input + char_count);
        parse_cmd->arg_num ++;
    }
    else
        parse_cmd->arg1 = NULL;

#ifdef DEBUG
    printf("Line %d: %s\n",  __LINE__, parse_cmd->arg1);
#endif

#ifdef DEBUG
    printf("Line %d: Number of args in this cmd: %d\n",  __LINE__, parse_cmd->arg_num);
#endif

    if(parse_cmd->arg_num == 0)
        parse_cmd->cmd = trimwhitespace(parse_cmd->cmd);
}

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

char *concatCMD(char * msg, struct s_cmd * parse_cmd){

    strcpy(msg, parse_cmd->cmd);

    if(parse_cmd->arg_num >= 1){
        strcat(msg, " ");
        strcat(msg, parse_cmd->arg0);
    }

    if(parse_cmd->arg_num == 2){
        strcat(msg, " ");
        strcat(msg, parse_cmd->arg1);
    }

    return msg;

}