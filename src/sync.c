#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "sync.h"
#include "storage.h"
#include "utils.h"

void sync_items(const Item items[], int item_count)
{
    if (!save_items(items, item_count))
    {
        fprintf(stderr, "保存物品数据失败，请检查文件权限。\n");
    }
    else
    {
        puts("物品数据同步成功。\n");
    }
}

void sync_borrows(const BorrowRecord borrows[], int borrow_count)
{
    if (!save_borrow_records(borrows, borrow_count))
    {
        fprintf(stderr, "保存借用记录数据失败，请检查文件权限。\n");
    }
    else
    {
        puts("借用记录数据同步成功。\n");
    }
}

void sync_returns(const ReturnRecord returns[], int return_count)
{
    if (!save_return_records(returns, return_count))
    {
        fprintf(stderr, "保存归还记录数据失败，请检查文件权限。\n");
    }
    else
    {
        puts("归还记录数据同步成功。\n");
    }
}

void sync_data(const Item items[], int item_count, const BorrowRecord borrows[], int borrow_count, const ReturnRecord returns[], int return_count)
{
    bool items_saved = save_items(items, item_count);
    bool borrows_saved = save_borrow_records(borrows, borrow_count);
    bool returns_saved = save_return_records(returns, return_count);
    if (!items_saved)
    {
        fprintf(stderr, "保存物品数据失败，请检查文件权限。\n");
        return;
    }
    if (!borrows_saved)
    {
        fprintf(stderr, "保存借用记录数据失败，请检查文件权限。\n");
        return;
    }
    if (!returns_saved)
    {
        fprintf(stderr, "保存归还记录数据失败，请检查文件权限。\n");
        return;
    }
    puts("数据同步成功。\n");
}

void sync_data_menu(const Item items[], int item_count, const BorrowRecord borrows[], int borrow_count, const ReturnRecord returns[], int return_count)
{
    char choice[16];
    clear_screen();
    print_items_preview(items, item_count);
    puts("\n=== 数据同步 ===");
    sync_data(items, item_count, borrows, borrow_count, returns, return_count);
    puts("\n数据已保存到文件。");
    
    stop();
}
