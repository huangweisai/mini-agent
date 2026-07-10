#include<ncurses.h>
int main(){
    initscr();
    printw("hello,ncurses!");
    printw("不换行试试");
    refresh();
    printw("\n按其他按键退出...");
    getch();//防止窗口一闪而过
    endwin();
}