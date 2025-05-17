#include "versionTable.h" // 版本号表头文件，包含版本号表的结构体和函数声明
#include <stdio.h>      // 标准输入输出库，用于文件操作、输入输出等功能
#include <stdlib.h>     // 标准库，用于内存分配、进程控制等功能
#include <string.h>     // 字符串处理库，用于字符串操作函数
#include <time.h>      // 时间处理库，用于获取当前时间
#include <dirent.h>  // 目录操作库，用于遍历目录

// 用于判断目录项类型
#ifndef DT_DIR
#define DT_DIR 4
#endif

/**
 * 设备端使用，用来读取本地固件版本号
 */
/**
 * 设备端：存放固件版本列表
 */
typedef struct {
    char fileName[256];  // 固件名
    char version[16]; // 版本号
}verTable; // 版本号表结构体

#define VER_TABLE_COUNT 20 // 版本号个数
/**
 * 初始化版本号表
 */
void initVerTable(verTable *table) {
    // 读取版本号表文件
    FILE *fp = fopen("versionTable.txt", "rb+");
    if (fp == NULL) {
        printf("fail to open versionTable.txt\n");
        return;
    }
    fread(table, sizeof(verTable), VER_TABLE_COUNT, fp);
    fclose(fp);
}

/**
 * 写入固件版本号
 */
void writeVerTable(verTable *table) {
    // 写入版本号表文件
    FILE *fp = fopen("versionTable.txt", "wb+");
    if (fp == NULL) {
        printf("fail to open versionTable.txt\n");
        return;
    }
    fwrite(table, sizeof(verTable), VER_TABLE_COUNT, fp);
    fclose(fp);
}

/**
 * 服务器端使用，扫描app文件夹，存放每个固件的最新版本
 */
void readVerTable(verTable *table) {
    DIR *dir;
    struct dirent *entry;
    int i = 0;

    // 打开目录
    dir = opendir("app");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // 遍历目录中的文件
    /**
     * 扫描app文件夹
     * 1. 读取除了patch文件夹以外的所有文件夹
     * 2. 对读到的文件夹，记录文件夹名
     * 3. 遍历读到的文件夹，记录vx.x的最大值
     */
    while ((entry = readdir(dir)) != NULL) {
        // 排除当前目录和上级目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, "patch") == 0) {
            continue;
        }
        // 判断是否为目录
        if (entry->d_type == DT_DIR) {
            // 读取文件夹名
            strcpy(table[i].fileName, entry->d_name);
            table[i].version[0] = '\0'; // 初始化版本号为空字符串
            // 遍历该目录下的文件，使用compare_version函数比较版本号
            DIR *subdir;
            struct dirent *subentry;
            subdir = opendir(entry->d_name);
            while ((subdir = readdir(subdir)) != NULL) {
                // 排除当前目录和上级目录
                if (strcmp(subentry->d_name, ".") == 0 || strcmp(subentry->d_name, "..") == 0) {
                    continue;
                }
                // 判断是否为文件
                if (subentry->d_type == DT_DIR) {
                    // 比较版本号
                    if (compare_version(subentry->d_name, table[i].version) > 0) {
                        strcpy(table[i].version, subentry->d_name);
                    }
                }
            }
            ++i;
        }
    }

    closedir(dir);
}

int compare_version(const char *v1, const char *v2) {
    int n1, n2;
    // 假设格式为 vX.Y
    sscanf(v1, "v%d.%d", &n1, &n2);
    int v1_major = n1, v1_minor = n2;
    sscanf(v2, "v%d.%d", &n1, &n2);
    int v2_major = n1, v2_minor = n2;
    if (v1_major != v2_major)
        return v1_major > v2_major ? 1 : -1;
    if (v1_minor != v2_minor)
        return v1_minor > v2_minor ? 1 : -1;
    return 0;
}

#if 0
int main(){
    strcpy(table[0].fileName, "test.txt");
    strcpy(table[0].version, "v1.0");
    writeVerTable();

    return 0;
}
#endif