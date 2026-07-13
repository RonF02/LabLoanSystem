#include "sync_ui.h"
#include "sync.h"
#include "utils.h"

#include <stdio.h>

void sync_data_menu(const Item items[], int item_count,
                    const BorrowRecord borrows[], int borrow_count,
                    const ReturnRecord returns[], int return_count)
{
    clear_screen();
    print_items_preview(items, item_count);
    puts("\n=== 数据同步 ===");

    if (sync_data(items, item_count, borrows, borrow_count, returns, return_count))
    {
        puts("数据同步成功。\n");
        puts("数据已保存到文件。");
    }
    else
    {
        fprintf(stderr, "数据同步失败，请检查文件权限。\n");
    }

    stop();
}
