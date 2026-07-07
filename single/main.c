#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define MAX_ITEMS 256
#define MAX_BORROW_RECORDS 256
#define MAX_RETURN_RECORDS 256
#define MAX_CODE_LENGTH 32
#define MAX_NAME_LENGTH 64
#define MAX_MODEL_LENGTH 64
#define MAX_DESC_LENGTH 128
#define MAX_USER_LENGTH 64
#define MAX_DATE_LENGTH 16

typedef struct {
    int id;
    char code[MAX_CODE_LENGTH];
    char name[MAX_NAME_LENGTH];
    char model[MAX_MODEL_LENGTH];
    int quantity;
    char description[MAX_DESC_LENGTH];
} Item;

typedef enum {
    BORROW_ACTIVE = 0,
    BORROW_RETURNED = 1
} BorrowStatus;

typedef struct {
    int id;
    char user[MAX_USER_LENGTH];
    char item_code[MAX_CODE_LENGTH];
    int quantity;
    char borrow_date[MAX_DATE_LENGTH];
    char due_date[MAX_DATE_LENGTH];
    BorrowStatus status;
} BorrowRecord;

typedef struct {
    int id;
    int borrow_id;
    char return_date[MAX_DATE_LENGTH];
    int quantity;
} ReturnRecord;

static const char *DATA_DIR = "data";
static const char *ITEMS_FILE = "data/items.txt";
static const char *BORROW_FILE = "data/borrow_records.txt";
static const char *RETURN_FILE = "data/return_records.txt";

static void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void print_items_preview(const Item items[], int count) {
    puts("=== 当前物品库存 ===");
    if (count <= 0) {
        puts("暂无物品记录。\n");
        return;
    }
    puts("ID | 编号 | 名称 | 型号 | 库存 | 描述");
    puts("----+------+------+------+------+------------------------");
    for (int i = 0; i < count; ++i) {
        printf("%-2d | %-8s | %-8s | %-8s | %-4d | %s\n",
               items[i].id, items[i].code, items[i].name, items[i].model,
               items[i].quantity, items[i].description);
    }
    puts("------------------------");
}

static bool ensure_file_exists(const char *path) {
    FILE *file = fopen(path, "a+");
    if (!file) return false;
    fclose(file);
    return true;
}

static bool ensure_data_directory(void) {
    struct stat st;
    if (stat(DATA_DIR, &st) != 0) {
        if (mkdir(DATA_DIR) != 0) return false;
    }
    return ensure_file_exists(ITEMS_FILE) && ensure_file_exists(BORROW_FILE) && ensure_file_exists(RETURN_FILE);
}

static bool read_line(char *buffer, int size) {
    if (!fgets(buffer, size, stdin)) return false;
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
    return true;
}

static bool read_int(const char *prompt, int *value) {
    char buffer[32];
    char *endptr = NULL;
    long result;
    printf("%s", prompt);
    if (!read_line(buffer, sizeof(buffer))) return false;
    result = strtol(buffer, &endptr, 10);
    if (endptr == buffer || *endptr != '\0') return false;
    *value = (int)result;
    return true;
}

static bool is_valid_date(const char *date) {
    if (!date || strlen(date) != 10) return false;
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') return false;
        } else if (!isdigit((unsigned char)date[i])) {
            return false;
        }
    }
    int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + (date[3] - '0');
    int month = (date[5] - '0') * 10 + (date[6] - '0');
    int day = (date[8] - '0') * 10 + (date[9] - '0');
    if (year < 1900 || month < 1 || month > 12 || day < 1) return false;
    int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) mdays[1] = 29;
    return day <= mdays[month - 1];
}

static int item_next_id(const Item items[], int count) {
    int max_id = 0;
    for (int i = 0; i < count; ++i) {
        if (items[i].id > max_id) max_id = items[i].id;
    }
    return max_id + 1;
}

static bool item_generate_code(const Item items[], int count, char *code, size_t size) {
    if (!code || size == 0) return false;
    int next_id = item_next_id(items, count);
    int written = snprintf(code, size, "ITEM-%03d", next_id);
    return written > 0 && (size_t)written < size;
}

