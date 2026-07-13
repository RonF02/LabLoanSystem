#ifndef QUERY_H
#define QUERY_H

#include "model.h"

typedef struct
{
    int borrow_id;
    char user[MAX_USER_LENGTH];
    char item_code[MAX_CODE_LENGTH];
    char item_name[MAX_NAME_LENGTH];
    int quantity;
    char borrow_date[MAX_DATE_LENGTH];
    char due_date[MAX_DATE_LENGTH];
    BorrowStatus status;
} BorrowQueryRow;

int query_borrow_records(const Item items[], int item_count,
                         const BorrowRecord borrows[], int borrow_count,
                         const char *keyword,
                         BorrowQueryRow rows[], int row_max);

#endif
