#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model.h"
#include "storage.h"

bool verify_login(const char *username, const char *password, User *current_user)
{
    User users[MAX_USER];
    int user_count = 0;

    if (!load_users(users, &user_count)) 
    {
        fprintf(stderr, "警告：读取用户数据失败，系统将以空数据启动。\n");
    }

    for (int i = 0; i < user_count; ++i) 
    {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) 
        {
            *current_user = users[i];
            return true;
        }
    }

    return false;
}

bool is_admin_user(const User *user)
{
    return strcmp(user->level, "admin") == 0;
}

bool register_user(const char *username, const char *password) 
{
    User users[MAX_USER];
    int user_count = 0;

    if (!load_users(users, &user_count)) 
    {
        fprintf(stderr, "警告：读取用户数据失败，系统将以空数据启动。\n");
    }

    // 检查用户名是否已存在
    for (int i = 0; i < user_count; ++i) 
    {
        if (strcmp(users[i].username, username) == 0) 
        {
            puts("用户名已存在，请选择其他用户名。");
            getchar();
            return false;
        }
    }

    // 添加新用户
    if (user_count < MAX_USER) 
    {
        strncpy(users[user_count].username, username, MAX_USERNAME_LENGTH - 1);
        users[user_count].username[MAX_USERNAME_LENGTH - 1] = '\0';
        strncpy(users[user_count].password, password, MAX_PASSWORD_LENGTH - 1);
        users[user_count].password[MAX_PASSWORD_LENGTH - 1] = '\0';
        strncpy(users[user_count].level, "guest", MAX_LEVEL_LENGTH - 1);
        users[user_count].level[MAX_LEVEL_LENGTH - 1] = '\0';
        user_count++;

        if (!save_users(users, user_count)) 
        {
            fprintf(stderr, "保存用户数据失败。\n");
            getchar();
            return false;
        }

        puts("注册成功！");
        getchar();
        return true;
    } 
    else 
    {
        puts("用户数量已达上限，无法注册新用户。");
        getchar();
        return false;
    }
}