#include <stdio.h>
#include <string.h>
#include "command.h"

enum CmdType command_parse(const char *input, Command *cmd)
{
    char tmp[2048];
    char *token;
    char *saveptr;
    memset(cmd, 0, sizeof(Command));
    if (input == NULL || input[0] == '\0') {
        return CMD_NONE;
    // 不是命令
    if (input[0] != '/') {
        return CMD_NONE;
    }
    strncpy(tmp, input, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    token = strtok_r(tmp, " ", &saveptr);
    if (token == NULL) {
        return CMD_NONE;
    }
    // /exit
    if (strcmp(token, "/exit") == 0) {
        cmd->type = CMD_EXIT;
        return CMD_EXIT;
    }
    // /read
    if (strcmp(token, "/read") == 0) {
        cmd->type = CMD_READ;
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            cmd->type = CMD_UNKNOWN;
            return CMD_UNKNOWN;
        }
        strncpy(cmd->file, token, sizeof(cmd->file) - 1);
        token = strtok_r(NULL, " ", &saveptr);
        if (token != NULL) {
            char *dash = strchr(token, '-');
            if (dash != NULL) {
                *dash = '\0';
                cmd->start_line = atoi(token);
                cmd->end_line = atoi(dash + 1);
            } else {
                cmd->lines = atoi(token);
            }
        }
        return CMD_READ;
    }
    // /write
    if (strcmp(token, "/write") == 0) {
        cmd->type = CMD_WRITE;
        token = strtok_r(NULL, " ", &saveptr);
        if (token == NULL) {
            cmd->type = CMD_UNKNOWN;
            return CMD_UNKNOWN;
        }
        strncpy(cmd->file, token, sizeof(cmd->file) - 1);

        if (saveptr != NULL) {
            strncpy(cmd->content, saveptr, sizeof(cmd->content) - 1);
            cmd->content[sizeof(cmd->content) - 1] = '\0';
        }
        return CMD_WRITE;
    }
    // /exec
    if (strcmp(token, "/exec") == 0) {
        cmd->type = CMD_EXEC;

        if (saveptr != NULL) {
            strncpy(cmd->content, saveptr, sizeof(cmd->content) - 1);
            cmd->content[sizeof(cmd->content) - 1] = '\0';
        }
        if (strlen(cmd->content) == 0) {
            cmd->type = CMD_UNKNOWN;
            return CMD_UNKNOWN;
        }
        return CMD_EXEC;
    }
    // /exec_input
    if (strcmp(token, "/exec_input") == 0) {
        cmd->type = CMD_EXEC_INPUT;
        if (saveptr != NULL) {
            strncpy(cmd->content, saveptr, sizeof(cmd->content) - 1);
            cmd->content[sizeof(cmd->content) - 1] = '\0';
        }
        if (strlen(cmd->content) == 0) {
            cmd->type = CMD_UNKNOWN;
            return CMD_UNKNOWN;
        }
        return CMD_EXEC_INPUT;
    }
    cmd->type = CMD_UNKNOWN;
    return CMD_UNKNOWN;
}
