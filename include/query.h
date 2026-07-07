#ifndef QUERY_H
#define QUERY_H
#include "model.h"

// 按关键字查询借用记录：匹配物品名称/物品编号/借用人
// keyword：搜索关键字，传空字符串则查询全部
void query_borrow_records(Item items[], int item_cnt,
                          BorrowRecord borrows[], int borrow_cnt,
                          const char *keyword);

#endif