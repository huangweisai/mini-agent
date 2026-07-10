#ifndef TOOLS_H
#define TOOLS_H

// 读取文件
// file: 文件路径
// lines: 要读取的行数，0表示读取整个文件
// start_line, end_line: 行范围，比如 "40-60"
// 返回: 0=成功, -1=失败
int tool_read_file(const char *file, int lines, int start_line, int end_line,
                   char *errmsg, int errmsg_size);

// 写入文件（追加模式）
int tool_write_file(const char *file, const char *content,
                    char *errmsg, int errmsg_size);

// 执行命令，返回退出码，-1=超时
int tool_exec_cmd(const char *cmd);

// 带输入的执行（进阶），临时切出ncurses让用户交互
int tool_exec_input_cmd(const char *cmd);

#endif
