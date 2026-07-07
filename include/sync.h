#ifndef SYNC_H
#define SYNC_H

#include "model.h"

void sync_data(const Item items[], int item_count,
               const BorrowRecord borrows[], int borrow_count,
               const ReturnRecord returns[], int return_count);

#endif // SYNC_H
