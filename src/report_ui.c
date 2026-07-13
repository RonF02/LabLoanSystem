#include "report_ui.h"
#include "report.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

static void print_detail_report_rows(const DetailReportRow rows[], int row_count)
{
    printf("\n==================== 出入账明细总报表 ====================\n");
    printf("借用ID\t借用人\t物品\t借出量\t借出日期\t预计归还\t归还日期\t归还量\t状态\n");
    printf("-------------------------------------------------------------------------\n");

    for (int i = 0; i < row_count; ++i)
    {
        const DetailReportRow *row = &rows[i];
        printf("%d\t%s\t%s\t%d\t%s\t%s\t%s\t%d\t%s\n",
               row->borrow_id,
               row->user,
               row->item_name,
               row->borrow_quantity,
               row->borrow_date,
               row->due_date,
               row->return_date,
               row->return_quantity,
               row->status == BORROW_RETURNED ? "已归还" : "未归还");
    }

    printf("=========================================================================\n\n");
}

static void print_stat_report_rows(const StatReportRow rows[], int row_count)
{
    printf("\n==================== 物品借用统计汇总报表 ====================\n");
    printf("物品编号\t物品名称\t总借出次数\t总借出数量\t总归还数量\t当前在外\n");
    printf("-------------------------------------------------------------------------\n");

    for (int i = 0; i < row_count; ++i)
    {
        const StatReportRow *row = &rows[i];
        printf("%s\t%s\t%d\t%d\t%d\t%d\n",
               row->item_code,
               row->item_name,
               row->total_borrow_times,
               row->total_borrow_quantity,
               row->total_return_quantity,
               row->out_quantity);
    }

    printf("=========================================================================\n\n");
}

static void show_report_menu(const Item items[], int item_count)
{
    clear_screen();
    print_items_preview(items, item_count);
    puts("\n=== 报表查询 ===");
    puts("1. 出入账明细报表");
    puts("2. 借用统计汇总报表");
    puts("0. 返回主菜单");
    printf("请选择：");
}

bool report_manage_menu(Item items[], int item_count,
                        BorrowRecord borrows[], int borrow_count,
                        ReturnRecord returns[], int return_count)
{
    char choice[16];
    while (1)
    {
        show_report_menu(items, item_count);

        if (!read_line(choice, sizeof(choice)))
        {
            puts("读取输入失败。\n");
            stop();
            return false;
        }

        if (strcmp(choice, "1") == 0)
        {
            DetailReportRow rows[MAX_BORROW_RECORDS];
            int row_count = build_detail_report(items, item_count, borrows, borrow_count, returns, return_count,
                                                rows, MAX_BORROW_RECORDS);
            print_detail_report_rows(rows, row_count);
            stop();
            continue;
        }
        else if (strcmp(choice, "2") == 0)
        {
            StatReportRow rows[MAX_ITEMS];
            int row_count = build_stat_report(items, item_count, borrows, borrow_count, returns, return_count,
                                              rows, MAX_ITEMS);
            print_stat_report_rows(rows, row_count);
            stop();
            continue;
        }
        else if (strcmp(choice, "0") == 0)
        {
            return true;
        }

        puts("无效选择。\n");
        stop();
    }
}
