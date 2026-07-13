#include "sync.h"
#include "storage.h"

bool sync_items(const Item items[], int item_count)
{
    return save_items(items, item_count);
}

bool sync_borrows(const BorrowRecord borrows[], int borrow_count)
{
    return save_borrow_records(borrows, borrow_count);
}

bool sync_returns(const ReturnRecord returns[], int return_count)
{
    return save_return_records(returns, return_count);
}

bool sync_data(const Item items[], int item_count,
               const BorrowRecord borrows[], int borrow_count,
               const ReturnRecord returns[], int return_count)
{
    return sync_items(items, item_count) &&
           sync_borrows(borrows, borrow_count) &&
           sync_returns(returns, return_count);
}
