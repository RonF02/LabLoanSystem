#ifndef MODEL_H
#define MODEL_H

#include <stdbool.h>

#define MAX_ITEMS 256
#define MAX_BORROW_RECORDS 256
#define MAX_RETURN_RECORDS 256

#define MAX_CODE_LENGTH 32
#define MAX_NAME_LENGTH 64
#define MAX_MODEL_LENGTH 64
#define MAX_DESC_LENGTH 128
#define MAX_USER_LENGTH 64
#define MAX_DATE_LENGTH 16

#define MAX_USER 256
#define MAX_USERNAME_LENGTH 64
#define MAX_PASSWORD_LENGTH 64
#define MAX_LEVEL_LENGTH 16

typedef struct 
{
    int id;
    char code[MAX_CODE_LENGTH];
    char name[MAX_NAME_LENGTH];
    char model[MAX_MODEL_LENGTH];
    int quantity;
    char description[MAX_DESC_LENGTH];
} Item;

typedef enum 
{
    BORROW_ACTIVE = 0,
    BORROW_RETURNED = 1
} BorrowStatus;

typedef struct 
{
    int id;
    char user[MAX_USER_LENGTH];
    char item_code[MAX_CODE_LENGTH];
    int quantity;
    char borrow_date[MAX_DATE_LENGTH];
    char due_date[MAX_DATE_LENGTH];
    BorrowStatus status;
} BorrowRecord;

typedef struct 
{
    int id;
    int borrow_id;
    char return_date[MAX_DATE_LENGTH];
    int quantity;
} ReturnRecord;

typedef struct 
{
    char username[MAX_USER_LENGTH];
    char password[MAX_USER_LENGTH];
    char level[MAX_LEVEL_LENGTH];
} User;

#endif // MODEL_H
