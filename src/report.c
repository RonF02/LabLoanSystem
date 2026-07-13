#include "report.h"
#include "utils.h"

#include <string.h>

static void copy_string(char *dest, size_t dest_size, const char *src)
{
    if (!dest || dest_size == 0)
        return;

    strncpy(dest, src ? src : "", dest_size - 1);
    dest[dest_size - 1] = '\0';
}

static const char *find_item_name(const Item items[], int item_count, const char *item_code)
{
    for (int i = 0; i < item_count; ++i)
    {
        if (safe_strcmp(items[i].code, item_code) == 0)
            return items[i].name;
    }
    return "未知";
}

int build_detail_report(const Item items[], int item_count,
                        const BorrowRecord borrows[], int borrow_count,
                        const ReturnRecord returns[], int return_count,
                        DetailReportRow rows[], int row_max)
{
    if (!rows || row_max <= 0)
        return 0;

    int row_count = 0;
    for (int i = 0; i < borrow_count && row_count < row_max; ++i)
    {
        const BorrowRecord *borrow = &borrows[i];
        DetailReportRow *row = &rows[row_count++];

        row->borrow_id = borrow->id;
        copy_string(row->user, sizeof(row->user), borrow->user);
        copy_string(row->item_name, sizeof(row->item_name), find_item_name(items, item_count, borrow->item_code));
        row->borrow_quantity = borrow->quantity;
        copy_string(row->borrow_date, sizeof(row->borrow_date), borrow->borrow_date);
        copy_string(row->due_date, sizeof(row->due_date), borrow->due_date);
        copy_string(row->return_date, sizeof(row->return_date), "-");
        row->return_quantity = 0;
        row->status = borrow->status;

        for (int r = 0; r < return_count; ++r)
        {
            if (returns[r].borrow_id == borrow->id)
            {
                copy_string(row->return_date, sizeof(row->return_date), returns[r].return_date);
                row->return_quantity = returns[r].quantity;
                break;
            }
        }
    }

    return row_count;
}

int build_stat_report(const Item items[], int item_count,
                      const BorrowRecord borrows[], int borrow_count,
                      const ReturnRecord returns[], int return_count,
                      StatReportRow rows[], int row_max)
{
    if (!rows || row_max <= 0)
        return 0;

    int row_count = 0;
    for (int i = 0; i < item_count && row_count < row_max; ++i)
    {
        const Item *item = &items[i];
        StatReportRow *row = &rows[row_count++];

        copy_string(row->item_code, sizeof(row->item_code), item->code);
        copy_string(row->item_name, sizeof(row->item_name), item->name);
        row->total_borrow_times = 0;
        row->total_borrow_quantity = 0;
        row->total_return_quantity = 0;

        for (int b = 0; b < borrow_count; ++b)
        {
            const BorrowRecord *borrow = &borrows[b];
            if (safe_strcmp(borrow->item_code, item->code) != 0)
                continue;

            row->total_borrow_times++;
            row->total_borrow_quantity += borrow->quantity;

            for (int r = 0; r < return_count; ++r)
            {
                if (returns[r].borrow_id == borrow->id)
                    row->total_return_quantity += returns[r].quantity;
            }
        }

        row->out_quantity = row->total_borrow_quantity - row->total_return_quantity;
    }

    return row_count;
}
