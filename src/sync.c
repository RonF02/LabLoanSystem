#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "sync.h"
#include "storage.h"
#include "utils.h"

void sync_data(const Item items[], int item_count,
               const BorrowRecord borrows[], int borrow_count,
               const ReturnRecord returns[], int return_count)
{
    char choice[16];
    while (1)
    {
        clear_screen();
        print_items_preview(items, item_count);
        puts("\n=== 数据同步 ===");
        puts("0. 返回主菜单");
        printf("请选择：");

        if (!read_line(choice, sizeof(choice)))
        {
            puts("读取输入失败。\n");
            return;
        }

        if (strcmp(choice, "0") == 0)
        {
            return;
        }

        puts("输入 0 返回上级。\n");
        char flush[16];
        read_line(flush, sizeof(flush));
    }
}
