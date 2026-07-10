#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

char *util_short_path(char *dest, size_t size, const char *path)//这个函数用来判断状态栏显示的工作路径，如果是在家目录的话替换成~符号
{
    const char *home = getenv("HOME");
    if (home && strncmp(path, home, strlen(home)) == 0) {
        
        snprintf(dest, size, "~%s", path + strlen(home));
    } else {
        snprintf(dest, size, "%s", path);
    }
    return dest;
}