static int item_find_by_code(const Item items[], int count, const char *code) {
    if (!code) return -1;
    for (int i = 0; i < count; ++i) {
        if (strcmp(items[i].code, code) == 0) return i;
    }
    return -1;
}

static int item_find_by_name(const Item items[], int count, const char *name, int result_indexes[], int result_max) {
    if (!name || !result_indexes || result_max <= 0) return 0;
    int matched = 0;
    for (int i = 0; i < count && matched < result_max; ++i) {
        if (strstr(items[i].name, name) != NULL) {
            result_indexes[matched++] = i;
        }
    }
    return matched;
}

static bool item_add(Item items[], int *count, const char *code, const char *name, const char *model, int quantity, const char *description) {
    if (!count || !name || !model || !description) return false;
    if (*count >= MAX_ITEMS || quantity < 0) return false;
    if (strlen(name) == 0 || strlen(model) == 0) return false;
    if (!code || strlen(code) == 0) return false;
    if (item_find_by_code(items, *count, code) >= 0) return false;
    Item *new_item = &items[*count];
    new_item->id = item_next_id(items, *count);
    strncpy(new_item->code, code, MAX_CODE_LENGTH - 1);
    new_item->code[MAX_CODE_LENGTH - 1] = '\0';
    strncpy(new_item->name, name, MAX_NAME_LENGTH - 1);
    new_item->name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(new_item->model, model, MAX_MODEL_LENGTH - 1);
    new_item->model[MAX_MODEL_LENGTH - 1] = '\0';
    new_item->quantity = quantity;
    strncpy(new_item->description, description, MAX_DESC_LENGTH - 1);
    new_item->description[MAX_DESC_LENGTH - 1] = '\0';
    (*count)++;
    return true;
}

static bool item_update(Item items[], int count, const char *code, const char *name, const char *model, int quantity, const char *description) {
    if (!code || !name || !model || !description || quantity < 0) return false;
    int idx = item_find_by_code(items, count, code);
    if (idx < 0) return false;
    strncpy(items[idx].name, name, MAX_NAME_LENGTH - 1);
    items[idx].name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(items[idx].model, model, MAX_MODEL_LENGTH - 1);
    items[idx].model[MAX_MODEL_LENGTH - 1] = '\0';
    items[idx].quantity = quantity;
    strncpy(items[idx].description, description, MAX_DESC_LENGTH - 1);
    items[idx].description[MAX_DESC_LENGTH - 1] = '\0';
    return true;
}

static bool item_remove(Item items[], int *count, const BorrowRecord borrows[], int borrow_count, const char *code) {
    if (!count || !code) return false;
    int idx = item_find_by_code(items, *count, code);
    if (idx < 0) return false;
    for (int i = 0; i < borrow_count; ++i) {
        if (strcmp(borrows[i].item_code, code) == 0 && borrows[i].status == BORROW_ACTIVE) return false;
    }
    for (int i = idx; i < *count - 1; ++i) items[i] = items[i + 1];
    (*count)--;
    return true;
}

static int borrow_next_id(const BorrowRecord borrows[], int count) {
    int max_id = 0;
    for (int i = 0; i < count; ++i) if (borrows[i].id > max_id) max_id = borrows[i].id;
    return max_id + 1;
}

static int return_next_id(const ReturnRecord returns[], int count) {
    int max_id = 0;
    for (int i = 0; i < count; ++i) if (returns[i].id > max_id) max_id = returns[i].id;
    return max_id + 1;
}

static int borrow_find_by_id(const BorrowRecord borrows[], int count, int id) {
    for (int i = 0; i < count; ++i) if (borrows[i].id == id) return i;
    return -1;
}

static int borrow_returned_quantity(const ReturnRecord returns[], int return_count, int borrow_id) {
    int total = 0;
    for (int i = 0; i < return_count; ++i) if (returns[i].borrow_id == borrow_id) total += returns[i].quantity;
    return total;
}

