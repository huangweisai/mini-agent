#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tui.h"
#include "util.h"
// 三个窗口的全局变量
WINDOW *win_conv = NULL;//对话区
WINDOW *win_status = NULL;//状态区
WINDOW *win_input = NULL;//输入区

// 用来保存对话区显示过的每一行
#define MAX_CHAT_LINES 512
static char  chat_text[MAX_CHAT_LINES][1024];
static int   chat_color[MAX_CHAT_LINES];
static int   chat_count = 0;// 总共有多少行
static int   chat_top = 0;// 当前显示的第一行是第几行
// 重新计算chat_top，让最后一行刚好在状态栏上面
static void recalc_chat_top(void)
{
    int height = getmaxy(win_conv);
    chat_top = chat_count - height;
    if (chat_top < 0) {
        chat_top = 0;
    }
}
//初始化ncurses
void tui_init(void)
{
    // 进入ncurses模式
    initscr();
    // raw模式：所有按键都交给程序处理，终端不做任何拦截
    // 这样Ctrl+C不会产生信号，而是作为字符3传给程序
    raw();
    //不显示方向键
    noecho();
    // 显示光标
    curs_set(1);
    // 启用方向按键
    keypad(stdscr, TRUE);

    // 初始化颜色
    if (has_colors()) {
        start_color();
        //分别是，编号，前景和背景
        init_pair(CP_STATUS, COLOR_BLACK,  COLOR_WHITE);
        init_pair(CP_GREEN,  COLOR_GREEN,  COLOR_BLACK);
        init_pair(CP_RED,    COLOR_RED,    COLOR_BLACK);
        init_pair(CP_BLUE,   COLOR_BLUE,   COLOR_BLACK);
    }
    // 创建窗口
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    // 计算每个窗口的高度
    int h_conv = rows - 4;  // 对话区占剩下的所有行
    // 状态栏1行，输入区3行，加起来4行

    // 创建三个窗口
    // newwin(高度, 宽度, 起始行, 起始列)
    win_conv   = newwin(h_conv,cols, 0,0);
    win_status = newwin(1,cols,h_conv,0);
    win_input  = newwin(3,cols,h_conv + 1,0);
    //画一次
    tui_draw_status();
    tui_draw_input("", 0);
}

//清理退出
void tui_cleanup(void)
{
    // 销毁窗口
    if (win_conv != NULL) {
        delwin(win_conv);
        win_conv = NULL;
    }
    if (win_status != NULL) {
        delwin(win_status);
        win_status = NULL;
    }
    if (win_input != NULL) {
        delwin(win_input);
        win_input = NULL;
    }
    endwin();
}

//画状态栏
void tui_draw_status(void)
{
    int cols = getmaxx(win_status);
    // 设置状态栏的背景颜色
    wbkgdset(win_status, COLOR_PAIR(CP_STATUS));
    werase(win_status);
    //左边显示当前目录
    char cwd[1024];
    char short_path[1024];
    // 获取当前工作目录
    getcwd(cwd, sizeof(cwd));
    //如果是家目录的话，要换成~
    util_short_path(short_path, sizeof(short_path), cwd);
    // 用绿色打印目录
    wattron(win_status, COLOR_PAIR(CP_GREEN));
    mvwprintw(win_status, 0, 1, "%s", short_path);
    wattroff(win_status, COLOR_PAIR(CP_GREEN));
    //右边显示时间
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[16];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", t);
    // 时间放在最右边，留1个字符的边距
    int time_x = cols - (int)strlen(timebuf) - 1;
    if (time_x < 0) {
        time_x = 0;
    }
    mvwprintw(win_status, 0, time_x, "%s", timebuf);
    // 刷新状态栏
    wrefresh(win_status);
}

//画输入区
void tui_draw_input(const char *buf, int cursor)
{
    // 放空
    werase(win_input);
    // 打印 "> "
    mvwprintw(win_input, 0, 0, "> ");

    // 打印用户输入的内容
    wprintw(win_input, "%s", buf);
  
    //把光标放到正确的位置，"> "是2个字符，加上cursor的偏移
    wmove(win_input, 0, 2 + cursor);
    // 刷新输入区
    wrefresh(win_input);
}

// ====== 往对话区加一行 ======
void tui_add_chat_line(const char *line, int color)
{
    // 如果满了，就把最早的丢掉，整体往前移动
    if (chat_count >= MAX_CHAT_LINES) {
        int i;
        for (i = 1; i < MAX_CHAT_LINES; i++) {
            strcpy(chat_text[i - 1], chat_text[i]);
            chat_color[i - 1] = chat_color[i];
        }
        chat_count = MAX_CHAT_LINES - 1;
    }

    strncpy(chat_text[chat_count], line, 1023);
    chat_text[chat_count][1023] = '\0';  // 确保有结尾
    chat_color[chat_count] = color;
    chat_count++;
}

// ====== 刷新对话区显示 ======
void tui_refresh_chat(void)
{
    // 先清空对话区窗口
    werase(win_conv);
    // 计算应该显示哪些行
    recalc_chat_top();
    // 获取窗口高度
    int rows = getmaxy(win_conv);
    // 逐行显示
    int i;
    for (i = 0; i < rows; i++) {
        int idx = chat_top + i;
        if (idx >= chat_count) {
            break;
        }
        int color = chat_color[idx];

        if (color != 0) {
            wattron(win_conv, COLOR_PAIR(color));
        }
        mvwaddstr(win_conv, i, 0, chat_text[idx]);
        if (color != 0) {
            wattroff(win_conv, COLOR_PAIR(color));
        }
    }

    // 刷新对话区
    wrefresh(win_conv);
}

//清空对话区
void tui_clear_chat(void)
{
    chat_count = 0;
    chat_top = 0;
    werase(win_conv);
    wrefresh(win_conv);
}
