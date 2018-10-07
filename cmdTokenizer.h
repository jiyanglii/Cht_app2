#pragma once

struct s_cmd{
    char *cmd;
    char *arge0;
    char *arge1;
};

void cmdTokenizer(char *input, struct s_cmd * parse_cmd);