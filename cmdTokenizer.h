#pragma once

#define MSG_SIZE    256

struct s_cmd{
    char *cmd;
    char *arg0;
    char *arg1;
    char *end;

    int arg_num;
};

void cmdTokenizer(char *input, struct s_cmd * parse_cmd);
char *trimwhitespace(char *str);
char *concatCMD(char * msg, struct s_cmd * parse_cmd);