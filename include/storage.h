#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include "model.h"

bool ensure_data_directory(void);
bool load_items(Item items[], int *count);
bool save_items(const Item items[], int count);
bool load_borrow_records(BorrowRecord borrows[], int *count);
bool save_borrow_records(const BorrowRecord borrows[], int count);
bool load_return_records(ReturnRecord returns[], int *count);
bool save_return_records(const ReturnRecord returns[], int count);

#endif // STORAGE_H
