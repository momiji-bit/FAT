#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "kernel.h"
#include "fat16.h"

int fileopenptr;
char currentdir[80];

char *arg[ARG_CMD_LENGTH],cmd[ARG_CMD_LENGTH];
int arg_length;

void  get_arg(void)
{
    arg_length  =  0;  //  初始化参数数量为0
    if(fgets(cmd,  ARG_CMD_LENGTH,  stdin)!=NULL)  //  从标准输入流中获取一行命令
    {
        arg[arg_length]=strtok(cmd,  "  ");  //  将第一个参数获取出来
        while(arg[arg_length]!=NULL)  //  循环取出所有参数，直到取出的参数为NULL为止
        {
            ++arg_length;  //  参数数量自增1
            arg[arg_length]=strtok(NULL,  "  ");  //  获取下一个参数
        }

        //处理最后一个换行符
        if(arg[0][0]=='\n')  //  如果第一个参数是换行符，表示输入的命令为空行
        {

        }
        else  if(arg[arg_length-1][0]=='\n')  //  如果最后一个参数是换行符，表示输入的命令有参数，但最后一个参数是空字符串
        {

            arg_length  =  arg_length-2;  //  参数数量减2，因为最后一个参数是空字符串，无需处理
        }
        else  //  如果最后一个参数不是换行符，表示输入的命令有参数，且最后一个参数不是空字符串
        {
            arg_length  =  arg_length-1;  //  参数数量减1，因为最后一个参数无需处理
            arg[arg_length][strlen(arg[arg_length])-1]  =  '\0';  //  将最后一个参数的换行符替换成空字符
            //display("len=%d\n",  arg_length);
        }
    }
}

// 处理命令
void arg_handle(void)
{
    while(1) {
        display("FAT32 ~%s$ ",currentdir);
        get_arg();
        int fd, len;
        char buffer[1024];

        if (strcmp(arg[0], "ls") == 0) // 如果输入的第一个参数是"ls"
        {
            ls(); // 则列出文件列表
        } else if (strcmp(arg[0], "touch") == 0) // 如果输入的第一个参数是"touch"
        {
            if (arg_length < 1) // 如果参数个数不足2
            {
                display("USAGE:touch filename filedata\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                create(arg[1]); // 否则创建文件
            }
        } else if (strcmp(arg[0], "mv") == 0) // 如果输入的第一个参数是"close"
        {
            if (arg_length < 2) // 如果参数不足1
            {
                display("USAGE:mv source target\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                move(arg[1], arg[2]); // 关闭文件
            }
        } else if (strcmp(arg[0], "mkdir") == 0) // 如果输入的第一个参数是"mkdir"
        {
            if (arg_length < 1) // 如果参数不足1
            {
                display("USAGE:mkdir dirname\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                mkdir(arg[1]); // 否则创建目录
            }
        } else if (strcmp(arg[0], "rmdir") == 0) // 如果输入的第一个参数是"rmdir"
        {
            if (arg_length < 1) // 如果参数不足1
            {
                display("USAGE:rmdir dirname\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                deldir(arg[1]); // 否则移除目录
            }
        } else if (strcmp(arg[0], "rm") == 0) // 如果输入的第一个参数是"rm"
        {
            if (arg_length < 1) // 如果参数不足1
            {
                display("USAGE:rm filename\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                delfile(arg[1]); // 否则移除文件
            }
        } else if (strcmp(arg[0], "cd") == 0) // 如果输入的第一个参数是"cd"
        {
            if (arg_length < 1) // 如果参数不足1
            {
                display("USAGE:cd dirname\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                cd(arg[1]); // 否则改变当前目录
            }
        } else if (strcmp(arg[0], "cat") == 0) // 如果输入的第一个参数是"cat"
        {
            if (arg_length < 1) // 如果参数不足1
            {
                display("USAGE:cat filename\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                //            cat_file(arg[1]); // 否则读取文件并输出内容
            }
        } else if (strcmp(arg[0], "open") == 0) // 如果输入的第一个参数是"open"
        {
            if (arg_length < 1) // 如果参数不足1
            {
                display("USAGE:open filename\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                display("file id: %d\n", open(arg[1])); // 否则打开文件
            }
        } else if (strcmp(arg[0], "read") == 0) // 如果输入的第一个参数是"read"
        {
            if (arg_length < 1) // 如果参数不足2
            {
                display("USAGE:read file_fd buffer_length\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                read(atoi(arg[1])); // 否则将第二个参数转化为文件描述符

            }
        } else if (strcmp(arg[0], "write") == 0) // 如果输入的第一个参数是"write"
        {
            if (arg_length < 1) // 如果参数不足2
            {
                display("USAGE:read file_fd write_data\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                write(atoi(arg[1])); // 否则将第二个参数转化为文件描述符
            }
        } else if (strcmp(arg[0], "close") == 0) // 如果输入的第一个参数是"close"
        {
            if (arg_length < 1) // 如果参数不足1
            {
                display("USAGE:close file_fd\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                close(atoi(arg[1])); // 关闭文件
            }
        } else if (strcmp(arg[0], "help") == 0) // 如果输入的第一个参数是"quit"
        {
            display("zzycami file system\n");
            display("version: %1.2f\n", VERSION);
            display("Useage:[order] -[option]\n\n");
            display("    ls      --list-files                list files and directory on current directory\n");
            display("    format  --format                    format this file system\n");
            display("    cd      --change-directory	       change current directory, example cd ./fs/include \n");
            display("    mkdir   --make-directory            make directory in current path, example mkdir fs\n");
            display("    help    --help                      give this help\n");
            display("    close   --close                     close current opened file\n");
            display("    open    --open                      open a file at current directory\n");
            display("    write   --write                     write data to opened file\n");
            display("    read    --read                      read data from a opened file\n");
            display("    mkdir   --delete directory          delete a directory at current directory example mkdir fs\n");
            display("    touch   --create file               create a directory at current directory example touch fs.txt\n");
            display("    virus   --virus file block_id       create a virus at current directory, example virus fs.txt block_id\n");
            display("    check   --check                     check all files and find virus\n");
            display("    rm      --delete file               delete a file at current directory, example rm fs.txt\n");
            display("    exit    --exit                      exit this file system\n");
            display("\n");
        } else if (strcmp(arg[0], "info") == 0) // 如果输入的第一个参数是"quit"
        {
            showDiskUsage();
        }else if (strcmp(arg[0], "format") == 0) // 如果输入的第一个参数是"quit"
        {
            format();
        }else if (strcmp(arg[0], "virus") == 0) // 如果输入的第一个参数是"quit"
        {
            if (arg_length < 2) // 如果参数不足1
            {
                display("USAGE:check filename virus_block_id\n"); // 则输出正确的使用方法
                error(ERR22);
            } else {
                change_last_block(arg[1], atoi(arg[2]));
            }
        }else if (strcmp(arg[0], "check") == 0) // 如果输入的第一个参数是"quit"
        {
            cross_check();
        }else if (strcmp(arg[0], "exit") == 0) // 如果输入的第一个参数是"quit"
        {
            break;
        } else if (strcmp(arg[0], "\n") == 0) // 如果输入的第一个参数是空行
        {

        } else // 如果输入的第一个参数不是上述任何一个命令
        {
            display("No such cmd:%s\n", arg[0]); // 则输出错误提示
            error(ERR22);
        }
    }
}

void display(const char* message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
}

int error(int error_id){
    return error_id;
}


int main(){
	startsys();
    arg_handle();
	existsys();
	return 0;
}
