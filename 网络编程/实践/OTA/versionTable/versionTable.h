#ifndef VERSIONTABLE_H
#define VERSIONTABLE_H

/**
 * 设备端：存放固件版本列表
 */
typedef struct {
    char fileName[256];  // 固件名
    char version[16]; // 版本号
} verTable; // 版本号表结构体

#define VER_TABLE_COUNT 20 // 版本号个数

void initVerTable(verTable *table);  // 设备端使用初始化table，读取不同固件对应的版本号
void writeVerTable(verTable *table);  // 设备端使用写入table，写入不同固件对应的版本号
void readVerTable(verTable *table);  // 服务器端使用，扫描app文件夹，存放每个固件的最新版本

#endif