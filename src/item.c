#include "item.h"
#include "utils.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 自动生成物品编号，格式如 001，确保编号唯一。
bool item_generate_code(const Item items[], int count, char *code, size_t size)
{
    if (!code || size == 0) // 如果缓存为空或大小为0，返回 false
        return false;

    int next_id = item_next_id(items, count); // 获取下一个可用的物品 ID
    int written = snprintf(code, size, "%03d", next_id); // 将 ID 格式化为三位数的字符串写入缓存
    return written > 0 && (size_t)written < size; // 如果写入成功且未超出缓存大小，返回 true
}

// 计算下一个可用的物品 ID，保证 ID 唯一且从 1 开始递增。
int item_next_id(const Item items[], int count)
{
    int max_id = 0; // 初始化最大 ID 为 0
    for (int i = 0; i < count; ++i) // 遍历所有物品，找到最大 ID
    {
        if (items[i].id > max_id)
            max_id = items[i].id;
    }
    return max_id + 1; // 返回下一个可用的 ID，即最大 ID + 1
}

// 按物品编号查找，返回下标；未找到返回 -1。
int item_find_by_code(const Item items[], int count, const char *code)
{
    if (!code) // 如果编号为空，返回 -1
        return -1;

    for (int i = 0; i < count; ++i) // 遍历所有物品，查找匹配的编号
    {
        if (safe_strcmp(items[i].code, code) == 0)
            return i;
    }
    return -1; // 未找到匹配的物品，返回 -1
}

// 按名称模糊匹配，返回匹配到的数量。
int item_find_by_name(const Item items[], int count, const char *name, int result_indexes[], int result_max)
{
    if (!name || !result_indexes || result_max <= 0) // 如果名称为空、结果数组为空或最大结果数小于等于0，返回 0
        return 0;

    int matched = 0; // 初始化匹配数量为 0
    for (int i = 0; i < count && matched < result_max; ++i) // 遍历所有物品，直到匹配数量达到最大结果数
    {
        if (strstr(items[i].name, name) != NULL) // 如果物品名称中包含查询关键字，记录下标
        {
            result_indexes[matched++] = i; // 将匹配的物品下标写入结果数组，并增加匹配数量
        }
    }
    return matched; // 返回匹配到的物品数量
}

// 检查库存是否满足数量需求。
bool item_check_quantity(const Item items[], int count, const char *code, int quantity)
{
    int idx = item_find_by_code(items, count, code); // 根据编号查找物品下标
    if (idx < 0) // 如果未找到物品，返回 false
        return false;
    if (quantity < 0) // 如果需求数量为负数，返回 false
        return false;
    return items[idx].quantity >= quantity; // 返回库存是否满足需求数量
}

// 调整库存数量，负数表示扣减，正数表示增加。
bool item_adjust_quantity(Item items[], int count, const char *code, int delta)
{
    int idx = item_find_by_code(items, count, code); // 根据编号查找物品下标
    if (idx < 0) // 如果未找到物品，返回 false
        return false; 

    int new_qty = items[idx].quantity + delta; // 计算调整后的库存数量
    if (new_qty < 0) // 如果调整后库存为负数，返回 false
        return false;

    items[idx].quantity = new_qty; // 更新库存数量
    return true; // 返回 true 表示调整成功
}

// 新增物品：先验证编号不重复，再写入数组末尾。
bool item_add(Item items[], int *count, const char *code, const char *name, const char *model, int quantity, const char *description)
{
    // 参数验证：检查指针是否为空，数量是否为负数，名称、型号、描述是否为空。
    if (!count || !name || !model || !description)
        return false;
    if (*count >= MAX_ITEMS)
        return false;
    if (quantity < 0)
        return false;

    // 如果编号为空，则自动生成编号
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

    // 将新物品写入数组末尾
    Item *new_item = &items[*count]; // 获取新物品的指针

    // 通过指针在结构体中设置新物品的属性，包括 ID、编号、名称、型号、库存数量和描述
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
bool item_update(Item items[], int count, const char *code, const char *name, const char *model, int quantity, const char *description)
{
    // 参数验证：检查指针是否为空，数量是否为负数。
    if (!code || !name || !model || !description)
        return false;
    if (quantity < 0)
        return false;

    // 根据编号查找物品下标，如果未找到则返回 false
    int idx = item_find_by_code(items, count, code);
    if (idx < 0)
        return false;

    // 更新物品在结构体数组中的信息，包括名称、型号、库存数量和描述
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
bool item_remove(Item items[], int *count, const BorrowRecord borrows[], int borrow_count, const char *code)
{
    // 参数验证：检查指针是否为空。
    if (!count || !code)
        return false;

    int idx = item_find_by_code(items, *count, code);
    if (idx < 0)
        return false;

    // 检查是否存在活动借用
    for (int i = 0; i < borrow_count; ++i)
    {
        if (safe_strcmp(borrows[i].item_code, code) == 0 && borrows[i].status == BORROW_ACTIVE)
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
