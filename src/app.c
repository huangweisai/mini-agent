#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "app.h"
#include "tui.h"
#include "input.h"
#include "markdown.h"
#include "command.h"
#include "tools.h"

//按照ascii表，把ctrl变成普通的字符3
static void sigint_handler(int sig)
{
    (void)sig;
}

// 初始化
void app_init(App *app)
{
    app->running = 1;
    signal(SIGINT, sigint_handler);
    tui_init();
}

// 清理
void app_cleanup(App *app)
{
    (void)app;
    tui_cleanup();
}

// 处理 /read
static void handle_read(const Command *cmd)
{
    char errmsg[512];
    char msg[1200];

    // 显示工具调用摘要
    if (cmd->start_line > 0 && cmd->end_line > 0) {
        snprintf(msg, sizeof(msg), "Tool_Use [read_file] file=%s lines=%d-%d",
                 cmd->file, cmd->start_line, cmd->end_line);
    } else if (cmd->lines > 0) {
        snprintf(msg, sizeof(msg), "Tool_Use [read_file] file=%s lines=%d",
                 cmd->file, cmd->lines);
    } else {
        snprintf(msg, sizeof(msg), "Tool_Use [read_file] file=%s", cmd->file);
    }
    tui_add_chat_line(msg, 0);
    tui_add_chat_line("", 0);

    int result = tool_read_file(cmd->file, cmd->lines,
                                cmd->start_line, cmd->end_line,
                                errmsg, sizeof(errmsg));

    if (result == 0) {
        snprintf(msg, sizeof(msg), "File %s Read success, %s", cmd->file, errmsg);
        tui_add_chat_line(msg, 0);
        tui_add_chat_line("", 0);
    } else {
        snprintf(msg, sizeof(msg), "File %s Read failed, %s", cmd->file, errmsg);
        tui_add_chat_line(msg, 0);
    }
}

// 处理 /write
static void handle_write(const Command *cmd)
{
    char errmsg[512];
    char msg[1200];

    snprintf(msg, sizeof(msg), "Tool_Use [write_file] file=%s", cmd->file);
    tui_add_chat_line(msg, 0);
    tui_add_chat_line("", 0);

    int result = tool_write_file(cmd->file, cmd->content,
                                 errmsg, sizeof(errmsg));

    if (result == 0) {
        snprintf(msg, sizeof(msg), "File %s Write success", cmd->file);
        tui_add_chat_line(msg, 0);
    } else {
        snprintf(msg, sizeof(msg), "File %s Write failed, %s", cmd->file, errmsg);
        tui_add_chat_line(msg, 0);
    }
}

// 处理 /exec
static void handle_exec(const Command *cmd)
{
    char msg[1200];

    snprintf(msg, sizeof(msg), "Tool_Use [exec_cmd] cmd=%s", cmd->content);
    tui_add_chat_line(msg, 0);
    tui_add_chat_line("", 0);

    tool_exec_cmd(cmd->content);
}

// 处理 /exec_input
static void handle_exec_input(const Command *cmd)
{
    char msg[1200];

    snprintf(msg, sizeof(msg), "Tool_Use [exec_cmd] cmd=%s", cmd->content);
    tui_add_chat_line(msg, 0);
    tui_add_chat_line("", 0);

    tool_exec_input_cmd(cmd->content);
}

// 处理用户输入
static void handle_input(App *app, const char *input)
{
    (void)app;

    Command cmd;
    char msg[1200];

    tui_add_chat_line("User:", 0);
    markdown_render_to_chat(input);
    tui_add_chat_line("", 0);

    enum CmdType type = command_parse(input, &cmd);

    switch (type) {
    case CMD_EXIT:
        break;

    case CMD_READ:
        handle_read(&cmd);
        break;

    case CMD_WRITE:
        handle_write(&cmd);
        break;

    case CMD_EXEC:
        handle_exec(&cmd);
        break;

    case CMD_EXEC_INPUT:
        handle_exec_input(&cmd);
        break;

    case CMD_UNKNOWN:
        snprintf(msg, sizeof(msg), "Unknown command: %s", input);
        tui_add_chat_line(msg, 0);
        break;

    case CMD_NONE:
    default:
        tui_add_chat_line("Assistant:", 0);
        markdown_render_to_chat("## `429` Too many Requests");
        markdown_render_to_chat("");
        markdown_render_to_chat("**服务器繁忙，请稍后再试。**");
        tui_add_chat_line("", 0);
        break;
    }

    tui_refresh_chat();
    tui_draw_status();
}

// 主循环
void app_run(App *app)
{
    char buf[INPUT_MAX_LEN];

    while (app->running) {
        tui_draw_status();
        int result = input_read_line(buf);

        if (result == -1) {
            app->running = 0;
            break;
        }

        if (result == 0) {
            continue;
        }

        if (strlen(buf) > 0) {
            handle_input(app, buf);
        }
    }
}
