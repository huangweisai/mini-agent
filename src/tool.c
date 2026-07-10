#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include "tools.h"
#include "tui.h"
#define MAX_READ_LINES 500
#define DEFAULT_READ_LIMIT 100
#define MAX_OUTPUT_SIZE 16384
#define EXEC_TIMEOUT 10

//reaad
int tool_read_file(const char *file, int lines, int start_line, int end_line,
                   char *errmsg, int errmsg_size)
{
    FILE *fp;
    char line_buf[2048];
    int line_count = 0;
    int displayed = 0;

    // 检查文件是否存在
    if (access(file, F_OK) != 0) {
        snprintf(errmsg, errmsg_size, "not found");
        return -1;
    }

    // 检查是否可读
    if (access(file, R_OK) != 0) {
        snprintf(errmsg, errmsg_size, "permission denied");
        return -1;
    }

    // 进阶：检查是否是文本文件
    // 方法1：用 file 命令判断MIME类型
    // 方法2：读文件前8KB，如果有null字节就是二进制文件
    {
        int is_text = 1;  // 默认认为是文本
        // 尝试用 file 命令
        char cmd[2048];
        char mime[256];
        snprintf(cmd, sizeof(cmd), "file -b --mime-type '%s'", file);
        FILE *pipe = popen(cmd, "r");
        if (pipe != NULL) {
            if (fgets(mime, sizeof(mime), pipe) != NULL) {
                int mlen = (int)strlen(mime);
                if (mlen > 0 && mime[mlen - 1] == '\n') {
                    mime[mlen - 1] = '\0';
                }
                if (strncmp(mime, "text/", 5) != 0) {
                    is_text = 0;
                    snprintf(errmsg, errmsg_size, "not a text file (type: %s)", mime);
                }
            }
            int ret = pclose(pipe);
            // 如果file命令本身执行失败，用备选方案
            if (ret != 0) {
                is_text = 1;  // 先重置，用备选方案判断
            }
        } else {
            // file命令不可用，用备选方案
            is_text = 1;
        }

        // 备选方案：检查文件前8KB有没有null字节
        if (is_text == 1) {
            FILE *check = fopen(file, "rb");
            if (check != NULL) {
                unsigned char buf[8192];
                size_t nread = fread(buf, 1, sizeof(buf), check);
                fclose(check);
                size_t i;
                for (i = 0; i < nread; i++) {
                    if (buf[i] == 0) {
                        is_text = 0;
                        snprintf(errmsg, errmsg_size, "not a text file (binary content detected)");
                        break;
                    }
                }
            }
        }

        if (is_text == 0) {
            return -1;
        }
    }

    fp = fopen(file, "r");
    if (fp == NULL) {
        snprintf(errmsg, errmsg_size, "open failed");
        return -1;
    }

    // 先统计行数
    int total_lines = 0;
    while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
        total_lines++;
    }
    rewind(fp);

    // 没指定行数且超过100行，拒绝
    if (lines == 0 && start_line == 0 && total_lines > DEFAULT_READ_LIMIT) {
        snprintf(errmsg, errmsg_size, "too many lines (%d), please specify line count",
                 total_lines);
        fclose(fp);
        return -1;
    }

    // 超过500行拒绝
    if (lines > MAX_READ_LINES) {
        snprintf(errmsg, errmsg_size, "too many lines (max %d)", MAX_READ_LINES);
        fclose(fp);
        return -1;
    }

    // 确定显示范围
    int show_start = 1;
    int show_end = total_lines;

    if (start_line > 0 && end_line > 0) {
        show_start = start_line;
        show_end = end_line;
    } else if (lines > 0) {
        show_start = 1;
        show_end = lines;
    }

    // 逐行读取
    while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
        line_count++;

        // 去掉换行符
        int len = (int)strlen(line_buf);
        if (len > 0 && line_buf[len - 1] == '\n') {
            line_buf[len - 1] = '\0';
        }

        if (line_count >= show_start && line_count <= show_end) {
            tui_add_chat_line(line_buf, 0);
            displayed++;
        }

        if (line_count > show_end) {
            break;
        }
    }

    fclose(fp);

    if (displayed == 0) {
        snprintf(errmsg, errmsg_size, "no lines in range %d-%d (file has %d lines)",
                 show_start, show_end, total_lines);
        return -1;
    }

    snprintf(errmsg, errmsg_size, "Line %d-%d", show_start,
             show_start + displayed - 1);
    return 0;
}

// ====== 写入文件 ======
int tool_write_file(const char *file, const char *content,
                    char *errmsg, int errmsg_size)
{
    FILE *fp;

    fp = fopen(file, "a");
    if (fp == NULL) {
        if (errno == EACCES) {
            snprintf(errmsg, errmsg_size, "permission denied");
        } else {
            snprintf(errmsg, errmsg_size, "open failed: %s", strerror(errno));
        }
        return -1;
    }

    int bytes = fprintf(fp, "%s\n", content);
    fclose(fp);

    if (bytes < 0) {
        snprintf(errmsg, errmsg_size, "write failed");
        return -1;
    }

    snprintf(errmsg, errmsg_size, "%d", bytes);
    return 0;
}

