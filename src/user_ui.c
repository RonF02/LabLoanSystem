#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "model.h"
#include "utils.h"
#include "user.h"
#include "storage.h"

bool login_manage_menu(User *current_user) 
{
    User users[MAX_USER];
    int user_count = 0;

    if (!load_users(users, &user_count)) 
    {
        fprintf(stderr, "警告：读取用户数据失败，系统将以空数据启动。\n");
    }

    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    printf("请输入用户名: ");
    if (!read_line(username, sizeof(username))) 
    {
        puts("读取输入失败");
        getchar();
        return false;
    }

    printf("请输入密码: ");
    if (!read_line(password, sizeof(password))) 
    {
        puts("读取输入失败");
        getchar();
        return false;
    }

    for (int i = 0; i < strlen(username); ++i) 
    {
        if (username[i] == '\n') 
        {
            username[i] = '\0';
            break;
        }
    }

    for (int i = 0; i < strlen(password); ++i) 
    {
        if (password[i] == '\n') 
        {
            password[i] = '\0';
            break;
        }
    }

    // 验证用户名和密码
    bool login_success = verify_login(username, password, current_user);

    if (login_success) 
    {
        puts("登录成功！按下回车进入系统");
        getchar();
        return 1;
    } 
    else 
    {
        puts("用户名或密码错误，请重新尝试。");
        getchar();
        return 0;
    }
}

bool register_manage_menu() 
{
    User users[MAX_USER];
    int user_count = 0;

    if (!load_users(users, &user_count)) 
    {
        fprintf(stderr, "警告：读取用户数据失败，系统将以空数据启动。\n");
    }

    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    printf("请输入用户名: ");
    if (!read_line(username, sizeof(username))) 
    {
        puts("读取输入失败");
        getchar();
        return false;
    }

    printf("请输入密码: ");
    if (!read_line(password, sizeof(password))) 
    {
        puts("读取输入失败");
        getchar();
        return false;
    }

        for (int i = 0; i < strlen(username); ++i) 
    {
        if (username[i] == '\n') 
        {
            username[i] = '\0';
            break;
        }
    }

    for (int i = 0; i < strlen(password); ++i) 
    {
        if (password[i] == '\n') 
        {
            password[i] = '\0';
            break;
        }
    }

    // 注册新用户
    bool register_success = register_user(username, password);

    if (register_success) 
    {
        puts("注册成功！");
        return 1;
    } 
    else 
    {
        puts("注册失败，请重新尝试。");
        getchar();
        return 0;
    }
}

bool show_menu(User *current_user)
{   
    char choice[16];
    puts("=== 欢迎使用实验室物品出入账管理系统 ===");
    puts("=== 用户菜单 ===");
    puts("1. 用户登录");
    puts("2. 用户注册");
    puts("0. 退出系统");
    printf("请选择操作: ");

    if (!read_line(choice, sizeof(choice))) 
    {
        puts("读取输入失败，程序退出。");
        return false;
    }

    if (strcmp(choice, "1") == 0) 
    {
        if (login_manage_menu(current_user)) // 用户登录逻辑
        {
            return 1;
        }
        else
        {
            clear_screen();
            return show_menu(current_user); // 登录失败后重新显示菜单
        }
    } 
    else if (strcmp(choice, "2") == 0) 
    {
        register_manage_menu(); // 用户注册逻辑
        clear_screen();
        return show_menu(current_user); // 注册后需要登录再进入系统
    } 
    else if (strcmp(choice, "0") == 0) 
    {
        puts("退出系统，感谢使用！\n");
        return 0;
    } 
    else 
    {
        puts("无效选择，请重新选择。\n");
        clear_screen();
        return show_menu(current_user); // 无效选择后重新显示菜单
    }
}


bool show_user_menu(User *current_user) 
{
    clear_screen();
    
    if (!ensure_data_directory()) 
    {
        fprintf(stderr, "无法创建数据目录或数据文件，请检查权限。\n");
        exit(EXIT_FAILURE);
    }

    return show_menu(current_user); // 显示用户菜单并处理用户选择
}

