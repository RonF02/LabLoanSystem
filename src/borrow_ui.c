#include "borrow_ui.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

// 借用管理菜单，提供借用登记和归还登记功能
bool borrow_manage_menu(BorrowRecord borrows[], int *borrow_count, ReturnRecord returns[], int *return_count, Item items[], int item_count)
{
    char choice[16];

    while (1)
    {
        clear_screen();
        print_items_preview(items, item_count);
        puts("\n=== 借用管理 ===");
        puts("1. 借用登记");
        puts("2. 归还登记");
        puts("0. 返回主菜单");
        printf("请选择：");

        if (!read_line(choice, sizeof(choice)))
        {
            puts("读取输入失败。\n");
            return false;
        }

        if (strcmp(choice, "1") == 0)
        {
            char user[MAX_USER_LENGTH]; // 缓存借用人
            char item_code[MAX_CODE_LENGTH]; // 缓存物品编号
            char borrow_date[MAX_DATE_LENGTH]; // 缓存借用日期
            char due_date[MAX_DATE_LENGTH]; // 缓存预计归还日期
            int quantity; // 缓存借用数量
            time_t now = time(NULL); // 获取当前系统时间
            struct tm *local = localtime(&now); // 将系统时间转换为本地时间结构

            printf("请输入借用人：");
            if (!read_line(user, sizeof(user)) || strlen(user) == 0)
            {
                puts("借用人不能为空。\n");
                continue;
            }

            printf("请输入物品编号：");
            if (!read_line(item_code, sizeof(item_code)) || strlen(item_code) == 0)
            {
                puts("物品编号不能为空。\n");
                continue;
            }

            if (!read_int("请输入借用数量：", &quantity) || quantity <= 0)
            {
                puts("借用数量输入无效。\n");
                continue;
            }

            if (!local)
            {
                puts("获取系统时间失败。\n");
                continue;
            }

            // 将当前日期格式化为 YYYY-MM-DD 形式
            snprintf(borrow_date, sizeof(borrow_date), "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);

            printf("请输入预计归还日期 (YYYY-MM-DD)：");
            if (!read_line(due_date, sizeof(due_date)) || !is_valid_date(due_date))
            {
                puts("预计归还日期格式无效。\n");
                continue;
            }
            if (!is_date_before_or_equal(borrow_date, due_date))
            {
                puts("预计归还日期不能早于借用日期。\n");
                continue;
            }

            int generated_id = borrow_next_id(borrows, *borrow_count);
            if (borrow_register(borrows, borrow_count, items, item_count,  user, item_code, quantity, borrow_date, due_date))
            {
                printf("借用登记成功，系统已自动生成借用记录 ID：%d\n\n", generated_id);
                puts("输入 0 返回上级。\n");
                char flush[16];
                read_line(flush, sizeof(flush));
                continue;
            }

            puts("借用登记失败，请检查物品编号、库存和输入信息。\n");
            puts("输入 0 返回上级。\n");
            char flush[16];
            read_line(flush, sizeof(flush));
            continue;
        }

        // 用户选择归还登记
        else if (strcmp(choice, "2") == 0)
        {
            int borrow_id;
            int quantity;
            char return_date[MAX_DATE_LENGTH];


            // 输入数据并进行验证
            if (!read_int("请输入借用记录 ID：", &borrow_id) || borrow_id <= 0)
            {
                puts("借用记录 ID 输入无效。\n");
                continue;
            }

            if (!read_int("请输入归还数量：", &quantity) || quantity <= 0)
            {
                puts("归还数量输入无效。\n");
                continue;
            }

            printf("请输入归还日期 (YYYY-MM-DD)：");
            if (!read_line(return_date, sizeof(return_date)) || !is_valid_date(return_date))
            {
                puts("归还日期格式无效。\n");
                continue;
            }

            // 调用 borrow_return 函数进行归还登记
            if (borrow_return(borrows, *borrow_count, returns, return_count, items, item_count, borrow_id, quantity, return_date))
            {
                puts("归还登记成功。\n");
                puts("输入 0 返回上级。\n");
                char flush[16];
                read_line(flush, sizeof(flush));
                continue;
            }

            puts("归还登记失败，请检查借用记录 ID、数量和日期。\n");
            continue;
        }
        else if (strcmp(choice, "0") == 0)
        {
            return true;
        }

        puts("无效选择。\n");
    }
}
