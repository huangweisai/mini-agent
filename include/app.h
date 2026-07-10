#ifndef APP_H
#define APP_H

// 应用的状态
typedef struct {
    int running;  // 1=还在跑，0=该退出了
} App;
// 初始化
void app_init(App *app);
void app_run(App *app);// 主循环
// 清理退出
void app_cleanup(App *app);
#endif
