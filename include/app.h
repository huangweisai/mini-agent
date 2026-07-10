#ifndef APP_H
#define APP_H
typedef struct {
    int running; //1结束，0没有结束
} App;
void app_init(App *app);
void app_run(App *app);// 主循环
void app_cleanup(App *app);//退出
#endif
