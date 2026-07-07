#include "query.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void query_borrow_records(Item items[], int item_cnt, BorrowRecord borrows[], int borrow_cnt, const char *keyword)
{
    int has_result = 0; // 标记是否有匹配结果
    printf("\n========================= 查询结果 =============================\n");
    printf("借用ID\t借用人\t物品编号\t物品名称\t借用数量\t借用日期\t预计归还\t状态\n");
    printf("---------------------------------------------------------\n");

    for (int i = 0; i < borrow_cnt; i++) // 遍历所有借用记录
    {
        BorrowRecord br = borrows[i];
        // 使用规定宏定义
        char item_name[MAX_NAME_LENGTH] = "未知物品";
        for (int j = 0; j < item_cnt; j++)
        {
            if (safe_strcmp(items[j].code, br.item_code) == 0)//safe_strcmp调用utils中的定义，安全调用避免崩溃
            {
                strncpy(item_name, items[j].name, MAX_NAME_LENGTH - 1);
                item_name[MAX_NAME_LENGTH - 1] = '\0';
                break;
            }
        }
        // 无关键字：全部输出；有关键字：匹配名称/编号/借用人;strstr查找算法
        int match = 0;
        if (keyword == NULL || safe_strcmp(keyword, "") == 0)
            match = 1;
        else
        {
            // 物品名称匹配
            if (strstr(item_name, keyword) != NULL) match = 1;
            // 物品编号匹配
            if (strstr(br.item_code, keyword) != NULL) match = 1;
            // 借用人匹配
            if (strstr(br.user, keyword) != NULL) match = 1;
        }
        if (!match) continue;

        has_result = 1;
        char status_str[16] = "未归还";
        if (br.status == BORROW_RETURNED) // 枚举判断，model.h头文件中参数
            strcpy(status_str, "已归还");
        printf("%d\t%s\t%s\t%s\t%d\t%s\t%s\t%s\n",
               br.id, br.user, br.item_code, item_name,
               br.quantity, br.borrow_date, br.due_date, status_str);
    }
    if (!has_result)
        printf("无匹配记录\n");
    printf("=========================================================\n\n");
}