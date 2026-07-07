#ifndef REPORT_H
#define REPORT_H
// report.h 只保留报表生成接口，UI 入口由 report_ui.h 提供。
#include "model.h"

// 1. 打印完整出入账明细报表（所有借用+对应归还）
void print_detail_report(Item items[], int item_cnt,
                         BorrowRecord borrows[], int borrow_cnt,
                         ReturnRecord returns[], int return_cnt);

// 2. 物品借用汇总统计报表：每种物品总借出、总归还、当前在外数量
void print_stat_report(Item items[], int item_cnt,
                       BorrowRecord borrows[], int borrow_cnt,
                       ReturnRecord returns[], int return_cnt);

#endif