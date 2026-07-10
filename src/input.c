#include <string.h>
#include <stdio.h>
#include "input.h"
#include "tui.h"
// 多行输入缓冲区
static char  lines[INPUT_MAX_LINES][INPUT_MAX_LEN];
static int   line_len[INPUT_MAX_LINES];
static int   num_lines = 1;
static int   cur_line = 0;
static int   cur_col = 0;
// 清空缓冲区
static void buf_clear(void)
{
    int i;
    for (i = 0; i < INPUT_MAX_LINES; i++) {
        lines[i][0] = '\0';
        line_len[i] = 0;
    }
    num_lines = 1;
    cur_line = 0;
    cur_col = 0;
}
// 在光标位置插入一个字符
static void buf_insert_char(char ch)
{
    char *line = lines[cur_line];
    int len = line_len[cur_line];
    if (len >= INPUT_MAX_LEN - 1) return;

    int i;
    for (i = len; i > cur_col; i--) {
        line[i] = line[i - 1];
    }
    line[cur_col] = ch;
    line[len + 1] = '\0';
    line_len[cur_line]++;
    cur_col++;
}
// 在光标位置插入新行
static void buf_insert_newline(void)
{
    if (num_lines >= INPUT_MAX_LINES) return;
    char *line = lines[cur_line];
    char rest[INPUT_MAX_LEN];
    strcpy(rest, line + cur_col);

    line[cur_col] = '\0';
    line_len[cur_line] = cur_col;
    // 后面的行往下挪
    int i;
    for (i = num_lines; i > cur_line + 1; i--) {
        strcpy(lines[i], lines[i - 1]);
        line_len[i] = line_len[i - 1];
    }

    cur_line++;
    strcpy(lines[cur_line], rest);
    line_len[cur_line] = (int)strlen(rest);
    num_lines++;
    cur_col = 0;
}
// 删除光标前的字符
static void buf_backspace(void)
{
    if (cur_col > 0) {
        char *line = lines[cur_line];
        int len = line_len[cur_line];
        int i;
        for (i = cur_col - 1; i < len - 1; i++) {
            line[i] = line[i + 1];
        }
        line[len - 1] = '\0';
        line_len[cur_line]--;
        cur_col--;
    } else if (cur_line > 0) {
        // 行首Backspace，合并到上一行
        char *prev = lines[cur_line - 1];
        int prev_len = line_len[cur_line - 1];
        int curr_len = line_len[cur_line];

        if (prev_len + curr_len < INPUT_MAX_LEN) {
            strcat(prev, lines[cur_line]);
            line_len[cur_line - 1] = prev_len + curr_len;

            int i;
            for (i = cur_line; i < num_lines - 1; i++) {
                strcpy(lines[i], lines[i + 1]);
                line_len[i] = line_len[i + 1];
            }
            lines[num_lines - 1][0] = '\0';
            line_len[num_lines - 1] = 0;
            num_lines--;
            cur_line--;
            cur_col = prev_len;
        }
    }
}
// 合并所有行到buf
static void buf_join(char *buf, int buf_size)
{
    buf[0] = '\0';
    int i;
    for (i = 0; i < num_lines; i++) {
        if (i > 0) {
            strncat(buf, "\n", buf_size - (int)strlen(buf) - 1);
        }
        strncat(buf, lines[i], buf_size - (int)strlen(buf) - 1);
    }
}
// 主函数
int input_read_line(char *buf)
{
    buf_clear();
    // 初始绘制：显示 "> " 提示符
    tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
    while (1) {
        int ch = wgetch(win_input);
        // Enter: 直接提交（13='\r'）
        if (ch == '\r' || ch == KEY_ENTER) {
            buf_join(buf, INPUT_MAX_LEN * INPUT_MAX_LINES);
            if (strcmp(buf, "/exit") == 0) return -1;
            return 1;
        }
        // Ctrl+J: 新开一行
        if (ch == '\n') {
            buf_insert_newline();
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
        // Ctrl+C
        if (ch == 3) {
            buf_clear();
            buf[0] = '\0';
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            return 0;
        }
        //ctrl新开一行
        if (ch == '\t') {
            buf_insert_newline();
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
        // 删除
        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            buf_backspace();
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
        // 左方向键
        if (ch == KEY_LEFT) {
            if (cur_col > 0) {
                cur_col--;
            } else if (cur_line > 0) {
                cur_line--;
                cur_col = line_len[cur_line];
            }
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
        // 右方向键
        if (ch == KEY_RIGHT) {
            if (cur_col < line_len[cur_line]) {
                cur_col++;
            } else if (cur_line < num_lines - 1) {
                cur_line++;
                cur_col = 0;
            }
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }

        // 上
        if (ch == KEY_UP) {
            if (cur_line > 0) {
                cur_line--;
                if (cur_col > line_len[cur_line]) cur_col = line_len[cur_line];
            }
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
        if (ch == KEY_DOWN) {
            if (cur_line < num_lines - 1) {
                cur_line++;
                if (cur_col > line_len[cur_line]) cur_col = line_len[cur_line];
            }
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
        //处理中文
        if (ch >= 192) {
            char utf8[5];
            int utf8_len = 0;
            utf8[utf8_len++] = (char)ch;
            int extra = 0;
            if ((ch & 0xE0) == 0xC0) extra = 1;
            else if ((ch & 0xF0) == 0xE0) extra = 2;
            else if ((ch & 0xF8) == 0xF0) extra = 3;
            int k;
            for (k = 0; k < extra && utf8_len < 4; k++) {
                int next = wgetch(win_input);
                if (next >= 128 && next < 192) {
                    utf8[utf8_len++] = (char)next;
                }
            }
            utf8[utf8_len] = '\0';
            for (k = 0; k < utf8_len; k++) {
                buf_insert_char(utf8[k]);
            }
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
        // 普通ASCII字符
        if (ch >= 32 && ch < 127) {
            buf_insert_char((char)ch);
            tui_draw_input_multiline(lines, line_len, num_lines, cur_line, cur_col);
            continue;
        }
    }
}
