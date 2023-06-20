#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kernel.h"


// 磁盘容量
#define DISK_CAPACITY (1024 * 1024 * 1024)  // 1 GB 单位字节
#define SECTOR_SIZE 512





// BOOT区 扇区0~31===============================================================================================================
typedef struct {
    // offset 0~35  扇区编号 0
    unsigned char BS_JmpBoot[3];                // 0    3字节      跳转指令
    char BS_OEMName[8];                         // 3    8字节      OEM名称
    unsigned short BPB_BytsPerSec;              // 11   2字节      每个扇区的字节数
    unsigned char BPB_SecPerClus;               // 13   1字节      每簇的扇区数
    unsigned short BPB_RsvdSecCnt;              // 14   2字节      保留扇区数（2字节）
    unsigned char BPB_NumFATs;                  // 16   1字节      FAT表的数量
    unsigned short BPB_RootEntCnt;              // 17   2字节      根目录区的目录项数
    unsigned short BPB_TotSec16;                // 19   2字节      总扇区数
    unsigned char BPB_Media;                    // 21   1字节      媒体类型
    unsigned short BPB_FATSz16;                 // 22   2字节      每个FAT表的扇区数
    unsigned short BPB_SecPerTrk;               // 24   2字节      每个磁道的扇区数
    unsigned short BPB_NumHeads;                // 26   2字节      磁头数
    unsigned int BPB_HiddSec;                   // 28   4字节      隐藏扇区数
    unsigned int BPB_TotSec32;                  // 32   4字节      总扇区数
    // offset 36~511
    unsigned int BPB_FATSz32;                   // 36   4字节      FAT的扇区数
    unsigned short BPB_ExtFlags;                // 40   2字节      扩展标志
    unsigned short BPB_FSVer;                   // 42   2字节      FAT32版本
    unsigned int BPB_RootClus;                  // 44   4字节      根目录的第一个簇号
    unsigned short BPB_FSInfo;                  // 48   2字节      FSInfo结构所在扇区的偏移量
    unsigned short BPB_BkBootSec;               // 50   2字节      备份引导扇区所在扇区的偏移量
    unsigned char BPB_Reserved[12];             // 52   12字节     保留字段
    unsigned char BS_DrvNum;                    // 64   1字节      磁盘驱动器编号
    unsigned char BS_Reserved;                  // 65   1字节      保留字段
    unsigned char BS_BootSig;                   // 66   1字节      扩展引导标志
    unsigned int BS_VolID;                      // 67   4字节      卷序列号
    char BS_VolLab[11];                         // 71   11字节     卷标
    char BS_FilSysType[8];                      // 82   8字节      文件系统类型
    unsigned char BS_BootCode32[420];           // 90   420字节    引导程序
    unsigned short BS_BootSign;                 // 510  2字节      引导标记 0xAA55
} BootSector;  // 1扇区

typedef struct {
    // offset 512~1023  扇区编号 1
    unsigned int FSI_LeadSig;                    // 0    4字节      FSInfo主要签名
    unsigned char FSI_Reserved1[480];            // 4    480字节    保留字段
    unsigned int FSI_StrucSig;                   // 484  4字节      FSInfo结构签名
    unsigned int FSI_Free_Count;                 // 488  4字节      上次已知的空闲簇计数
    unsigned int FSI_Nxt_Free;                   // 492  4字节      提示FAT驱动程序开始查找空闲簇的簇号
    unsigned char FSI_Reserved2[12];             // 496  12字节     保留字段
    unsigned int FSI_TrailSig;                   // 508  4字节      FSInfo尾部签名
} FSInfoSector;  // 引导扇区 占1扇区

typedef struct {
    // offset 1024~4095  扇区编号 2~7
    unsigned char Reserved1[512];               // 0      512字节    保留字段
    unsigned char Reserved2[512];               // 512    512字节    保留字段
    unsigned char Reserved3[512];               // 1024   512字节    保留字段
    unsigned char Reserved4[512];               // 1536   512字节    保留字段
    unsigned char Reserved5[512];               // 2048   512字节    保留字段
    unsigned char Reserved6[512];               // 2560   512字节    保留字段
} BackupBootSector;  // FSInfo扇区 占6扇区

