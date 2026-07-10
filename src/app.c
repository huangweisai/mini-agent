#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "app.h"
#include "tui.h"
#include "input.h"
#include "markdown.h"

// 处理SIGINT信号（Ctrl+C产生的信号）
// 这里什么都不做，让它变成一个普通的字符3
// 这样input_read_line里面就能捕获Ctrl+C了
static void sigint_handler(int sig)
{
    (void)sig;  // 避免unused参数的警告
}

// 初始化应用
void app_init(App *app)
{
    app->running = 1;

    // 注册信号处理，让Ctrl+C不要直接杀掉程序
    signal(SIGINT, sigint_handler);
    tui_init();
}
void app_cleanup(App *app)
{
    (void)app;
    tui_cleanup();
}


//处理输入的函数，是普通文字就回复429
static void handle_input(App *app, const char *input)
{
    (void)app;

    // 先在对话区显示用户输入了什么
    tui_add_chat_line("User:", 0);
    tui_add_chat_line(input, 0);
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
        // 每次循环都刷新一下状态栏（更新时间）
        tui_draw_status();

        // 读一行输入（会阻塞，等用户按Enter或Ctrl+C）
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
