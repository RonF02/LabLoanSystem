#include "borrow_ui.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

bool borrow_manage_menu(BorrowRecord borrows[], int *borrow_count,
                        ReturnRecord returns[], int *return_count,
                        Item items[], int item_count)
{
    char choice[16];
    puts("\n=== 借用管理 ===");
    puts("1. 借用登记");
    puts("2. 归还登记");
    puts("0. 返回主菜单");
    printf("请选择：");

    if (!read_line(choice, sizeof(choice)))
    {
        puts("读取输入失败。\n");
        return false;
    }

    if (strcmp(choice, "1") == 0)
    {
        char user[MAX_USER_LENGTH];
        char item_code[MAX_CODE_LENGTH];
        char borrow_date[MAX_DATE_LENGTH];
        char due_date[MAX_DATE_LENGTH];
        int quantity;

        printf("请输入借用人：");
        if (!read_line(user, sizeof(user)) || strlen(user) == 0)
        {
            puts("借用人不能为空。\n");
            return false;
        }

        printf("请输入物品编号：");
        if (!read_line(item_code, sizeof(item_code)) || strlen(item_code) == 0)
        {
            puts("物品编号不能为空。\n");
            return false;
        }

        if (!read_int("请输入借用数量：", &quantity) || quantity <= 0)
        {
            puts("借用数量输入无效。\n");
            return false;
        }

        printf("请输入借用日期 (YYYY-MM-DD)：");
        if (!read_line(borrow_date, sizeof(borrow_date)) || !is_valid_date(borrow_date))
        {
            puts("借用日期格式无效。\n");
            return false;
        }

        printf("请输入预计归还日期 (YYYY-MM-DD)：");
        if (!read_line(due_date, sizeof(due_date)) || !is_valid_date(due_date))
        {
            puts("预计归还日期格式无效。\n");
            return false;
        }

        if (borrow_register(borrows, borrow_count, items, item_count,
                            user, item_code, quantity, borrow_date, due_date))
        {
            puts("借用登记成功。\n");
            return true;
        }

        puts("借用登记失败，请检查物品编号、库存和输入信息。\n");
        return false;
    }
    else if (strcmp(choice, "2") == 0)
    {
        int borrow_id;
        int quantity;
        char return_date[MAX_DATE_LENGTH];

        if (!read_int("请输入借用记录 ID：", &borrow_id) || borrow_id <= 0)
        {
            puts("借用记录 ID 输入无效。\n");
            return false;
        }

        if (!read_int("请输入归还数量：", &quantity) || quantity <= 0)
        {
            puts("归还数量输入无效。\n");
            return false;
        }

        printf("请输入归还日期 (YYYY-MM-DD)：");
        if (!read_line(return_date, sizeof(return_date)) || !is_valid_date(return_date))
        {
            puts("归还日期格式无效。\n");
            return false;
        }

        if (borrow_return(borrows, *borrow_count, returns, return_count,
                          items, item_count, borrow_id, quantity, return_date))
        {
            puts("归还登记成功。\n");
            return true;
        }

        puts("归还登记失败，请检查借用记录 ID、数量和日期。\n");
        return false;
    }
    else if (strcmp(choice, "0") == 0)
    {
        return true;
    }

    puts("无效选择。\n");
    return false;
}
