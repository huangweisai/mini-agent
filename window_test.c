//刚刚学了一下ncurses的Window的概念，现在试一试效果
#include <ncurses.h>

int main(void)
{
    initscr();

    // 创建两个窗口
    WINDOW *top = newwin(10, 80, 0, 0);   // 上半部分
    WINDOW *bottom = newwin(3, 80, 11, 0); // 下半部分（中间空一行）

    // 在上窗口打印
    mvwprintw(top, 0, 0, "This is  top ");
    wrefresh(top);

    // 在下窗口打印
    mvwprintw(bottom, 0, 0, "> type here");
    wrefresh(bottom);

    getch();
    endwin();
    return 0;
}