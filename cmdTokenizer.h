#pragma once

struct s_cmd{
    char *cmd;
    char *arg0;
    char *arg1;

    int arg_num;
};

void cmdTokenizer(char *input, struct s_cmd * parse_cmd);