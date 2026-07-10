#include <string.h>
#include "markdown.h"
#include "tui.h"
// 检查s是否以prefix开头
static int starts_with(const char *s, const char *prefix)
{
    if (strncmp(s, prefix, strlen(prefix)) == 0) {
        return 1;
    }
    return 0;
}
// 把一行markdown渲染到对话区，先看是不是标题，不是的话就一个字符一个字符扫描
void markdown_render_to_chat(const char *line)
{
    char segment[1024];  // 临时存放一个片段
    int seg_len;         // 片段的长度
    int len;             // 输入字符串的长度
    int i, j;            // 循环变量
    int code_len;        // 代码片段的长度
    int bold_len;        // 加粗片段的长度
    // 空指针不处理
    if (line == NULL) {
        return;
    }
    len = (int)strlen(line);
    //先检查是不是标题
    //二级标题 "## xxx"
    if (starts_with(line, "## ")) {
        // 整行绿色显示
        tui_add_chat_line(line, CP_GREEN);
        return;
    }
    // 一级标题 "# xxx"
    if (starts_with(line, "# ")) {
        // 整行绿色显示
        tui_add_chat_line(line, CP_GREEN);
        return;
    }
    seg_len = 0;
    i = 0;
    while (i < len) {
        // 检查`code`格式
        if (line[i] == '`') {
            // 先把之前积累的普通文字输出
            if (seg_len > 0) {
                segment[seg_len] = '\0';
                tui_add_chat_line(segment, 0);  // 0=默认色
                seg_len = 0;
            }
            // 找匹配的闭合反引号
            j = i + 1;
            while (j < len && line[j] != '`') {
                j++;
            }
            if (j < len) {
                // 找到了闭合反引号
                // 提取中间的代码内容（不包括反引号本身）
                code_len = j - i - 1;
                if (code_len > 0) {
                    memcpy(segment, line + i + 1, code_len);
                    segment[code_len] = '\0';
                    tui_add_chat_line(segment, CP_BLUE);  // 蓝色
                }
                i = j + 1;  // 跳过闭合反引号
            } else {
                // 没找到闭合的，当普通字符处理
                segment[seg_len] = line[i];
                seg_len++;
                i++;
            }
            continue;
        }
        //**text**格式
        if (line[i] == '*' && (i + 1) < len && line[i + 1] == '*') {
            // 先把之前积累的普通文字输出
            if (seg_len > 0) {
                segment[seg_len] = '\0';
                tui_add_chat_line(segment, 0);
                seg_len = 0;
            }
            // 从"**"后面开始找匹配的"**"
            j = i + 2;
            while ((j + 1) < len) {
                if (line[j] == '*' && line[j + 1] == '*') {
                    break;
                }
                j++;
            }
            if ((j + 1) < len) {
                // 找到了闭合的 **
                bold_len = j - i - 2;
                if (bold_len > 0) {
                    memcpy(segment, line + i + 2, bold_len);
                    segment[bold_len] = '\0';
                    tui_add_chat_line(segment, CP_RED);  // 红色
                }
                i = j + 2;  // 跳过闭合的 **
            } else {
                // 没找到闭合的，当普通字符
                segment[seg_len] = line[i];
                seg_len++;
                i++;
            }

            continue;
        }
        //__text__格式，其实和上面的是完全一样的，应该用||符号的
        if (line[i] == '_' && (i + 1) < len && line[i + 1] == '_') {
            // 和 ** 一样的逻辑
            if (seg_len > 0) {
                segment[seg_len] = '\0';
                tui_add_chat_line(segment, 0);
                seg_len = 0;
            }
            j = i + 2;
            while ((j + 1) < len) {
                if (line[j] == '_' && line[j + 1] == '_') {
                    break;
                }
                j++;
            }
            if ((j + 1) < len) {
                bold_len = j - i - 2;
                if (bold_len > 0) {
                    memcpy(segment, line + i + 2, bold_len);
                    segment[bold_len] = '\0';
                    tui_add_chat_line(segment, CP_RED);
                }
                i = j + 2;
            } else {
                segment[seg_len] = line[i];
                seg_len++;
                i++;
            }
            continue;
        }

        //普通字符，写到segment里
        segment[seg_len] = line[i];
        seg_len++;
        i++;
        // 防
        if (seg_len >= 1022) {
            segment[seg_len] = '\0';
            tui_add_chat_line(segment, 0);
            seg_len = 0;
        }
    }
    // 最后如果有剩余的普通文字，也要输出
    if (seg_len > 0) {
        segment[seg_len] = '\0';
        tui_add_chat_line(segment, 0);
    }
}
