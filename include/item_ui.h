#ifndef ITEM_UI_H
#define ITEM_UI_H

#include <stdbool.h>
#include "model.h"
#include "item.h"
#include "borrow.h"

bool item_manage_menu(Item items[], int *item_count,
                      const BorrowRecord borrows[], int borrow_count);

#endif // ITEM_UI_H
