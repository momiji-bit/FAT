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


#define ERR1  (-10001)  // "please input directory name!\n"
#define ERR2  (-10002)  // "directopry name or file name can't contain / \\ : * ? \" < > | \n"
#define ERR3  (-10003)  // "already have the file or directory has the same name, please create a new name!\n"
#define ERR4  (-10004)  // "have no such directory!\n"
#define ERR5  (-10005)  // "please input directory name!\n"
#define ERR6  (-10006)  // "error ocurred when modify fat!\n"

#define ERR7  (-10007)  // "Error opening file.\n"
#define ERR8  (-10008)  // "current directory is full, please change a directory and create new file!\n"
#define ERR9  (-10009)  // "file doesn't exist!\n"
#define ERR10 (-10010)  // "illegal file id(%d)!\n", fid
#define ERR11 (-10011)  // "you haven't open file whose fid is %d!", fid
#define ERR12 (-10012)  // "have no opened file or the file you opened can't write!\n"
#define ERR13 (-10013)  // "write file error!\n"
#define ERR14 (-10014)  // "fat error!\n"
#define ERR15 (-10015)  // "file resrouce id is error , may be you have not open it"
#define ERR16 (-10016)  // "have no opened file or the file you opened can't write!\n"
#define ERR17 (-10017)  // "please input file name!\n"
#define ERR18 (-10018)  // "Source file doesn't exist.\n"
#define ERR19 (-10019)  // "Destination directory doesn't exist.\n"
#define ERR20 (-10020)  // "Destination directory is full. Cannot move file.\n"
#define ERR21 (-10021)  // "Error opening file.\n"

#define ERR22 (-10022)  // "CMD errors"

#define OUT1 (10001)  //





#endif //_CONST_H
