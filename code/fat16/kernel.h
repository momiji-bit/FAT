/* kernel.h contain all the function prototypes */
#ifndef _KERNEL_H
#define _KERNEL_H
/* system function prototypes */
void startsys();
void existsys();
void display(const char* message, ...);
int error(int error_id);

/* disk function prototypes */
void format();
void showDiskUsage();

/* directory function prototypes */
void mkdir(char *dirname);
void cd(char *dirname);
void deldir(char *dirname);
void ls();

/* file function prototypes */
int create(char *filename);
void delfile(char *filename);
int open(char *filename);
void close(int fid);
int write(int fid);
int dowrite(int fid, char *text, int len, char wstyle);
int read(int fid);
void move(char *src, char *dest);

void cross_check();
void change_last_block(char * filename, unsigned short id );

#endif
