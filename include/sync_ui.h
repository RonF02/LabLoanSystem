#ifndef SYNC_UI_H
#define SYNC_UI_H

#include "model.h"

void sync_data_menu(const Item items[], int item_count,
                    const BorrowRecord borrows[], int borrow_count,
                    const ReturnRecord returns[], int return_count);

#endif // SYNC_UI_H
