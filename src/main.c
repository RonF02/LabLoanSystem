#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "model.h"
#include "storage.h"
#include "utils.h"
#include "borrow_ui.h"
#include "item_ui.h"
#include "report_ui.h"
#include "sync.h"

static void show_main_menu(void)
{
    puts("=== 实验室物品出入账管理系统 ===");
    puts("1. 物品管理");
    puts("2. 借用管理");
    puts("3. 报表查询");
    puts("4. 数据同步");
    puts("0. 退出");
    printf("请选择：");
}

static void handle_placeholder(const char *name)
{
    printf("[%s] 功能尚未实现，后续 P2-P5 负责开发。\n\n", name);
}

int main(void)
{
#ifdef _WIN32
    /*
     * 在 Windows 控制台中启用 UTF-8 输出/输入，配合 chcp 65001 使用。
     * 这能改善 printf 输出中文的乱码问题（仍需终端字体支持 UTF-8）。
     */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    Item items[MAX_ITEMS];
    BorrowRecord borrows[MAX_BORROW_RECORDS];
    ReturnRecord returns[MAX_RETURN_RECORDS];
    int item_count = 0;
    int borrow_count = 0;
    int return_count = 0;
    char choice[16];

    if (!ensure_data_directory()) 
    {
        fprintf(stderr, "无法创建数据目录或数据文件，请检查权限。\n");
        return EXIT_FAILURE;
    }

    if (!load_items(items, &item_count)) 
    {
        fprintf(stderr, "警告：读取物品数据失败，系统将以空数据启动。\n");
    }
    if (!load_borrow_records(borrows, &borrow_count)) 
    {
        fprintf(stderr, "警告：读取借用数据失败，系统将以空数据启动。\n");
    }
    if (!load_return_records(returns, &return_count)) 
    {
        fprintf(stderr, "警告：读取归还数据失败，系统将以空数据启动。\n");
    }

    while (1) 
    {
        show_main_menu();
        if (!read_line(choice, sizeof(choice))) 
        {
            puts("输入读取失败，程序退出。");
            break;
        }

        if (strcmp(choice, "0") == 0) 
        {
            break;
        } 
        else if (strcmp(choice, "1") == 0) 
        {
            item_manage_menu(items, &item_count, borrows, borrow_count);
        } 
        else if (strcmp(choice, "2") == 0) 
        {
            borrow_manage_menu(borrows, &borrow_count, returns, &return_count, items, item_count);
        } 
        else if (strcmp(choice, "3") == 0) 
        {
            report_manage_menu(items, item_count, borrows, borrow_count, returns, return_count);
        } 
        else if (strcmp(choice, "4") == 0) 
        {
            sync_data(items, item_count, borrows, borrow_count, returns, return_count);
        } 
        else 
        {
            puts("无效选择，请重新输入。\n");
        }
    }

    bool items_saved = save_items(items, item_count);
    bool borrows_saved = save_borrow_records(borrows, borrow_count);
    bool returns_saved = save_return_records(returns, return_count);
    if (!items_saved || !borrows_saved || !returns_saved) 
    {
        fprintf(stderr, "退出时保存数据失败，请检查文件权限。\n");
        return EXIT_FAILURE;
    }

    puts("系统已退出，数据已保存。");
    return EXIT_SUCCESS;
}
