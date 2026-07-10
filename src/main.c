#include <locale.h>
#include "app.h"
int main(void)
{
    App app;
    setlocale(LC_ALL, "");//要获取时间的话，可能需要先获取本地的一些格式
    // 初始化（包括ncurses和窗口）
    app_init(&app);
    // 进入主循环
    app_run(&app);
    // 清理退出
    app_cleanup(&app);
    return 0;
}
