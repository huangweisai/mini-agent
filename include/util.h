#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

// 把长路径变短，比如 /home/user/proj 变成 ~/proj
// dest是输出的地方，size是dest有多大，path是原始路径
char *util_short_path(char *dest, size_t size, const char *path);

#endif
