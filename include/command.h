#ifndef COMMAND_H
#define COMMAND_H
// 命令类型
enum CmdType {
    CMD_NONE,
    CMD_READ,
    CMD_WRITE,
    CMD_EXEC,
    CMD_EXEC_INPUT,
    CMD_EXIT,
    CMD_UNKNOWN
};
// 解析后的命令结构
typedef struct {
    enum CmdType type;
    char file[1024];
    char content[2048];
    int  lines;
    int  start_line;
    int  end_line;
} Command;
// 解析用户输入，返回命令类型
enum CmdType command_parse(const char *input, Command *cmd);
#endif
