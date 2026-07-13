#!/bin/bash
# 1. 定义你的发布文件夹
DEPLOY_DIR="D:/desktop/release"

# 2. 先用 windeployqt 处理 Qt 自身依赖
windeployqt "$DEPLOY_DIR/LabLoanSystemGUI.exe"

# 3. 循环使用 ldd 查找并复制所有缺失的依赖
while true; do
    # 找所有 exe 和 dll，用 ldd 分析依赖，过滤出 UCRT64 的依赖并复制
    find "$DEPLOY_DIR" -name '*.exe' -o -name '*.dll' | while read file; do
        ldd "$file" | grep "/ucrt64/bin/" | awk '{print $3}' | while read dll; do
            [ -f "$dll" ] && cp -n "$dll" "$DEPLOY_DIR/"
        done
    done
    # 检查是否还有新文件被复制，如果没有则退出循环
    # ... (此处省略复杂检查，简单起见可手动重复运行几次)
    break
done