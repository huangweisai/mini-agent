#ifndef TUI_H
#define TUI_H
#include <ncurses.h>
// 颜色对编号
#define CP_STATUS 1   // 状态栏背景
#define CP_GREEN  2   // 绿色文字，用于目录和标题
#define CP_RED    3   // 红色文字，用于加粗
#define CP_BLUE   4   // 蓝色文字，用于代码
// 初始化ncurses，创建窗口
void tui_init(void);
// 退出ncurses，清理窗口
void tui_cleanup(void);
// 画状态栏（目录 + 时间）
void tui_draw_status(void);
// 画输入区，显示当前输入的内容和光标位置（单行版本，保留兼容）
void tui_draw_input(const char *buf, int cursor);
// 画输入区，支持多行显示和自动换行
// lines: 每行内容, line_len: 每行长度, num_lines: 行数
// cur_line, cur_col: 光标位置
void tui_draw_input_multiline(char lines[][1024], int *line_len,int num_lines, int cur_line, int cur_col);
// 往对话区加一行文字，color是颜色对编号，0表示默认色
void tui_add_chat_line(const char *line, int color);
// 刷新对话区的显示
void tui_refresh_chat(void);
// 清空对话区
void tui_clear_chat(void);
// 三个窗口，其他模块也要用
extern WINDOW *win_conv;
extern WINDOW *win_status;
extern WINDOW *win_input;
#endif
