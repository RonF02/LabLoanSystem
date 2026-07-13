#ifndef REPORT_H
#define REPORT_H

#include "model.h"

typedef struct
{
    int borrow_id;
    char user[MAX_USER_LENGTH];
    char item_name[MAX_NAME_LENGTH];
    int borrow_quantity;
    char borrow_date[MAX_DATE_LENGTH];
    char due_date[MAX_DATE_LENGTH];
    char return_date[MAX_DATE_LENGTH];
    int return_quantity;
    BorrowStatus status;
} DetailReportRow;

typedef struct
{
    char item_code[MAX_CODE_LENGTH];
    char item_name[MAX_NAME_LENGTH];
    int total_borrow_times;
    int total_borrow_quantity;
    int total_return_quantity;
    int out_quantity;
} StatReportRow;

int build_detail_report(const Item items[], int item_count,
                        const BorrowRecord borrows[], int borrow_count,
                        const ReturnRecord returns[], int return_count,
                        DetailReportRow rows[], int row_max);

int build_stat_report(const Item items[], int item_count,
                      const BorrowRecord borrows[], int borrow_count,
                      const ReturnRecord returns[], int return_count,
                      StatReportRow rows[], int row_max);

#endif // REPORT_H
