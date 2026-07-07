#ifndef BORROW_H
#define BORROW_H

#include <stdbool.h>
#include "model.h"

// 借用登记：检查物品库存，不足则拒绝，否则扣减库存并写入借用记录。
bool borrow_register(BorrowRecord borrows[], int *borrow_count,
                     Item items[], int item_count,
                     const char *user, const char *item_code,
                     int quantity, const char *borrow_date,
                     const char *due_date);

// 归还登记：创建归还记录，恢复物品库存，并更新借用状态。
// 若归还数量超过借用数量，返回 false。
bool borrow_return(BorrowRecord borrows[], int borrow_count,
                   ReturnRecord returns[], int *return_count,
                   Item items[], int item_count,
                   int borrow_id, int quantity, const char *return_date);

// 根据 ID 查找借用记录，返回下标；未找到返回 -1。
int borrow_find_by_id(const BorrowRecord borrows[], int count, int id);

// 根据物品编号查询借用记录，将匹配下标写入结果数组，返回匹配数量。
int borrow_find_by_item_code(const BorrowRecord borrows[], int count,
                             const char *item_code,
                             int result_indexes[], int result_max);

// 根据借用人姓名查询借用记录，返回匹配数量。
int borrow_find_by_user(const BorrowRecord borrows[], int count,
                        const char *user,
                        int result_indexes[], int result_max);

// 计算已归还数量：遍历归还记录按 borrow_id 汇总。
int borrow_returned_quantity(const ReturnRecord returns[], int return_count,
                             int borrow_id);

// 获取下一个可用的借用记录 ID。
int borrow_next_id(const BorrowRecord borrows[], int count);

// 获取下一个可用的归还记录 ID。
int return_next_id(const ReturnRecord returns[], int count);

void print_record_by_name(const BorrowRecord borrows[], int count, const char *user);

#endif // BORROW_H
