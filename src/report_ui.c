#include "report_ui.h"
#include "report.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

static void show_report_menu(void)
{
    puts("\n=== 报表查询 ===");
    puts("1. 出入账明细报表");
    puts("2. 借用统计汇总报表");
    puts("0. 返回主菜单");
    printf("请选择：");
}

bool report_manage_menu(Item items[], int item_count,
                        BorrowRecord borrows[], int borrow_count,
                        ReturnRecord returns[], int return_count)
{
    char choice[16];
    show_report_menu();

    if (!read_line(choice, sizeof(choice)))
    {
        puts("读取输入失败。\n");
        return false;
    }

    if (strcmp(choice, "1") == 0)
    {
        print_detail_report(items, item_count, borrows, borrow_count, returns, return_count);
        return true;
    }
    else if (strcmp(choice, "2") == 0)
    {
        print_stat_report(items, item_count, borrows, borrow_count, returns, return_count);
        return true;
    }
    else if (strcmp(choice, "0") == 0)
    {
        return true;
    }

    puts("无效选择。\n");
    return false;
}
