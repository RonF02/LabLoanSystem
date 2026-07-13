#include "model.h"
#include "storage.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static const char *DATA_DIR = "data";
static const char *ITEMS_FILE = "data/items.txt";
static const char *BORROW_FILE = "data/borrow_records.txt";
static const char *RETURN_FILE = "data/return_records.txt";
static const char *USERS_FILE = "data/users.txt";

// 确保文件存在
static bool ensure_file_exists(const char *path)
{
    FILE *file = fopen(path, "a+"); 
    if (!file) 
    {
        return false; 
    }
    else
    {
        fclose(file);
        return true;
    }
}

static bool ensure_default_admin_user(void)
{
    FILE *file = fopen(USERS_FILE, "r");
    if (!file)
        return false;

    bool has_content = false;
    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t')
        {
            has_content = true;
            break;
        }
    }
    fclose(file);

    if (has_content)
        return true;

    file = fopen(USERS_FILE, "w");
    if (!file)
        return false;

    fprintf(file, "admin,123456,admin\n");
    fclose(file);
    return true;
}

// 保证数据目录存在
bool ensure_data_directory(void)
{
    struct stat st; // 定义一个 stat 结构体变量 st
    if (stat(DATA_DIR, &st) != 0) // 查看 DATA_DIR 指向的路径 !=0表示不成功
    {
        if (mkdir(DATA_DIR) != 0) // 如果目录不存在，就创建目录。创建失败则return false
        {
            return false;
        }
    }

    bool items_file_exists = ensure_file_exists(ITEMS_FILE);
    bool borrow_file_exists = ensure_file_exists(BORROW_FILE);
    bool return_file_exists = ensure_file_exists(RETURN_FILE);
    bool users_file_exists = ensure_file_exists(USERS_FILE);
    bool default_user_exists = users_file_exists && ensure_default_admin_user();

    return items_file_exists && borrow_file_exists && return_file_exists && default_user_exists;
}

// 解包item.txt下的每一行
static bool parse_item_line(const char *line, Item *item)
{
    int l = sscanf
    (
        line, 
        "%d,%31[^,],%63[^,],%63[^,],%d,%127[^,]", 
        &item->id, 
        item->code, 
        item->name, 
        item->model, 
        &item->quantity, 
        item->description
    );
    for (int i = 0; i < strlen(item->description); ++ i)
    {
        if (item->description[i] == '\n')
        {
            item->description[i] = '\0';
            break;
        }
    }
    return l == 6;
}

// 解包borrow_records.txt下的每一行
static bool parse_borrow_line(const char *line, BorrowRecord *record)
{
    int status;
    int l = sscanf
    (
        line,
        "%d,%63[^,],%31[^,],%d,%15[^,],%15[^,],%d\n",
        &record->id, 
        record->user, 
        record->item_code, 
        &record->quantity, 
        record->borrow_date, 
        record->due_date, 
        &status
    );

    if (l == 7) 
    {
        record->status = status;
        return true;
    }
    return false;
}

// 解包return_records.txt下的每一行
static bool parse_return_line(const char *line, ReturnRecord *record)
{
    int l = sscanf
    (
        line, 
        "%d,%d,%15[^,],%d", 
        &record->id, 
        &record->borrow_id, 
        record->return_date, 
        &record->quantity
    );
    return l == 4;
}

// 加载物品数据
bool load_items(Item items[], int *count)
{
    // 读取对应的文件
    FILE *file = fopen(ITEMS_FILE, "r"); 
    if (!file)
        return false;

    // 读取每一行数据
    char line[256];
    *count = 0; // 用于验证输入的物品数量是否已经到达上限
    while (fgets(line, sizeof(line), file) && *count < MAX_ITEMS) 
    {
        if (parse_item_line(line, &items[*count])) //检查是否能够成功解包
        {
            (*count)++;
        }
    }
    fclose(file);
    return true;
}

// 保存物品数据
bool save_items(const Item items[], int count)
{
    FILE *file = fopen(ITEMS_FILE, "w"); // 写入相应文件
    if (!file) 
    {
        return false;
    }

    for (int i = 0; i < count; ++i) {
        fprintf(
            file, "%d,%s,%s,%s,%d,%s\n",
            items[i].id,
            items[i].code,
            items[i].name,
            items[i].model,
            items[i].quantity,
            items[i].description
        );
    }
    fclose(file);
    return true;
}

// 加载借用数据
bool load_borrow_records(BorrowRecord borrows[], int *count)
{
    FILE *file = fopen(BORROW_FILE, "r");
    if (!file)
        return false;

    char line[256];
    *count = 0;
    while (fgets(line, sizeof(line), file) && *count < MAX_BORROW_RECORDS) 
    {
        if (parse_borrow_line(line, &borrows[*count])) 
        {
            (*count)++;
        }
    }
    fclose(file);
    return true;
}

// 保存借用数据
bool save_borrow_records(const BorrowRecord borrows[], int count)
{
    FILE *file = fopen(BORROW_FILE, "w");
    if (!file)
        return false;
    
    for (int i = 0; i < count; ++i) 
    {
        fprintf(
            file, "%d,%s,%s,%d,%s,%s,%d\n",
            borrows[i].id,
            borrows[i].user,
            borrows[i].item_code,
            borrows[i].quantity,
            borrows[i].borrow_date,
            borrows[i].due_date,
            (int)borrows[i].status
        );
    }
    fclose(file);
    return true;
}

// 读取归还数据
bool load_return_records(ReturnRecord returns[], int *count)
{
    FILE *file = fopen(RETURN_FILE, "r");
    if (!file)
        return false;
        
    char line[256];
    *count = 0;
    while (fgets(line, sizeof(line), file) && *count < MAX_RETURN_RECORDS) 
    {
        if (parse_return_line(line, &returns[*count])) 
        {
            (*count)++;
        }
    }
    fclose(file);
    return true;
}

// 保存归还数据
bool save_return_records(const ReturnRecord returns[], int count)
{
    FILE *file = fopen(RETURN_FILE, "w");
    if (!file)
        return false;

    for (int i = 0; i < count; ++i) 
    {
        fprintf
        (
            file, "%d,%d,%s,%d\n",
            returns[i].id,
            returns[i].borrow_id,
            returns[i].return_date,
            returns[i].quantity
        );
    }
    fclose(file);
    return true;
}


bool load_users(User users[], int *count)
{
    FILE *file = fopen(USERS_FILE, "r");
    if (!file)
        return false;

    char line[256];
    *count = 0;
    while (fgets(line, sizeof(line), file) && *count < MAX_USER) 
    {
        int fields = sscanf(
            line,
            "%63[^,],%63[^,],%15[^\n]",
            users[*count].username,
            users[*count].password,
            users[*count].level
        );

        if (fields != 3)
        {
            fields = sscanf(
                line,
                "%63[^,],%63[^\n]",
                users[*count].username,
                users[*count].password
            );
            if (fields == 2)
            {
                strncpy(users[*count].level, "guest", MAX_LEVEL_LENGTH - 1);
                users[*count].level[MAX_LEVEL_LENGTH - 1] = '\0';
            }
        }

        if (fields == 2 || fields == 3) 
        {
            (*count)++;
        }
    }
    fclose(file);
    return true;
}

bool save_users(const User users[], int count)
{
    FILE *file = fopen(USERS_FILE, "w");
    if (!file)
        return false;

    for (int i = 0; i < count; ++i) 
    {
        fprintf(file, "%s,%s,%s\n", users[i].username, users[i].password, users[i].level);
    }
    fclose(file);
    return true;
}