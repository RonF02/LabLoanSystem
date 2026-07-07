#include <stdio.h>
#include <stdbool.h>

#include "sync.h"
#include "storage.h"

void sync_data(const Item items[], int item_count,
               const BorrowRecord borrows[], int borrow_count,
               const ReturnRecord returns[], int return_count)
{
    bool items_saved = save_items(items, item_count);
    bool borrows_saved = save_borrow_records(borrows, borrow_count);
    bool returns_saved = save_return_records(returns, return_count);

    if (items_saved && borrows_saved && returns_saved)
    {
        puts("数据已成功同步。");
    }
    else
    {
        puts("数据同步失败，请检查文件写入权限。");
    }
}