static bool borrow_register(BorrowRecord borrows[], int *borrow_count, Item items[], int item_count, const char *user, const char *item_code, int quantity, const char *borrow_date, const char *due_date) {
    if (!borrow_count || !user || !item_code || !borrow_date || !due_date) return false;
    if (*borrow_count >= MAX_BORROW_RECORDS || quantity <= 0) return false;
    if (strlen(user) == 0 || strlen(item_code) == 0 || strlen(borrow_date) == 0 || strlen(due_date) == 0) return false;
    if (!is_valid_date(borrow_date) || !is_valid_date(due_date)) return false;
    int idx = item_find_by_code(items, item_count, item_code);
    if (idx < 0) return false;
    if (items[idx].quantity < quantity) return false;
    items[idx].quantity -= quantity;
    BorrowRecord *record = &borrows[*borrow_count];
    record->id = borrow_next_id(borrows, *borrow_count);
    strncpy(record->user, user, MAX_USER_LENGTH - 1);
    record->user[MAX_USER_LENGTH - 1] = '\0';
    strncpy(record->item_code, item_code, MAX_CODE_LENGTH - 1);
    record->item_code[MAX_CODE_LENGTH - 1] = '\0';
    record->quantity = quantity;
    strncpy(record->borrow_date, borrow_date, MAX_DATE_LENGTH - 1);
    record->borrow_date[MAX_DATE_LENGTH - 1] = '\0';
    strncpy(record->due_date, due_date, MAX_DATE_LENGTH - 1);
    record->due_date[MAX_DATE_LENGTH - 1] = '\0';
    record->status = BORROW_ACTIVE;
    (*borrow_count)++;
    return true;
}

static bool borrow_return(BorrowRecord borrows[], int borrow_count, ReturnRecord returns[], int *return_count, Item items[], int item_count, int borrow_id, int quantity, const char *return_date) {
    if (!return_count || !return_date) return false;
    if (*return_count >= MAX_RETURN_RECORDS || quantity <= 0) return false;
    if (!is_valid_date(return_date)) return false;
    int borrow_idx = borrow_find_by_id(borrows, borrow_count, borrow_id);
    if (borrow_idx < 0 || borrows[borrow_idx].status == BORROW_RETURNED) return false;
    int already = borrow_returned_quantity(returns, *return_count, borrow_id);
    if (already + quantity > borrows[borrow_idx].quantity) return false;
    int item_idx = item_find_by_code(items, item_count, borrows[borrow_idx].item_code);
    if (item_idx < 0) return false;
    items[item_idx].quantity += quantity;
    ReturnRecord *ret = &returns[*return_count];
    ret->id = return_next_id(returns, *return_count);
    ret->borrow_id = borrow_id;
    strncpy(ret->return_date, return_date, MAX_DATE_LENGTH - 1);
    ret->return_date[MAX_DATE_LENGTH - 1] = '\0';
    ret->quantity = quantity;
    (*return_count)++;
    if (already + quantity >= borrows[borrow_idx].quantity) borrows[borrow_idx].status = BORROW_RETURNED;
    return true;
}

static bool load_items(Item items[], int *count) {
    FILE *file = fopen(ITEMS_FILE, "r");
    if (!file) return false;
    char line[256];
    *count = 0;
    while (fgets(line, sizeof(line), file) && *count < MAX_ITEMS) {
        int id, quantity;
        char code[MAX_CODE_LENGTH], name[MAX_NAME_LENGTH], model[MAX_MODEL_LENGTH], description[MAX_DESC_LENGTH];
        if (sscanf(line, "%d,%31[^,],%63[^,],%63[^,],%d,%127[^,\n]", &id, code, name, model, &quantity, description) == 6) {
            Item *item = &items[*count];
            item->id = id;
            strncpy(item->code, code, MAX_CODE_LENGTH - 1);
            item->code[MAX_CODE_LENGTH - 1] = '\0';
            strncpy(item->name, name, MAX_NAME_LENGTH - 1);
            item->name[MAX_NAME_LENGTH - 1] = '\0';
            strncpy(item->model, model, MAX_MODEL_LENGTH - 1);
            item->model[MAX_MODEL_LENGTH - 1] = '\0';
            item->quantity = quantity;
            strncpy(item->description, description, MAX_DESC_LENGTH - 1);
            item->description[MAX_DESC_LENGTH - 1] = '\0';
            (*count)++;
        }
    }
    fclose(file);
    return true;
}

