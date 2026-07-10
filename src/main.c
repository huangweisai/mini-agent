// mini-agent 主程序入口
// 这是一个没有大模型的古法Agent终端
// 能读文件、写文件、执行命令、记录历史
#include <locale.h>
#include "app.h"
int main(void)
{
    App app;
    // 必须在initscr之前调用
    setlocale(LC_ALL, "");
    // 初始化（包括ncurses和窗口）
    app_init(&app);
    // 进入主循环
    app_run(&app);
    // 清理退出
    app_cleanup(&app);
    return 0;
}