// ====== 执行命令 ======
int tool_exec_cmd(const char *cmd)
{
    int pipefd[2];
    pid_t pid;
    char output[MAX_OUTPUT_SIZE];
    int total_read = 0;

    if (pipe(pipefd) == -1) {
        tui_add_chat_line("Error: pipe failed", 0);
        return -1;
    }

    // 把管道读端设为非阻塞，这样read不会卡住
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    pid = fork();
    if (pid == -1) {
        tui_add_chat_line("Error: fork failed", 0);
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        // 子进程
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execlp("sh", "sh", "-c", cmd, NULL);
        _exit(127);
    }

    // 父进程
    close(pipefd[1]);

    int status;
    int timed_out = 0;
    time_t start_time = time(NULL);

    while (1) {
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result > 0) {
            // 子进程结束，读剩余输出
            while (1) {
                int n = read(pipefd[0], output + total_read,
                             MAX_OUTPUT_SIZE - total_read - 1);
                if (n <= 0) break;
                total_read += n;
            }
            break;
        }

        // 检查超时
        if (time(NULL) - start_time >= EXEC_TIMEOUT) {
            timed_out = 1;
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            break;
        }

        // 读取一些输出（非阻塞，没数据会返回-1）
        int n = read(pipefd[0], output + total_read,
                     MAX_OUTPUT_SIZE - total_read - 1);
        if (n > 0) {
            total_read += n;
        }
        // 没有数据时短暂等待，避免忙等浪费CPU
        if (n <= 0) {
            usleep(10000);  // 10ms
        }
    }

    close(pipefd[0]);
    output[total_read] = '\0';

    if (timed_out) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Command timeout after %ds", EXEC_TIMEOUT);
        tui_add_chat_line(msg, 0);
        return -1;
    }

    int exit_code = 0;
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    }

    char msg[256];
    snprintf(msg, sizeof(msg), "Command exited with code %d", exit_code);
    tui_add_chat_line(msg, 0);
    tui_add_chat_line("", 0);

    // 按行显示输出
    if (total_read > 0) {
        char *line = output;
        int line_count = 0;
        while (*line && line_count < 200) {
            char *newline = strchr(line, '\n');
            if (newline) {
                *newline = '\0';
            }
            tui_add_chat_line(line, 0);
            line_count++;
            if (newline) {
                line = newline + 1;
            } else {
                break;
            }
        }
    }

    return exit_code;
}

// ====== 带输入的执行（进阶） ======
// 临时切出ncurses，让用户在普通终端里与程序交互
// 用 script 命令记录终端会话，结束后读取记录显示到对话区
int tool_exec_input_cmd(const char *cmd)
{
    char script_cmd[2100];
    char log_path[] = "/tmp/mini_agent_exec.log";
    char msg[256];
    int ret;

    // 构造 script 命令
    // -q: 安静模式，不显示开始/结束信息
    // -e: 让 script 返回被执行命令的退出码
    // -c: 指定要执行的命令
    snprintf(script_cmd, sizeof(script_cmd),
             "script -q -e -c '%s' %s", cmd, log_path);

    // 暂停ncurses，恢复终端到正常模式
    def_prog_mode();   // 保存当前ncurses状态
    endwin();          // 退出ncurses

    // 在正常终端里执行命令
    printf("\n");
    ret = system(script_cmd);
    printf("\n[按回车返回 mini-agent]\n");
    getchar();  // 等用户按回车再回去

    // 恢复ncurses
    reset_prog_mode();  // 恢复之前保存的ncurses状态
    refresh();          // 刷新屏幕

    // 读取script记录的日志，显示到对话区
    FILE *logfp = fopen(log_path, "r");
    if (logfp != NULL) {
        char line_buf[2048];
        int line_count = 0;
        while (fgets(line_buf, sizeof(line_buf), logfp) != NULL && line_count < 200) {
            // 去掉末尾换行
            int len = (int)strlen(line_buf);
            if (len > 0 && line_buf[len - 1] == '\n') {
                line_buf[len - 1] = '\0';
            }
            tui_add_chat_line(line_buf, 0);
            line_count++;
        }
        fclose(logfp);
        remove(log_path);  // 删除临时日志文件
    }

    // 显示退出码
    int exit_code = 0;
    if (WIFEXITED(ret)) {
        exit_code = WEXITSTATUS(ret);
    }
    snprintf(msg, sizeof(msg), "Command exited with code %d", exit_code);
    tui_add_chat_line(msg, 0);

    return exit_code;
}