static bool save_items(const Item items[], int count) {
    FILE *file = fopen(ITEMS_FILE, "w");
    if (!file) return false;
    for (int i = 0; i < count; ++i) {
        fprintf(file, "%d,%s,%s,%s,%d,%s\n", items[i].id, items[i].code, items[i].name, items[i].model, items[i].quantity, items[i].description);
    }
    fclose(file);
    return true;
}

static bool load_borrow_records(BorrowRecord borrows[], int *count) {
    FILE *file = fopen(BORROW_FILE, "r");
    if (!file) return false;
    char line[256];
    *count = 0;
    while (fgets(line, sizeof(line), file) && *count < MAX_BORROW_RECORDS) {
        int id, quantity, status;
        char user[MAX_USER_LENGTH], item_code[MAX_CODE_LENGTH], borrow_date[MAX_DATE_LENGTH], due_date[MAX_DATE_LENGTH];
        if (sscanf(line, "%d,%63[^,],%31[^,],%d,%15[^,],%15[^,],%d", &id, user, item_code, &quantity, borrow_date, due_date, &status) == 7) {
            BorrowRecord *record = &borrows[*count];
            record->id = id;
            strncpy(record->user, user, MAX_USER_LENGTH - 1);
            record->user[MAX_USER_LENGTH - 1] = '\0';
            strncpy(record->item_code, item_code, MAX_CODE_LENGTH - 1);
            record->item_code[MAX_CODE_LENGTH - 1] = '\0';
            record->quantity = quantity;
            strncpy(record->borrow_date, borrow_date, MAX_DATE_LENGTH - 1);
            record->borrow_date[MAX_DATE_LENGTH - 1] = '\0';
            strncpy(record->due_date, due_date, MAX_DATE_LENGTH - 1);
            record->due_date[MAX_DATE_LENGTH - 1] = '\0';
            record->status = (BorrowStatus)status;
            (*count)++;
        }
    }
    fclose(file);
    return true;
}

static bool save_borrow_records(const BorrowRecord borrows[], int count) {
    FILE *file = fopen(BORROW_FILE, "w");
    if (!file) return false;
    for (int i = 0; i < count; ++i) {
        fprintf(file, "%d,%s,%s,%d,%s,%s,%d\n", borrows[i].id, borrows[i].user, borrows[i].item_code, borrows[i].quantity, borrows[i].borrow_date, borrows[i].due_date, (int)borrows[i].status);
    }
    fclose(file);
    return true;
}

static bool load_return_records(ReturnRecord returns[], int *count) {
    FILE *file = fopen(RETURN_FILE, "r");
    if (!file) return false;
    char line[256];
    *count = 0;
    while (fgets(line, sizeof(line), file) && *count < MAX_RETURN_RECORDS) {
        int id, borrow_id, quantity;
        char return_date[MAX_DATE_LENGTH];
        if (sscanf(line, "%d,%d,%15[^,],%d", &id, &borrow_id, return_date, &quantity) == 4) {
            ReturnRecord *record = &returns[*count];
            record->id = id;
            record->borrow_id = borrow_id;
            strncpy(record->return_date, return_date, MAX_DATE_LENGTH - 1);
            record->return_date[MAX_DATE_LENGTH - 1] = '\0';
            record->quantity = quantity;
            (*count)++;
        }
    }
    fclose(file);
    return true;
}

static bool save_return_records(const ReturnRecord returns[], int count) {
    FILE *file = fopen(RETURN_FILE, "w");
    if (!file) return false;
    for (int i = 0; i < count; ++i) {
        fprintf(file, "%d,%d,%s,%d\n", returns[i].id, returns[i].borrow_id, returns[i].return_date, returns[i].quantity);
    }
    fclose(file);
    return true;
}

