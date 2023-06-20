#ifndef _CONST_H
#define _CONST_H

#define BLOCKSIZE	1024 /* the size of disk block */
#define VERSION 	0.01
#define SIZE		1024000 /* the size of virtual disk */
#define END			65535 /* the flag whitch means end of file in fat*/
#define FREE		0 /* the flag whitch means the disk block is free */
#define ROOTBLOCKNUM	2 /* the number of root block  */
#define MAXOPENFILE	10 /* the maximum number to open file in the same time */
#define ARG_CMD_LENGTH       1024  //命令参数最大长度
#define PROGRESS_BAR_WIDTH 20  // 进图条长度

#endif //_CONST_H