typedef struct {
    // offset 4096~16383  扇区编号 8~31
    unsigned char Reserved1[512];               // 0     512字节     保留字段
    unsigned char Reserved2[512];               // 512   512字节     保留字段
    unsigned char Reserved3[512];               // 1024  512字节     保留字段
    unsigned char Reserved4[512];               // 1536  512字节     保留字段
    unsigned char Reserved5[512];               // 2048  512字节     保留字段
    unsigned char Reserved6[512];               // 2560  512字节     保留字段
    unsigned char Reserved7[512];               // 3072  512字节     保留字段
    unsigned char Reserved8[512];               // 3584  512字节     保留字段
    unsigned char Reserved9[512];               // 4096  512字节     保留字段
    unsigned char Reserved10[512];              // 4608  512字节     保留字段
    unsigned char Reserved11[512];              // 5120  512字节     保留字段
    unsigned char Reserved12[512];              // 5632  512字节     保留字段
    unsigned char Reserved13[512];              // 6144  512字节     保留字段
    unsigned char Reserved14[512];              // 6656  512字节     保留字段
    unsigned char Reserved15[512];              // 7168  512字节     保留字段
    unsigned char Reserved16[512];              // 7680  512字节     保留字段
    unsigned char Reserved17[512];              // 8192  512字节     保留字段
    unsigned char Reserved18[512];              // 8704  512字节     保留字段
    unsigned char Reserved19[512];              // 9216  512字节     保留字段
    unsigned char Reserved20[512];              // 9728  512字节     保留字段
    unsigned char Reserved21[512];              // 10240 512字节     保留字段
    unsigned char Reserved22[512];              // 10752 512字节     保留字段
    unsigned char Reserved23[512];              // 11264 512字节     保留字段
    unsigned char Reserved24[512];              // 11776 512字节     保留字段
} ReservedSectors;  // 备份引导扇区 占24扇区

// FAT区 扇区32~ ================================================================================================================
// FAT[0]和FAT[1]是保留的，并且不与任何簇相关联
// FAT[0] = 0xFFFFFF??；FAT[1] = 0xFFFFFFFF；
// FAT[0]中的??的值与BPB_Media的值相同，但该条目没有任何功能。FAT[1]中的某些位记录错误历史。
// 第三个FAT项，FAT[2]，是与数据区的第一个簇相关联的项，有效的簇号从2开始
typedef unsigned int FATEntry;

// DATA区===============================================================================================================
// FAT目录项
typedef struct {
    // DIR_Name[0]，是一个重要的数据，用于指示目录项的状态。
    // 当值为0xE5时，表示该条目未使用（可供新分配）。
    // 当值为0x00时，表示该条目未使用（与0xE5相同），并且在此之后没有分配的条目（所有后续条目的DIR_Name[0]也都设置为0）
    char DIR_Name[11];                   // 0   11 bytes    文件名
    unsigned char DIR_Attr;              // 11  1 byte      文件属性
    unsigned char DIR_NTRes;             // 12  1 byte      可选的大小写信息标志
    unsigned char DIR_CrtTimeTenth;      // 13  1 byte      可选的文件创建时间的子秒信息
    unsigned short DIR_CrtTime;          // 14  2 bytes     可选的文件创建时间
    unsigned short DIR_CrtDate;          // 16  2 bytes     可选的文件创建日期
    unsigned short DIR_LstAccDate;       // 18  2 bytes     可选的最后访问日期
    unsigned short DIR_FstClusHI;        // 20  2 bytes     簇号高16位
    unsigned short DIR_WrtTime;          // 22  2 bytes     最后修改时间
    unsigned short DIR_WrtDate;          // 24  2 bytes     最后修改日期
    unsigned short DIR_FstClusLO;        // 26  2 bytes     簇号低16位
    unsigned int DIR_FileSize;           // 28  4 bytes     文件大小（单位字节），目录始终为0
} DirectoryEntry;

// =====================================================================================================================
// 数据区


// =====================================================================================================================
int read_sector(int sectorNumber, unsigned char *data) {  // 要读取的扇区号，从0开始计数
    FILE *imageFile = fopen("disk_image.img", "rb");
    if (imageFile == NULL) {
        display("Failed to open disk image file\n");
        return 0;
    }
    // 定位到要读取的扇区
    fseek(imageFile, sectorNumber * SECTOR_SIZE, SEEK_SET);
    // 读取指定扇区的数据
    if (fread(data, 1, SECTOR_SIZE, imageFile) != SECTOR_SIZE) {
        display("Failed to read sector %d\n", sectorNumber);
        fclose(imageFile);
        return -1;
    }
    fclose(imageFile);
    return 1;
}

int write_sector(int sectorNumber, const unsigned char *data) {
    FILE *imageFile = fopen("disk_image.img", "r+b");
    if (imageFile == NULL) {
        display("Failed to open disk image file\n");
        fclose(imageFile);
        return -1;
    }
    // 定位到要写入的扇区
    fseek(imageFile, sectorNumber * SECTOR_SIZE, SEEK_SET);
    // 写入数据到指定扇区
    if (fwrite(data, 1, SECTOR_SIZE, imageFile) != SECTOR_SIZE) {
        display("Failed to write sector %d\n", sectorNumber);
        fclose(imageFile);
        return -1;
    }
    fclose(imageFile);
    return 1;
}

