#ifndef MARKDOWN_H
#define MARKDOWN_H

// 把一行markdown文字渲染到对话区
// 支持：
//   # 标题 / ## 标题  → 绿色
//   **加粗** / __加粗__ → 红色
//   `代码` → 蓝色
void markdown_render_to_chat(const char *line);

#endif
