#include "query.h"
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
    return "未知物品";
}

int query_borrow_records(const Item items[], int item_count,
                         const BorrowRecord borrows[], int borrow_count,
                         const char *keyword,
                         BorrowQueryRow rows[], int row_max)
{
    if (!rows || row_max <= 0)
        return 0;

    int row_count = 0;
    for (int i = 0; i < borrow_count && row_count < row_max; ++i)
    {
        const BorrowRecord *borrow = &borrows[i];
        const char *item_name = find_item_name(items, item_count, borrow->item_code);
        int match = keyword == NULL || safe_strcmp(keyword, "") == 0;

        if (!match)
        {
            match = strstr(item_name, keyword) != NULL ||
                    strstr(borrow->item_code, keyword) != NULL ||
                    strstr(borrow->user, keyword) != NULL;
        }

        if (!match)
            continue;

        BorrowQueryRow *row = &rows[row_count++];
        row->borrow_id = borrow->id;
        copy_string(row->user, sizeof(row->user), borrow->user);
        copy_string(row->item_code, sizeof(row->item_code), borrow->item_code);
        copy_string(row->item_name, sizeof(row->item_name), item_name);
        row->quantity = borrow->quantity;
        copy_string(row->borrow_date, sizeof(row->borrow_date), borrow->borrow_date);
        copy_string(row->due_date, sizeof(row->due_date), borrow->due_date);
        row->status = borrow->status;
    }

    return row_count;
}
