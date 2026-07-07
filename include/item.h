#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include <stddef.h>
#include "model.h"

// 新增物品：将新物品加入列表，自动分配 ID。若编号重复则返回 false。
bool item_add(Item items[], int *count, const char *code, const char *name,
              const char *model, int quantity, const char *description);

// 修改物品：根据编号找到物品并更新名称、型号、库存、描述。
bool item_update(Item items[], int count, const char *code, const char *name,
                 const char *model, int quantity, const char *description);

// 删除物品：根据编号删除物品，仅当物品未存在活动借用时允许删除。
bool item_remove(Item items[], int *count, const BorrowRecord borrows[],
                 int borrow_count, const char *code);

// 根据编号查询物品，返回下标；未找到返回 -1。
int item_find_by_code(const Item items[], int count, const char *code);

// 根据名称模糊查询物品，将匹配下标写入结果数组，返回匹配数量。
int item_find_by_name(const Item items[], int count, const char *name,
                      int result_indexes[], int result_max);

// 检查库存是否足够，返回 true 表示满足数量需求。
bool item_check_quantity(const Item items[], int count, const char *code,
                         int quantity);

// 调整物品库存（正数为增加，负数为减少）；库存不足时返回 false。
bool item_adjust_quantity(Item items[], int count, const char *code,
                          int delta);

// 获取下一个可用的物品 ID（供新增和外部调用使用）。
int item_next_id(const Item items[], int count);

// 生成一个可用的物品编号，格式如 ITEM-001。
bool item_generate_code(const Item items[], int count, char *code, size_t size);

// 物品管理 UI 入口由 item_ui.c 提供。

#endif // ITEM_H
