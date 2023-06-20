#include "const.h"
#ifndef _FAT16_H
#define _FAT16_H

/* file control block*/
#ifndef __FS_FAT16_FCB__
#define __FS_FAT16_FCB__
typedef struct FCB{
	char filename[11];/*file name include extend file name*/
	unsigned char attribute;/*file attribution*/
	unsigned char other[10];
	unsigned short time;/*file create time*/
	unsigned short date;/*file create data*/
	unsigned short first;/*file start disk block*/
	unsigned int length;/*file length*/
}fcb;
#endif//__FS_FAT16_FCB__

/* file allocation table */
#ifndef __FS_FAT16_FAT__
#define __FS_FAT16_FAT__
typedef struct FAT{
	unsigned short id;
}fat;
#endif//__FS_FAT16_FAT__

/* user open table*/
#ifndef __FS_FAT16_USEROPEN__
#define __FS_FAT16_USEROPEN__
typedef struct USEROPEN {
    char filename[11]; // 文件名，包括扩展名
    unsigned char attribute; // 文件属性
    unsigned short time; // 文件创建时间
    unsigned short date; // 文件创建日期
    unsigned short first; // 文件起始磁盘块号
    unsigned short length; // 文件长度
    char free; // 标记该目录是否为空，0 表示空，1 表示已分配
    int dirno; // 父目录中已打开文件所在的磁盘块号
    int diroff; // 父目录中已打开文件的磁盘块号的索引
    char dir[MAXOPENFILE][80]; // 已打开文件的目录，用于快速检查文件是否已打开
    int count; // 文件指针的位置
    char fcbstate; // FCB 内容是否被修改，如果被修改则设为 1，否则设为 0
    char topenfile; // 已打开文件表项是否为空，如果值为 0 则为空，否则被占用
} useropen;
#endif//__FS_FAT16_USEROPEN__

#ifndef __FS_FAT16_BLOCK0__
#define __FS_FAT16_BLOCK0__
typedef struct BLOCK0 {
	char name[20];/* name of the file system */
	float version;
	int blocksize;
	int size;
	int maxopenfile;
	unsigned short root;
}block0;
#endif //__FS_FAT16_BLOCK0__

extern unsigned char *vhard; /* pointer to the start of vertual disk*/
extern useropen openfilelist[MAXOPENFILE]; /* array of user open file list*/
extern int fileopenptr;
extern useropen ptrcurdir; /*pointer to current user open list*/
extern int Dirs[1000]; /*pointer to current user open list*/
extern int DirNum; /*pointer to current user open list*/
extern char currentdir[80]; /*recode cureent dir (include path)*/
extern unsigned char* startp; /* recode the start position in data area of vertual disk*/

#endif //_FAT16_H