static void print_detail_report(const Item items[], int item_count, const BorrowRecord borrows[], int borrow_count, const ReturnRecord returns[], int return_count) {
    puts("=== 出入账明细报表 ===");
    for (int i = 0; i < borrow_count; ++i) {
        printf("借用记录 ID:%d 用户:%s 物品:%s 数量:%d 日期:%s 预计归还:%s 状态:%s\n",
               borrows[i].id, borrows[i].user, borrows[i].item_code, borrows[i].quantity,
               borrows[i].borrow_date, borrows[i].due_date,
               borrows[i].status == BORROW_ACTIVE ? "借出" : "已归还");
    }
    for (int i = 0; i < return_count; ++i) {
        printf("归还记录 ID:%d 借用ID:%d 日期:%s 数量:%d\n",
               returns[i].id, returns[i].borrow_id, returns[i].return_date, returns[i].quantity);
    }
    puts("");
}

static void print_stat_report(const Item items[], int item_count, const BorrowRecord borrows[], int borrow_count, const ReturnRecord returns[], int return_count) {
    puts("=== 借用统计汇总报表 ===");
    int active = 0;
    int returned = 0;
    for (int i = 0; i < borrow_count; ++i) {
        if (borrows[i].status == BORROW_ACTIVE) active++; else returned++;
    }
    printf("总借用记录:%d\n", borrow_count);
    printf("未归还:%d\n", active);
    printf("已归还:%d\n", returned);
    printf("当前库存物品数:%d\n", item_count);
    puts("");
}

static void show_main_menu(const Item items[], int item_count) {
    clear_screen();
    puts("=== 实验室物品出入账管理系统 ===");
    print_items_preview(items, item_count);
    puts("1. 物品管理");
    puts("2. 借用管理");
    puts("3. 报表查询");
    puts("4. 数据同步");
    puts("0. 退出");
    printf("请选择：");
}

static void handle_item_menu(Item items[], int *item_count, const BorrowRecord borrows[], int borrow_count) {
    char choice[16];
    while (1) {
        clear_screen();
        print_items_preview(items, *item_count);
        puts("\n=== 物品管理 ===");
        puts("1. 新增物品");
        puts("2. 修改物品");
        puts("3. 删除物品");
        puts("4. 查询物品");
        puts("0. 返回主菜单");
        printf("请选择：");
        if (!read_line(choice, sizeof(choice))) return;
        if (strcmp(choice, "1") == 0) {
            char code[MAX_CODE_LENGTH];
            char name[MAX_NAME_LENGTH];
            char model[MAX_MODEL_LENGTH];
            char description[MAX_DESC_LENGTH];
            int quantity;
            if (!item_generate_code(items, *item_count, code, sizeof(code))) { puts("生成编号失败。\n"); continue; }
            printf("请输入物品名称：");
            if (!read_line(name, sizeof(name)) || strlen(name) == 0) { puts("名称不能为空。\n"); continue; }
            printf("请输入物品型号：");
            if (!read_line(model, sizeof(model)) || strlen(model) == 0) { puts("型号不能为空。\n"); continue; }
            if (!read_int("请输入库存数量：", &quantity) || quantity < 0) { puts("数量无效。\n"); continue; }
            printf("请输入物品描述：");
            if (!read_line(description, sizeof(description))) { puts("描述读取失败。\n"); continue; }
            if (item_add(items, item_count, code, name, model, quantity, description)) {
                printf("新增成功，编号：%s\n\n", code);
            } else {
                puts("新增失败。\n");
            }
        } else if (strcmp(choice, "2") == 0) {
            char code[MAX_CODE_LENGTH];
            char name[MAX_NAME_LENGTH];
            char model[MAX_MODEL_LENGTH];
            char description[MAX_DESC_LENGTH];
            int quantity;
            printf("请输入物品编号：");
            if (!read_line(code, sizeof(code)) || strlen(code) == 0) { puts("编号不能为空。\n"); continue; }
            printf("请输入新名称：");
            if (!read_line(name, sizeof(name)) || strlen(name) == 0) { puts("名称不能为空。\n"); continue; }
            printf("请输入新型号：");
            if (!read_line(model, sizeof(model)) || strlen(model) == 0) { puts("型号不能为空。\n"); continue; }
            if (!read_int("请输入新库存数量：", &quantity) || quantity < 0) { puts("数量无效。\n"); continue; }
            printf("请输入新描述：");
            if (!read_line(description, sizeof(description))) { puts("描述读取失败。\n"); continue; }
            if (item_update(items, *item_count, code, name, model, quantity, description)) puts("修改成功。\n"); else puts("修改失败。\n");
        } else if (strcmp(choice, "3") == 0) {
            char code[MAX_CODE_LENGTH];
            printf("请输入物品编号：");
            if (!read_line(code, sizeof(code)) || strlen(code) == 0) { puts("编号不能为空。\n"); continue; }
            if (item_remove(items, item_count, borrows, borrow_count, code)) puts("删除成功。\n"); else puts("删除失败。\n");
        } else if (strcmp(choice, "4") == 0) {
            char query[MAX_NAME_LENGTH];
            int result_indexes[MAX_ITEMS];
            printf("请输入编号或名称关键字：");
            if (!read_line(query, sizeof(query)) || strlen(query) == 0) { puts("关键字不能为空。\n"); continue; }
            int idx = item_find_by_code(items, *item_count, query);
            if (idx >= 0) {
                printf("ID:%d 编号:%s 名称:%s 型号:%s 库存:%d 描述:%s\n\n", items[idx].id, items[idx].code, items[idx].name, items[idx].model, items[idx].quantity, items[idx].description);
            } else {
                int found = item_find_by_name(items, *item_count, query, result_indexes, MAX_ITEMS);
                if (found > 0) {
                    for (int i = 0; i < found; ++i) {
                        printf("ID:%d 编号:%s 名称:%s 型号:%s 库存:%d 描述:%s\n", items[result_indexes[i]].id, items[result_indexes[i]].code, items[result_indexes[i]].name, items[result_indexes[i]].model, items[result_indexes[i]].quantity, items[result_indexes[i]].description);
                    }
                    puts("");
                } else {
                    puts("未找到匹配物品。\n");
                }
            }
        } else if (strcmp(choice, "0") == 0) {
            return;
        } else {
            puts("无效选择。\n");
        }
    }
}

