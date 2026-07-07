#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "model.h"

bool read_line(char *buffer, int size);
bool read_int(const char *prompt, int *value);
bool is_valid_date(const char *date);
int safe_strcmp(const char *a, const char *b);
void clear_screen(void);
void print_items_preview(const Item items[], int count);
bool is_date_before_or_equal(const char *left, const char *right);
void stop(void);
char* get_current_date(void);

#endif // UTILS_H
