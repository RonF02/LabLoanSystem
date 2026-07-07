#include "report.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void print_detail_report(Item items[], int item_cnt,
                         BorrowRecord borrows[], int borrow_cnt,
                         ReturnRecord returns[], int return_cnt)
{
    printf("\n==================== 出入账明细总报表 ====================\n");
    printf("借用ID\t借用人\t物品\t借出量\t借出日期\t预计归还\t归还日期\t归还量\t状态\n");
    printf("-------------------------------------------------------------------------\n");

    for (int i = 0; i < borrow_cnt; i++)
    {
        BorrowRecord br = borrows[i];//依旧调用BorrowRecord
        char item_name[MAX_NAME_LENGTH] = "未知";// 定义物品名称缓冲区，默认值为"未知"（防止匹配不到物品时乱码）
        for (int j = 0; j < item_cnt; j++)
        {
            if (safe_strcmp(items[j].code, br.item_code) == 0)// 使用安全字符串比较，匹配物品编号
            {
                strncpy(item_name, items[j].name, MAX_NAME_LENGTH - 1);// 安全拷贝物品名称，限制长度防止缓冲区溢出
                item_name[MAX_NAME_LENGTH - 1] = '\0';// 手动补字符串结束符，避免strncpy丢失'\0'
                break;// 找到对应物品，跳出物品循环
            }
        }
        // 查找对应归还记录
        char ret_date[MAX_DATE_LENGTH] = "-";// 初始化归还日期，无归还记录时显示"-"
        int ret_qty = 0;//qtr=数量quantity
        for (int r = 0; r < return_cnt; r++)
        {
            if (returns[r].borrow_id == br.id)
            {
                strncpy(ret_date, returns[r].return_date, MAX_DATE_LENGTH - 1);// 拷贝实际归还日期，限制长度防溢出
                ret_date[MAX_DATE_LENGTH - 1] = '\0';
                ret_qty = returns[r].quantity;
                break;
            }
        }
        char stat[16] = "未归还";// 状态文字默认：未归还
        if (br.status == BORROW_RETURNED)
            strcpy(stat, "已归还");
        printf("%d\t%s\t%s\t%d\t%s\t%s\t%s\t%d\t%s\n",
               br.id, br.user, item_name, br.quantity,
               br.borrow_date, br.due_date, ret_date, ret_qty, stat);
    }
    printf("=========================================================================\n\n");
}

void print_stat_report(Item items[], int item_cnt,
                       BorrowRecord borrows[], int borrow_cnt,
                       ReturnRecord returns[], int return_cnt)
{
    printf("\n==================== 物品借用统计汇总报表 ====================\n");
    printf("物品编号\t物品名称\t总借出次数\t总借出数量\t总归还数量\t当前在外\n");
    printf("-------------------------------------------------------------------------\n");

    for (int i = 0; i < item_cnt; i++)
    {
        Item it = items[i];// 获取当前循环的物品信息
        int total_borrow_times = 0;
        int total_borrow_qty = 0;
        int total_return_qty = 0;

        // 遍历所有借用统计该物品
        for (int b = 0; b < borrow_cnt; b++)
        {
            BorrowRecord br = borrows[b];
            if (safe_strcmp(br.item_code, it.code) != 0) 
                continue;// 借用记录的物品编号和当前物品不匹配，跳过本条记录
            total_borrow_times++;// 匹配到该物品的借用记录，借出次数+1
            total_borrow_qty += br.quantity;
            // 匹配归还
            for (int r = 0; r < return_cnt; r++)
            {
                if (returns[r].borrow_id == br.id)
                {
                    total_return_qty += returns[r].quantity;
                }
            }
        }
        int out_stock = total_borrow_qty - total_return_qty;
        printf("%s\t%s\t%d\t%d\t%d\t%d\n",
               it.code, it.name, total_borrow_times,
               total_borrow_qty, total_return_qty, out_stock);
    }
    printf("=========================================================================\n\n");
}