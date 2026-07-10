#ifndef INPUT_H
#define INPUT_H
// 输入区最大能放多少字符
#define INPUT_MAX_LEN 1024
// 从输入区读一行
// buf是存放结果的数组
// 返回值：1=正常提交，0=按了Ctrl+C，-1=输入了/exit
int input_read_line(char *buf);
#endif
