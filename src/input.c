#include <string.h>
#include "input.h"
#include "tui.h"

// 从输入区读取一行用户输入
// buf: 存放输入内容的数组
// 返回: 1=用户按了Enter提交, 0=用户按了Ctrl+C, -1=用户输入了/exit
int input_read_line(char *buf)
{
    int cursor = 0;  // 光标位置
    int len = 0;     // 输入了多少个字符
    int i;           // 临时变量，循环用

    // 先清空buf
    buf[0] = '\0';

    // 画一下输入区
    tui_draw_input(buf, cursor);

    // 不断读取按键
    while (1) {
        int ch = wgetch(win_input);

        // ====== 按了Enter ======
        if (ch == '\n' || ch == KEY_ENTER) {
            buf[len] = '\0';

            // 检查是不是 /exit
            if (strcmp(buf, "/exit") == 0) {
                return -1;
            }

            // 正常提交
            return 1;
        }

        // ====== 按了Ctrl+C ======
        // Ctrl+C的ASCII码是3
        if (ch == 3) {
            // 清空输入
            len = 0;
            cursor = 0;
            buf[0] = '\0';
            tui_draw_input(buf, cursor);
            return 0;
        }

        // ====== 按了Backspace ======
        // KEY_BACKSPACE是ncurses定义的，127是DEL键
        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if (cursor > 0) {
                // 把cursor后面的字符往前挪一位
                for (i = cursor - 1; i < len - 1; i++) {
                    buf[i] = buf[i + 1];
                }
                cursor--;
                len--;
                buf[len] = '\0';
            }
            tui_draw_input(buf, cursor);
            continue;
        }

        // ====== 按了左方向键 ======
        if (ch == KEY_LEFT) {
            if (cursor > 0) {
                cursor--;
            }
            tui_draw_input(buf, cursor);
            continue;
        }

        // ====== 按了右方向键 ======
        if (ch == KEY_RIGHT) {
            if (cursor < len) {
                cursor++;
            }
            tui_draw_input(buf, cursor);
            continue;
        }

        // ====== 普通可打印字符 ======
        // ASCII 32是空格，126是~，32~126是可打印字符
        if (ch >= 32 && ch < 127) {
            // 还有空间才插入
            if (len < INPUT_MAX_LEN - 1) {
                // 先把cursor位置及后面的字符往后挪一位
                for (i = len; i > cursor; i--) {
                    buf[i] = buf[i - 1];
                }
                // 在cursor位置放新字符
                buf[cursor] = (char)ch;
                cursor++;
                len++;
                buf[len] = '\0';
            }
            tui_draw_input(buf, cursor);
            continue;
        }

        // 其他键忽略，不做处理
    }
}
