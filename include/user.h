#ifndef USER_H
#define USER_H

#include <stdbool.h>
#include "model.h"

bool verify_login(const char *username, const char *password, User *current_user);
bool register_user(const char *username, const char *password);
bool is_admin_user(const User *user);

#endif