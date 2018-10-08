#pragma once

struct s_cmd{
    char *cmd;
    char *arg0;
    char *arg1;
};

void cmdTokenizer(char *input, struct s_cmd * parse_cmd);