static void handle_borrow_menu(BorrowRecord borrows[], int *borrow_count, ReturnRecord returns[], int *return_count, Item items[], int item_count) {
    char choice[16];
    while (1) {
        clear_screen();
        print_items_preview(items, item_count);
        puts("\n=== 借用管理 ===");
        puts("1. 借用登记");
        puts("2. 归还登记");
        puts("0. 返回主菜单");
        printf("请选择：");
        if (!read_line(choice, sizeof(choice))) return;
        if (strcmp(choice, "1") == 0) {
            char user[MAX_USER_LENGTH];
            char item_code[MAX_CODE_LENGTH];
            char due_date[MAX_DATE_LENGTH];
            char borrow_date[MAX_DATE_LENGTH];
            int quantity;
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            printf("请输入借用人：");
            if (!read_line(user, sizeof(user)) || strlen(user) == 0) { puts("借用人不能为空。\n"); continue; }
            printf("请输入物品编号：");
            if (!read_line(item_code, sizeof(item_code)) || strlen(item_code) == 0) { puts("物品编号不能为空。\n"); continue; }
            if (!read_int("请输入借用数量：", &quantity) || quantity <= 0) { puts("数量无效。\n"); continue; }
            printf("请输入预计归还日期 (YYYY-MM-DD)：");
            if (!read_line(due_date, sizeof(due_date)) || !is_valid_date(due_date)) { puts("日期无效。\n"); continue; }
            if (!local) { puts("获取时间失败。\n"); continue; }
            snprintf(borrow_date, sizeof(borrow_date), "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
            if (borrow_register(borrows, borrow_count, items, item_count, user, item_code, quantity, borrow_date, due_date)) {
                printf("借用登记成功，ID:%d\n\n", borrow_next_id(borrows, *borrow_count) - 1);
            } else {
                puts("借用登记失败。\n");
            }
        } else if (strcmp(choice, "2") == 0) {
            int borrow_id;
            int quantity;
            char return_date[MAX_DATE_LENGTH];
            if (!read_int("请输入借用记录 ID：", &borrow_id) || borrow_id <= 0) { puts("ID无效。\n"); continue; }
            if (!read_int("请输入归还数量：", &quantity) || quantity <= 0) { puts("数量无效。\n"); continue; }
            printf("请输入归还日期 (YYYY-MM-DD)：");
            if (!read_line(return_date, sizeof(return_date)) || !is_valid_date(return_date)) { puts("日期无效。\n"); continue; }
            if (borrow_return(borrows, *borrow_count, returns, return_count, items, item_count, borrow_id, quantity, return_date)) puts("归还登记成功。\n"); else puts("归还登记失败。\n");
        } else if (strcmp(choice, "0") == 0) {
            return;
        } else {
            puts("无效选择。\n");
        }
    }
}

static void handle_report_menu(Item items[], int item_count, BorrowRecord borrows[], int borrow_count, ReturnRecord returns[], int return_count) {
    char choice[16];
    while (1) {
        clear_screen();
        print_items_preview(items, item_count);
        puts("\n=== 报表查询 ===");
        puts("1. 出入账明细报表");
        puts("2. 借用统计汇总报表");
        puts("0. 返回主菜单");
        printf("请选择：");
        if (!read_line(choice, sizeof(choice))) return;
        if (strcmp(choice, "1") == 0) {
            print_detail_report(items, item_count, borrows, borrow_count, returns, return_count);
            puts("请输入 0 返回主菜单。\n");
            if (!read_line(choice, sizeof(choice))) return;
            if (strcmp(choice, "0") == 0) continue;
        } else if (strcmp(choice, "2") == 0) {
            print_stat_report(items, item_count, borrows, borrow_count, returns, return_count);
            puts("请输入 0 返回主菜单。\n");
            if (!read_line(choice, sizeof(choice))) return;
            if (strcmp(choice, "0") == 0) continue;
        } else if (strcmp(choice, "0") == 0) {
            return;
        } else {
            puts("无效选择。\n");
        }
    }
}

static void handle_sync_menu(const Item items[], int item_count, const BorrowRecord borrows[], int borrow_count, const ReturnRecord returns[], int return_count) {
    char choice[16];
    while (1) {
        clear_screen();
        print_items_preview(items, item_count);
        puts("\n=== 数据同步 ===");
        puts("0. 返回主菜单");
        printf("请选择：");
        if (!read_line(choice, sizeof(choice))) return;
        if (strcmp(choice, "0") == 0) return;
        puts("请输入 0 返回主菜单。\n");
    }
}

int main(void) {
    Item items[MAX_ITEMS];
    BorrowRecord borrows[MAX_BORROW_RECORDS];
    ReturnRecord returns[MAX_RETURN_RECORDS];
    int item_count = 0;
    int borrow_count = 0;
    int return_count = 0;
    char choice[16];

    ensure_data_directory();
    load_items(items, &item_count);
    load_borrow_records(borrows, &borrow_count);
    load_return_records(returns, &return_count);

    while (1) {
        show_main_menu(items, item_count);
        if (!read_line(choice, sizeof(choice))) break;
        if (strcmp(choice, "0") == 0) break;
        else if (strcmp(choice, "1") == 0) handle_item_menu(items, &item_count, borrows, borrow_count);
        else if (strcmp(choice, "2") == 0) handle_borrow_menu(borrows, &borrow_count, returns, &return_count, items, item_count);
        else if (strcmp(choice, "3") == 0) handle_report_menu(items, item_count, borrows, borrow_count, returns, return_count);
        else if (strcmp(choice, "4") == 0) handle_sync_menu(items, item_count, borrows, borrow_count, returns, return_count);
        else puts("无效选择。\n");
    }

    save_items(items, item_count);
    save_borrow_records(borrows, borrow_count);
    save_return_records(returns, return_count);
    puts("系统已退出。\n");
    return 0;
}