int initial(){
    // 初始化磁盘
    // 创建磁盘镜像文件
    FILE *imageFile = fopen("disk_image.img", "w+b");
    if (imageFile == NULL) {
        fclose(imageFile);
        display("Failed to create disk image file\n");
        return 0;
    }
    // 扩展文件大小到指定容量
    fseek(imageFile, DISK_CAPACITY - 1, SEEK_SET);
    fputc(0, imageFile);

    // 初始化引导扇区
    BootSector bootSector;
    memset(&bootSector, 0, sizeof(BootSector));             // 初始化为零值
    memcpy(bootSector.BS_JmpBoot, "\xEB\x58\x90", 3);       // 跳转指令 设置跳转指令
    memcpy(bootSector.BS_OEMName, "MYFAT32 ", 8);           // OEM名称 设置OEM名称为 "MYFAT32"
    bootSector.BPB_BytsPerSec = 512;                                      // 每个扇区的字节数 512 Byte
    bootSector.BPB_SecPerClus = 8;                                        // 每簇的扇区数 8*512=4kB
    bootSector.BPB_RsvdSecCnt = 32;                                       // 保留扇区数 对应BootSector、FSInfoSector、BackupBootSector和ReservedSectors
    bootSector.BPB_NumFATs = 2;                                           // FAT表的数量
    bootSector.BPB_RootEntCnt = 0;                                        // 根目录区的目录项数 在FAT32卷中，该字段必须为0
    bootSector.BPB_TotSec16 = 0;                                          // 总扇区数 对于FAT32卷，该字段必须始终为0，表示总扇区数使用32位字段 BPB_TotSec32
    bootSector.BPB_Media = 0xF0;                                          // 媒体类型 非分区的可移动磁盘标识
    bootSector.BPB_FATSz16 = 0;                                           // 每个FAT表的扇区数 在FAT32卷中，它必须是一个无效值0，而使用BPB_FATSz32代替
    bootSector.BPB_SecPerTrk = 32;                                        // 每个磁道的扇区数 在使用FAT文件系统时，这个字段的值通常是固定的，例如32或63，不会对文件系统的正常运行产生影响。
    bootSector.BPB_NumHeads = 1;                                          // 磁头数 在使用FAT文件系统时，这个字段的值通常是固定的，例如1或255，不会对文件系统的正常运行产生影响。
    bootSector.BPB_HiddSec = 0;                                           // 隐藏扇区数 在使用FAT文件系统时，这个字段的值通常是固定的，例如0，不会对文件系统的正常运行产生影响
    bootSector.BPB_TotSec32 = DISK_CAPACITY/bootSector.BPB_BytsPerSec;    // 总扇区数 磁盘容量/扇区大小
    bootSector.BPB_FATSz32 = (DISK_CAPACITY/bootSector.BPB_BytsPerSec)*32/bootSector.BPB_BytsPerSec*bootSector.BPB_NumFATs;     // FAT的扇区数
    bootSector.BPB_ExtFlags = 0x01;                                       // 扩展标志
    bootSector.BPB_FSVer = 0x0000;                                        // FAT32版本
    bootSector.BPB_RootClus = 2;                                          // 根目录的第一个簇号
    bootSector.BPB_FSInfo = 1;                                            // FSInfo结构所在扇区的偏移量 第2个扇区
    bootSector.BPB_BkBootSec = 2;                                         // 备份引导扇区所在扇区的偏移量
    memset(bootSector.BPB_Reserved, 0, 12);                 // 保留字段
    bootSector.BS_DrvNum = 0;                                             // 磁盘驱动器编号
    bootSector.BS_Reserved = 0;                                           // 保留字段
    bootSector.BS_BootSig = 0;                                            // 扩展引导标志
    bootSector.BS_VolID = 0x12345678;                                     // 卷序列号
    memcpy(bootSector.BS_VolLab, "MY_VOLUME", 11);          // 卷标
    memcpy(bootSector.BS_FilSysType, "FAT32   ", 8);        // 文件系统类型
    memset(bootSector.BS_BootCode32, 0, 420);               // 引导程序
    bootSector.BS_BootSign = 0xAA55;                                      // 引导标记 0xAA55
    // 初始化FSInfo区
    FSInfoSector fsInfo;
    fsInfo.FSI_LeadSig = 0x41615252;
    memset(fsInfo.FSI_Reserved1, 0, 480);
    fsInfo.FSI_StrucSig = 0x61417272;
    fsInfo.FSI_Free_Count = 0xFFFFFFFF;
    fsInfo.FSI_Nxt_Free = 0xFFFFFFFF;
    memset(fsInfo.FSI_Reserved2, 0, 12);
    fsInfo.FSI_TrailSig = 0xAA550000;
    // 初始化备份引导扇区
    BackupBootSector backupBootSector;
    memset(backupBootSector.Reserved1, 0, bootSector.BPB_BytsPerSec);
    memset(backupBootSector.Reserved2, 0, bootSector.BPB_BytsPerSec);
    memset(backupBootSector.Reserved3, 0, bootSector.BPB_BytsPerSec);
    memset(backupBootSector.Reserved4, 0, bootSector.BPB_BytsPerSec);
    // 初始化备份引导扇区
    ReservedSectors reservedSectors;
    memset(reservedSectors.Reserved1, 0, bootSector.BPB_BytsPerSec);      // 0     bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved2, 0, bootSector.BPB_BytsPerSec);      // bootSector.BPB_BytsPerSec   bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved3, 0, bootSector.BPB_BytsPerSec);      // 2*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved4, 0, bootSector.BPB_BytsPerSec);      // 3*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved5, 0, bootSector.BPB_BytsPerSec);      // 4*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved6, 0, bootSector.BPB_BytsPerSec);      // 5*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved7, 0, bootSector.BPB_BytsPerSec);      // 6*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved8, 0, bootSector.BPB_BytsPerSec);      // 7*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved9, 0, bootSector.BPB_BytsPerSec);      // 8*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved10, 0, bootSector.BPB_BytsPerSec);     // 9*bootSector.BPB_BytsPerSec  bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved11, 0, bootSector.BPB_BytsPerSec);     // 10*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved12, 0, bootSector.BPB_BytsPerSec);     // 11*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved13, 0, bootSector.BPB_BytsPerSec);     // 12*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved14, 0, bootSector.BPB_BytsPerSec);     // 13*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved15, 0, bootSector.BPB_BytsPerSec);     // 14*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved16, 0, bootSector.BPB_BytsPerSec);     // 15*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved17, 0, bootSector.BPB_BytsPerSec);     // 16*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved18, 0, bootSector.BPB_BytsPerSec);     // 17*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved19, 0, bootSector.BPB_BytsPerSec);     // 18*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved20, 0, bootSector.BPB_BytsPerSec);     // 19*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved21, 0, bootSector.BPB_BytsPerSec);     // 20*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved22, 0, bootSector.BPB_BytsPerSec);     // 21*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved23, 0, bootSector.BPB_BytsPerSec);     // 22*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    memset(reservedSectors.Reserved24, 0, bootSector.BPB_BytsPerSec);     // 23*bootSector.BPB_BytsPerSec bootSector.BPB_BytsPerSec字节     保留字段
    // 写回虚拟磁盘
    // BootSector
    unsigned char buffer[sizeof(BootSector)];
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &bootSector, sizeof(BootSector));
    fseek(imageFile, 0, SEEK_SET);
    fwrite(buffer, 1, sizeof(buffer), imageFile);
    // FSInfoSector
    unsigned char buffer2[sizeof(FSInfoSector)];
    memset(buffer2, 0, sizeof(buffer2));
    memcpy(buffer2, &fsInfo, sizeof(FSInfoSector));
    fseek(imageFile, bootSector.BPB_FSInfo * bootSector.BPB_BytsPerSec, SEEK_SET);
    fwrite(buffer2, 1, sizeof(buffer2), imageFile);
    // BackupBootSector

    // ReservedSectors

    // 初始化FAT
    // 创建并初始化FAT表
    FATEntry *fatTable = (FATEntry *) malloc(bootSector.BPB_TotSec32 * sizeof(FATEntry) * 2);
    if (fatTable == NULL) {
        display("Failed to allocate memory for FAT table\n");
        return -1;
    }
    // 初始化FAT表
    for (unsigned int i = 0; i < bootSector.BPB_TotSec32 * 2; i++) {
        fatTable[i] = 0;  // 初始时所有簇都为空闲状态
    }
    // 定位到FAT表的扇区位置
    fseek(imageFile, bootSector.BPB_BytsPerSec * 32, SEEK_SET);
    // 写入FAT表到磁盘镜像文件
    if (fwrite(fatTable, 1, bootSector.BPB_TotSec32 * sizeof(FATEntry), imageFile) != bootSector.BPB_TotSec32 * sizeof(FATEntry)) {
        display("Failed to write FAT table to disk image file\n");
        fclose(imageFile);
        return -1;
    }
    // 释放FAT表的内存
    if (fatTable != NULL) {
        free(fatTable);
    }

    // 初始化根目录区




    fclose(imageFile);
}

