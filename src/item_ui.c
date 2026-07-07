#include "item_ui.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

// 显示物品具体信息
static void print_item(const Item *item)
{
    printf("ID: %d\n", item->id);
    printf("编号: %s\n", item->code);
    printf("名称: %s\n", item->name);
    printf("型号: %s\n", item->model);
    printf("库存: %d\n", item->quantity);
    printf("描述: %s\n", item->description);
    puts("------------------------");
}

// 物品管理菜单，提供新增、修改、删除、查询功能
bool item_manage_menu(Item items[], int *item_count, const BorrowRecord borrows[], int borrow_count)
{
    char choice[16];

    while (1)
    {
        clear_screen();
        print_items_preview(items, *item_count);
        puts("\n=== 物品管理 ===");
        puts("1. 新增物品");
        puts("2. 修改物品");
        puts("3. 删除物品");
        puts("4. 查询物品");
        puts("0. 返回主菜单");
        printf("请选择：");

        // 异常处理
        if (!read_line(choice, sizeof(choice)))
        {
            puts("读取输入失败。\n");
            return false;
        }

        // 用户选择新增物品
        if (strcmp(choice, "1") == 0)
        {
            char code[MAX_CODE_LENGTH]; // 缓存编号
            char name[MAX_NAME_LENGTH]; // 缓存名称
            char model[MAX_MODEL_LENGTH]; // 缓存型号
            char description[MAX_DESC_LENGTH]; // 缓存描述
            int quantity; // 缓存库存数量

            if (!item_generate_code(items, *item_count, code, sizeof(code)))
            {
                puts("生成物品编号失败。\n");
                continue;
            }

            printf("请输入物品名称：");
            if (!read_line(name, sizeof(name)) || strlen(name) == 0)
            {
                puts("物品名称不能为空。\n");
                continue;
            }

            printf("请输入物品型号：");
            if (!read_line(model, sizeof(model)) || strlen(model) == 0)
            {
                puts("物品型号不能为空。\n");
                continue;
            }

            if (!read_int("请输入库存数量：", &quantity) || quantity < 0)
            {
                puts("库存数量输入无效。\n");
                continue;
            }

            printf("请输入物品描述：");
            if (!read_line(description, sizeof(description)))
            {
                puts("物品描述读取失败。\n");
                continue;
            }

            // 调用 item_add 函数新增物品
            if (item_add(items, item_count, code, name, model, quantity, description))
            {
                printf("物品新增成功，系统已自动生成物品编号：%s\n\n", code);
                puts("输入 0 返回上级。\n");
                char flush[16];
                read_line(flush, sizeof(flush));
                continue;
            }

            puts("新增物品失败，请检查编号是否重复或输入是否合法。\n");
            continue;
        }
        // 用户选择修改物品 
        else if (strcmp(choice, "2") == 0)
        {
            char code[MAX_CODE_LENGTH]; // 缓存编号
            char name[MAX_NAME_LENGTH]; // 缓存名称
            char model[MAX_MODEL_LENGTH]; // 缓存型号
            char description[MAX_DESC_LENGTH]; // 缓存描述
            int quantity;

            printf("请输入要修改的物品编号：");
            if (!read_line(code, sizeof(code)) || strlen(code) == 0)
            {
                puts("物品编号不能为空。\n");
                continue;
            }

            printf("请输入新名称：");
            if (!read_line(name, sizeof(name)) || strlen(name) == 0)
            {
                puts("物品名称不能为空。\n");
                continue;
            }

            printf("请输入新型号：");
            if (!read_line(model, sizeof(model)) || strlen(model) == 0)
            {
                puts("物品型号不能为空。\n");
                continue;
            }

            if (!read_int("请输入新库存数量：", &quantity) || quantity < 0)
            {
                puts("库存数量输入无效。\n");
                continue;
            }

            printf("请输入新描述：");
            if (!read_line(description, sizeof(description)))
            {
                puts("物品描述读取失败。\n");
                continue;
            }

            // 调用 item_update 函数修改物品信息
            if (item_update(items, *item_count, code, name, model, quantity, description))
            {
                puts("物品修改成功。\n");
                puts("输入 0 返回上级。\n");
                char flush[16];
                read_line(flush, sizeof(flush));
                continue;
            }

            puts("修改物品失败，请检查物品编号是否存在。\n");
            continue;
        }
        // 用户选择删除物品
        else if (strcmp(choice, "3") == 0)
        {
            char code[MAX_CODE_LENGTH]; // 缓存编号

            printf("请输入要删除的物品编号：");
            if (!read_line(code, sizeof(code)) || strlen(code) == 0)
            {
                puts("物品编号不能为空。\n");
                continue;
            }

            // 调用 item_remove 函数删除物品
            if (item_remove(items, item_count, borrows, borrow_count, code))
            {
                puts("物品删除成功。\n");
                puts("输入 0 返回上级。\n");
                char flush[16];
                read_line(flush, sizeof(flush));
                continue;
            }

            puts("删除物品失败，请检查编号是否存在或是否存在活动借用。\n");
            continue;
        }
        
        // 用户选择查询物品
        else if (strcmp(choice, "4") == 0)
        {
            char query[MAX_NAME_LENGTH]; // 缓存查询关键字

            printf("请输入物品编号或名称关键字：");
            if (!read_line(query, sizeof(query)) || strlen(query) == 0)
            {
                puts("查询关键字不能为空。\n");
                continue;
            }

            // 先按编号查询
            int index = item_find_by_code(items, *item_count, query);
            if (index >= 0)
            {
                print_item(&items[index]); // 打印物品信息
                puts("输入 0 返回上级。\n");
                char flush[16];
                read_line(flush, sizeof(flush));
                continue;
            }

            int result_indexes[MAX_ITEMS]; // 缓存查询结果索引
            
            // 按名称查询
            int found = item_find_by_name(items, *item_count, query, result_indexes, MAX_ITEMS);
            if (found > 0)
            {
                for (int i = 0; i < found; ++i)
                {
                    print_item(&items[result_indexes[i]]); // 打印物品信息
                }
                puts("输入 0 返回上级。\n");
                char flush[16];
                read_line(flush, sizeof(flush));
                continue;
            }

            puts("未找到匹配的物品。\n");
            puts("输入 0 返回上级。\n");
            char flush[16];
            read_line(flush, sizeof(flush));
            continue;
        }
        else if (strcmp(choice, "0") == 0)
        {
            return true;
        }

        puts("无效选择。\n");
        puts("输入 0 返回上级。\n");
        char flush[16];
        read_line(flush, sizeof(flush));
    }
}
