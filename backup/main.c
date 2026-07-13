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
#include "user.h"
#include "user_ui.h"

static void show_main_menu(const Item items[], int item_count, const User *current_user)
{
    clear_screen();
    puts("=== 实验室物品出入账管理系统 ===");
    printf("当前用户：%s（%s）\n", current_user->username, current_user->level);
    print_items_preview(items, item_count);
    if (is_admin_user(current_user))
    {
        puts("1. 物品管理");
    }
    puts("2. 借用管理");
    if (is_admin_user(current_user))
    {
        puts("3. 报表查询");
        puts("4. 数据同步");
    }
    puts("5. 切换用户");
    puts("0. 退出");
    printf("请选择：");
}

// static void handle_placeholder(const char *name)
// {
//     printf("[%s] 功能尚未实现，后续 P2-P5 负责开发。\n\n", name);
// }

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

    Item items[MAX_ITEMS]; // 创建全局的物品数组，缓存修改
    BorrowRecord borrows[MAX_BORROW_RECORDS]; // 创建全局的借用记录数组，缓存修改
    ReturnRecord returns[MAX_RETURN_RECORDS]; // 创建全局的归还记录数组，缓存修改
    int item_count = 0; // 当前物品数量
    int borrow_count = 0; // 当前借用记录数量
    int return_count = 0; // 当前归还记录数量
    char choice[16]; // 用户输入缓冲区
    User current_user;

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


    if(!show_user_menu(&current_user))
    {
        return EXIT_FAILURE;
    } 

    // 程序入口
    while (1) 
    {
        show_main_menu(items, item_count, &current_user); // 展示主菜单
        if (!read_line(choice, sizeof(choice))) // 使用工具中的 read_line 函数读取用户输入
        {
            puts("输入读取失败，程序退出。");
            break;
        }

        // 根据用户选择调用不同的管理菜单
        if (strcmp(choice, "0") == 0)
        {
            break;
        } 
        else if (strcmp(choice, "1") == 0 && is_admin_user(&current_user)) 
        {
            item_manage_menu(items, &item_count, borrows, borrow_count); // 已人工验收通过
        } 
        else if (strcmp(choice, "2") == 0) 
        {
            borrow_manage_menu(borrows, &borrow_count, returns, &return_count, items, item_count, current_user.username);
        } 
        else if (strcmp(choice, "3") == 0 && is_admin_user(&current_user)) 
        {
            report_manage_menu(items, item_count, borrows, borrow_count, returns, return_count);
        } 
        else if (strcmp(choice, "4") == 0 && is_admin_user(&current_user)) 
        {
            sync_data_menu(items, item_count, borrows, borrow_count, returns, return_count);
        } 
        else if (strcmp(choice, "5") == 0 && is_admin_user(&current_user)) 
        {
            if(!show_user_menu(&current_user))
            {
                break;
            }
        } 
        else 
        {
            puts("无效选择，请重新输入。\n");
            stop();
        }
    }

    // 退出时自动保存数据
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
