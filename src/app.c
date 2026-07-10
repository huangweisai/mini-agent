#include <stdio.h>
#include <string.h>
#include <signal.h>//我要用到sigal函数来处理题目中说的用户按下ctrl c实现清空输入区的功能
#include "app.h"
#include "tui.h"
#include "input.h"
#include "markdown.h"
//按照ascii表，把ctrl变成普通的字符3
static void sigint_handler(int sig)
{
    (void)sig;
}

// 初始化
void app_init(App *app)
{
    app->running = 1;
    //先对ctrl c信号进行处理
    signal(SIGINT, sigint_handler);
    tui_init();
}

// 清理
void app_cleanup(App *app)
{
    (void)app;
    tui_cleanup();
}

// 处理用户输入的文字
static void handle_input(App *app, const char *input)
{
    (void)app;
    // 先在对话区显示用户输入了什么
    tui_add_chat_line("User:", 0);
    markdown_render_to_chat(input);
    tui_add_chat_line("", 0);  // 空行分隔

    // 不管是指令还是普通文字，都回复429
    // 后面Level 2再加真正的指令处理
    tui_add_chat_line("Assistant:", 0);
    // 用markdown渲染标题
    markdown_render_to_chat("## `429` Too many Requests");
    markdown_render_to_chat("");
    // 用markdown渲染加粗文字
    markdown_render_to_chat("**服务器繁忙，请稍后再试。**");
    tui_add_chat_line("", 0);

    // 刷新对话区和状态栏
    tui_refresh_chat();
    tui_draw_status();
}
// 主循环
void app_run(App *app)
{
    char buf[INPUT_MAX_LEN];
    while (app->running) {
        tui_draw_status();//更新状态栏，主要是更新时间
        // 读一行输入（等用户按Enter或Ctrl+C）
        int result = input_read_line(buf);
        // 用户输入了 /exit
        if (result == -1) {
            app->running = 0;
            break;
        }
        // 用户按了Ctrl+C，输入已清空，继续等下一次输入
        if (result == 0) {
            continue;
        }
        // result == 1，用户正常提交了
        // 如果输入不为空，就处理
        if (strlen(buf) > 0) {
            handle_input(app, buf);
        }
    }
}
