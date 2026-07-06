#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

bool read_line(char *buffer, int size);
bool is_valid_date(const char *date);
int safe_strcmp(const char *a, const char *b);

#endif // UTILS_H
