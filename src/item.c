#include "item.h"
#include "utils.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool item_generate_code(const Item items[], int count, char *code, size_t size)
{
    if (!code || size == 0)
        return false;

    int next_id = item_next_id(items, count);
    int written = snprintf(code, size, "ITEM-%03d", next_id);
    return written > 0 && (size_t)written < size;
}

// 计算下一个可用的物品 ID，保证 ID 唯一且从 1 开始递增。
int item_next_id(const Item items[], int count)
{
    int max_id = 0;
    for (int i = 0; i < count; ++i)
    {
        if (items[i].id > max_id)
            max_id = items[i].id;
    }
    return max_id + 1;
}

// 按物品编号查找，返回下标；未找到返回 -1。
int item_find_by_code(const Item items[], int count, const char *code)
{
    if (!code)
        return -1;
    for (int i = 0; i < count; ++i)
    {
        if (safe_strcmp(items[i].code, code) == 0)
            return i;
    }
    return -1;
}

// 按名称模糊匹配，返回匹配到的数量。
int item_find_by_name(const Item items[], int count, const char *name, int result_indexes[], int result_max)
{
    if (!name || !result_indexes || result_max <= 0)
        return 0;

    int matched = 0;
    for (int i = 0; i < count && matched < result_max; ++i)
    {
        if (strstr(items[i].name, name) != NULL)
        {
            result_indexes[matched++] = i;
        }
    }
    return matched;
}

// 检查库存是否满足数量需求。
bool item_check_quantity(const Item items[], int count, const char *code,
                         int quantity)
{
    int idx = item_find_by_code(items, count, code);
    if (idx < 0)
        return false;
    if (quantity < 0)
        return false;
    return items[idx].quantity >= quantity;
}

// 调整库存数量，负数表示扣减，正数表示增加。
bool item_adjust_quantity(Item items[], int count, const char *code,
                          int delta)
{
    int idx = item_find_by_code(items, count, code);
    if (idx < 0)
        return false;

    int new_qty = items[idx].quantity + delta;
    if (new_qty < 0)
        return false;

    items[idx].quantity = new_qty;
    return true;
}

// 新增物品：先验证编号不重复，再写入数组末尾。
bool item_add(Item items[], int *count, const char *code, const char *name,
              const char *model, int quantity, const char *description)
{
    if (!count || !name || !model || !description)
        return false;
    if (*count >= MAX_ITEMS)
        return false;
    if (quantity < 0)
        return false;

    char generated_code[MAX_CODE_LENGTH];
    const char *final_code = code;
    if (!code || strlen(code) == 0)
    {
        if (!item_generate_code(items, *count, generated_code, sizeof(generated_code)))
            return false;
        final_code = generated_code;
    }

    if (strlen(final_code) == 0 || strlen(name) == 0 || strlen(model) == 0)
        return false;

    // 编号不得重复
    if (item_find_by_code(items, *count, final_code) >= 0)
        return false;

    Item *new_item = &items[*count];
    new_item->id = item_next_id(items, *count);
    strncpy(new_item->code, final_code, MAX_CODE_LENGTH - 1);
    new_item->code[MAX_CODE_LENGTH - 1] = '\0';
    strncpy(new_item->name, name, MAX_NAME_LENGTH - 1);
    new_item->name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(new_item->model, model, MAX_MODEL_LENGTH - 1);
    new_item->model[MAX_MODEL_LENGTH - 1] = '\0';
    new_item->quantity = quantity;
    strncpy(new_item->description, description, MAX_DESC_LENGTH - 1);
    new_item->description[MAX_DESC_LENGTH - 1] = '\0';

    (*count)++;
    return true;
}

// 修改物品信息，允许更新名称、型号、库存和描述。
bool item_update(Item items[], int count, const char *code, const char *name,
                 const char *model, int quantity, const char *description)
{
    if (!code || !name || !model || !description)
        return false;
    if (quantity < 0)
        return false;

    int idx = item_find_by_code(items, count, code);
    if (idx < 0)
        return false;

    strncpy(items[idx].name, name, MAX_NAME_LENGTH - 1);
    items[idx].name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(items[idx].model, model, MAX_MODEL_LENGTH - 1);
    items[idx].model[MAX_MODEL_LENGTH - 1] = '\0';
    items[idx].quantity = quantity;
    strncpy(items[idx].description, description, MAX_DESC_LENGTH - 1);
    items[idx].description[MAX_DESC_LENGTH - 1] = '\0';

    return true;
}

// 删除物品：如果该物品存在活动借用记录则拒绝删除。
bool item_remove(Item items[], int *count, const BorrowRecord borrows[],
                 int borrow_count, const char *code)
{
    if (!count || !code)
        return false;

    int idx = item_find_by_code(items, *count, code);
    if (idx < 0)
        return false;

    // 检查是否存在活动借用
    for (int i = 0; i < borrow_count; ++i)
    {
        if (safe_strcmp(borrows[i].item_code, code) == 0 &&
            borrows[i].status == BORROW_ACTIVE)
        {
            return false;
        }
    }

    // 前移覆盖
    for (int i = idx; i < *count - 1; ++i)
    {
        items[i] = items[i + 1];
    }
    (*count)--;

    return true;
}
