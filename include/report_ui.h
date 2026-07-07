#ifndef REPORT_UI_H
#define REPORT_UI_H

#include <stdbool.h>
#include "model.h"

bool report_manage_menu(Item items[], int item_count,
                        BorrowRecord borrows[], int borrow_count,
                        ReturnRecord returns[], int return_count);

#endif // REPORT_UI_H
