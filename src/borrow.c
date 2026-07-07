#include "borrow.h"
#include "item.h"
#include "utils.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 计算下一个可用的借用记录 ID，保证 ID 唯一且从 1 开始递增。
int borrow_next_id(const BorrowRecord borrows[], int count)
{
    int max_id = 0;
    for (int i = 0; i < count; ++i)
    {
        if (borrows[i].id > max_id)
            max_id = borrows[i].id;
    }
    return max_id + 1;
}

// 计算下一个可用的归还记录 ID，保证 ID 唯一且从 1 开始递增。
int return_next_id(const ReturnRecord returns[], int count)
{
    int max_id = 0;
    for (int i = 0; i < count; ++i)
    {
        if (returns[i].id > max_id)
            max_id = returns[i].id;
    }
    return max_id + 1;
}

// 根据借用记录 ID 查找借用记录下标，未找到返回 -1。
int borrow_find_by_id(const BorrowRecord borrows[], int count, int id)
{
    for (int i = 0; i < count; ++i)
    {
        if (borrows[i].id == id)
            return i;
    }
    return -1;
}

// 根据物品编号查找所有借用记录下标，返回匹配数量。
int borrow_find_by_item_code(const BorrowRecord borrows[], int count, const char *item_code, int result_indexes[], int result_max)
{
    if (!item_code || !result_indexes || result_max <= 0)
        return 0;

    int matched = 0;
    for (int i = 0; i < count && matched < result_max; ++i) // O(n)匹配
    {
        if (safe_strcmp(borrows[i].item_code, item_code) == 0)
        {
            result_indexes[matched++] = i;
        }
    }
    return matched;
}

// 根据借用人查找所有借用记录下标，返回匹配数量。
int borrow_find_by_user(const BorrowRecord borrows[], int count, const char *user, int result_indexes[], int result_max)
{
    if (!user || !result_indexes || result_max <= 0)
        return 0;

    int matched = 0;
    for (int i = 0; i < count && matched < result_max; ++i)
    {
        if (strstr(borrows[i].user, user) != NULL)
        {
            result_indexes[matched++] = i;
        }
    }
    return matched;
}

void print_borrow_record(const BorrowRecord *record)
{
    if (!record)
        return;

    printf("ID:%d 用户:%s 物品编号:%s 数量:%d 借用日期:%s 预计归还日期:%s 状态:%s\n",
           record->id,
           record->user,
           record->item_code,
           record->quantity,
           record->borrow_date,
           record->due_date,
           (record->status == BORROW_ACTIVE) ? "借用中" : "已归还");
}

void print_record_by_name(const BorrowRecord borrows[], int count, const char *user)
{
    int indexes[MAX_BORROW_RECORDS];
    int matched = borrow_find_by_user(borrows, count, user, indexes, MAX_BORROW_RECORDS);
    if (matched == 0)
    {
        puts("未找到相关借用记录。\n");
        return;
    }
    for (int i = 0; i < matched; ++i)
    {
        print_borrow_record(&borrows[indexes[i]]);
    }
}

// 计算某个借用记录已归还的总数量。
int borrow_returned_quantity(const ReturnRecord returns[], int return_count, int borrow_id)
{
    int total = 0;
    for (int i = 0; i < return_count; ++i)
    {
        if (returns[i].borrow_id == borrow_id)
            total += returns[i].quantity;
    }
    return total;
}

// 借用登记：先验证参数与库存，再扣减库存，最后写入借用记录。
bool borrow_register
(
    BorrowRecord borrows[], 
    int *borrow_count,
    Item items[], 
    int item_count,
    const char *user, 
    const char *item_code,
    int quantity, 
    const char *borrow_date,
    const char *due_date
)
{
    // 参数验证
    if (!borrow_count || !user || !item_code || !borrow_date || !due_date)
        return false;
    if (*borrow_count >= MAX_BORROW_RECORDS)
        return false;
    if (quantity <= 0)
        return false;
    if (strlen(user) == 0 || strlen(item_code) == 0 ||
        strlen(borrow_date) == 0 || strlen(due_date) == 0)
        return false;
    if (!is_valid_date(borrow_date) || !is_valid_date(due_date))
        return false;
    if (!is_date_before_or_equal(borrow_date, due_date))
        return false;

    int item_idx = item_find_by_code(items, item_count, item_code);
    if (item_idx < 0)
        return false;
    if (!item_check_quantity(items, item_count, item_code, quantity))
        return false;

    if (!item_adjust_quantity(items, item_count, item_code, -quantity))
        return false;

    // 写入借用记录
    BorrowRecord *record = &borrows[*borrow_count];

    record->id = borrow_next_id(borrows, *borrow_count);

    strncpy(record->user, user, MAX_USER_LENGTH - 1);
    record->user[MAX_USER_LENGTH - 1] = '\0';

    strncpy(record->item_code, item_code, MAX_CODE_LENGTH - 1);
    record->item_code[MAX_CODE_LENGTH - 1] = '\0';
    record->quantity = quantity;

    strncpy(record->borrow_date, borrow_date, MAX_DATE_LENGTH - 1);
    record->borrow_date[MAX_DATE_LENGTH - 1] = '\0';

    strncpy(record->due_date, due_date, MAX_DATE_LENGTH - 1);
    record->due_date[MAX_DATE_LENGTH - 1] = '\0';
    record->status = BORROW_ACTIVE;

    (*borrow_count)++;
    return true;
}

// 归还登记：验证借用记录与剩余数量，恢复库存，追加归还记录并更新借用状态。
bool borrow_return
(
    BorrowRecord borrows[], 
    int borrow_count,
    ReturnRecord returns[], 
    int *return_count,
    Item items[], 
    int item_count,
    int borrow_id, 
    int quantity, 
    const char *return_date
)
{
    // 参数验证
    if (!return_count || !return_date)
        return false;
    if (*return_count >= MAX_RETURN_RECORDS)
        return false;
    if (quantity <= 0)
        return false;
    if (!is_valid_date(return_date))
        return false;

    int borrow_idx = borrow_find_by_id(borrows, borrow_count, borrow_id);
    if (borrow_idx < 0)
        return false;
    if (borrows[borrow_idx].status == BORROW_RETURNED)
        return false;

    int item_idx = item_find_by_code(items, item_count, borrows[borrow_idx].item_code);
    if (item_idx < 0)
        return false;

    int already = borrow_returned_quantity(returns, *return_count, borrow_id);
    if (already + quantity > borrows[borrow_idx].quantity)
        return false;

    if (!item_adjust_quantity(items, item_count, borrows[borrow_idx].item_code, quantity))
        return false;

    // 追加归还记录
    ReturnRecord *ret = &returns[*return_count];
    ret->id = return_next_id(returns, *return_count);
    ret->borrow_id = borrow_id;
    strncpy(ret->return_date, return_date, MAX_DATE_LENGTH - 1);
    ret->return_date[MAX_DATE_LENGTH - 1] = '\0';
    ret->quantity = quantity;

    (*return_count)++;

    // 如果归还数量已达到借用数量，则更新借用状态为已归还
    if (already + quantity >= borrows[borrow_idx].quantity) 
    {
        borrows[borrow_idx].status = BORROW_RETURNED;
    }

    return true;
}
