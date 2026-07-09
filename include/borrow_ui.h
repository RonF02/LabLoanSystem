#ifndef BORROW_UI_H
#define BORROW_UI_H

#include <stdbool.h>
#include "model.h"
#include "borrow.h"
#include "item.h"

bool borrow_manage_menu
(
    BorrowRecord borrows[], 
    int *borrow_count,           
    ReturnRecord returns[], 
    int *return_count,
    Item items[], int item_count,
    const char *current_username
);

#endif // BORROW_UI_H
