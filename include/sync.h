#ifndef SYNC_H
#define SYNC_H

#include <stdbool.h>

#include "model.h"

bool sync_items(const Item items[], int item_count);
bool sync_borrows(const BorrowRecord borrows[], int borrow_count);
bool sync_returns(const ReturnRecord returns[], int return_count);

bool sync_data(const Item items[], int item_count,
               const BorrowRecord borrows[], int borrow_count,
               const ReturnRecord returns[], int return_count);

#endif // SYNC_H
