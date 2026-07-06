#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

// 从标准输入读取一行字符串
bool read_line(char *buffer, int size)
{
    if (!fgets(buffer, size, stdin)) // 先尝试读取到缓存中，如果读取失败，返回 false
        return false;

    size_t len = strlen(buffer); 
    if (len > 0 && buffer[len - 1] == '\n') // 如果读取到了换行符，将其替换为字符串结束符
        buffer[len - 1] = '\0';

    return true;
}

// 判断日期是否合法
bool is_valid_date(const char *date)
{
    if (!date || strlen(date) != 10) // 如果数据为空或者长度不为10，返回 false
        return false;

    for (int i = 0; i < 10; ++i) 
    {
        if (i == 4 || i == 7) // 检验格式YYYY-MM-DD中的-是否正确
        {
            if (date[i] != '-')
            {
                return false;
            }
        } 
        else if (!isdigit((unsigned char)date[i])) // 检验格式YYYY-MM-DD中的数字是否正确
        {
            return false;
        }
    }
    
    // 将字符串的年月日转换为整数
    int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + (date[3] - '0');
    int month = (date[5] - '0') * 10 + (date[6] - '0');
    int day = (date[8] - '0') * 10 + (date[9] - '0');
    
    if (year < 1900 || month < 1 || month > 12 || day < 1) // 验证年月日是否合法
        return false;

    // 检查每个月的天数是否正确
    int mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; 
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) 
    {
        mdays[1] = 29;
    }
    return day <= mdays[month - 1];
}

// 确保字符串不会和空指针进行比较，将其他返回false的内容改成空字符串
int safe_strcmp(const char *a, const char *b)
{
    if (!a) a = "";
    if (!b) b = "";
    return strcmp(a, b);
}
