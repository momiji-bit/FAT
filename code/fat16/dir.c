#include "kernel.h"
#include "fat16.h"
#include "stack.h"
#include <time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

unsigned char *vhard; /* pointer to the start of vertual disk*/
useropen openfilelist[MAXOPENFILE]; /* array of user open file list*/
useropen ptrcurdir; /*pointer to current user open list*/
int Dirs[1000]={0};

void ls(){
    fcb *file; // 定义指向fcb结构体的指针变量file
    char type[11]; // 定义长度为11的字符数组type
    int i = BLOCKSIZE/sizeof(fcb); // 计算每个磁盘块中fcb结构体的数量，并赋值给变量i
    file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE); // 计算当前目录在虚拟硬盘中的地址，并将其赋值给file

    while((i--) > 0){ // 循环遍历fcb结构体
        int year, mon, day; // 定义年、月、日的变量
        int hour, min, sec; // 定义小时、分钟、秒的变量
        int length; // 定义文件长度的变量

        if(!strcmp(file->filename, "")){ // 判断当前fcb结构体的文件名是否为空
            file ++; // 如果为空，指针移向下一个fcb结构体
            continue; // 继续下一次循环
        }

        if(file->attribute == 0x4){ // 判断当前fcb结构体的属性是否为目录
            length = sizeof(fcb) * file->length; // 如果是目录，计算目录的长度
            strcpy(type, "dir"); // 将类型字符串设为"dir"
        }else {
            int l = strlen(file->filename); // 获取当前fcb结构体文件名的长度
            length = file->length; // 如果不是目录，获取文件的长度
            while(l--){ // 从文件名的末尾向前遍历
                if(file->filename[l] == '.'){ // 判断是否找到文件名中的点字符
                    strcpy(type, (char *)(file->filename + l*sizeof(char) + 1)); // 将点后面的字符复制到type数组中
                    break; // 找到后跳出循环
                }
            }
        }

        year = ((file->date & 0xfe00) >> 9) + 80 + 1900; // 计算文件的年份
        mon = ((file->date & 0x01e0) >> 5); // 计算文件的月份
        day = file->date & 0x001f; // 计算文件的日期

        hour = (file->time & 0xf800) >> 11; // 计算文件的小时
        min = (file->time & 0x07e0) >> 5; // 计算文件的分钟
        sec = (file->time & 0x001f) << 1; // 计算文件的秒数

        display("%8d %04d-%02d-%02d %02d:%02d:%02d %6s %s\n",length , year, mon, day, hour, min, sec, type, file->filename); // 打印文件信息
        file ++; // 指针移向下一个fcb结构体
    }
}

void mkdir(char *dirname){
	fcb * dir, *temp;
	time_t *now;
	struct tm *current;
	fat *table = (fat *)(vhard + BLOCKSIZE);
	fat *table2 = (fat *)(vhard + 3*BLOCKSIZE);
	int i = 0;

	
	//check the dir name
	if((!strcmp(dirname, "")) || (!strcmp(dirname, ".")) || (!strcmp(dirname, ".."))){
		display("please input directory name!\n");
        error(ERR1);
		return ;
	}
	for(i = 0; i < (int)strlen(dirname) ; i ++){
		if(dirname[i] == '/' 
			|| dirname[i] == '\\' 
			|| dirname[i] == ':' 
			|| dirname[i] == '*'
			|| dirname[i] == '?'
			|| dirname[i] == '"'
			|| dirname[i] == '<'
			|| dirname[i] == '>'
			|| dirname[i] == '|'){
            display("directopry name or file name can't contain / \\ : * ? \" < > | \n");
            error(ERR2);
            return ;
		}
	}
	
	temp = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i < BLOCKSIZE/sizeof(fcb) ; i ++){
		if(!strcmp(temp->filename, dirname)){
			display("already have the file or directory has the same name, please create a new name!\n");
            error(ERR3);
			return ;
		}
		temp ++;
	}

	//create the new fcb
	dir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i< BLOCKSIZE/sizeof(fcb); i++){
		if(!strcmp(dir->filename, "")){
			break;
		}
		dir ++;
	}
	//dir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE + ptrcurdir.length * sizeof(fcb));
	strcpy(dir->filename, dirname);
	dir->attribute = 0x4;
	
	now = (time_t*)malloc(sizeof(time_t));
	time(now);
	current = localtime(now);
	dir->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	dir->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	
	for(i=0;i<(BLOCKSIZE*2)/sizeof(fat);i++){
		//display("i:%d - fat:%d\n",i,(int)table->id);
		if(table->id == FREE){
			break;
		}
		table ++;
		table2 ++;
	}
	dir->first = i;
    Dirs[i] = 1;
	//display("alloc block:%d\n", i);
	table->id = END;
	table2->id = END;
	dir->length = 2;
	ptrcurdir.length ++;
	((fcb*)(vhard + ptrcurdir.dirno * BLOCKSIZE + ptrcurdir.diroff * sizeof(fcb)))->length ++;

	// create two fcb . and ..
	dir = (fcb*)(vhard + dir->first * BLOCKSIZE);

	now = (time_t*)malloc(sizeof(time_t));
	strcpy(dir->filename, ".");
	dir->attribute = 0x4;
	time(now);
	current = localtime(now);
	dir->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	dir->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	/* the fcb->first that filename is '.' point to the current block id */
	dir->first = i;
	dir->length = 1;

	dir ++;
	strcpy(dir->filename, "..");
	dir->attribute = 0x4;
	time(now);
	current = localtime(now);
	dir->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
	dir->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);
	/* the fcb.first that filename is '..' point to the id of father of current dir*/
	dir->first = ptrcurdir.first;
	dir->length = 1;
}

void cd(char *dirname){
	char dirs[MAXOPENFILE][80];
	int dirslength = 0;
	char currentdirc[MAXOPENFILE][80];
	int currentdirclength = 0;
	int x = 0, y = 0;
	int i = 0;
	fat* table = (fat *)(vhard + BLOCKSIZE);
	fcb* cdir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	fcb* root = (fcb *)(vhard + 5*BLOCKSIZE);
	int off;
	int limit;
	int issort = 0;
	
	// 分割当前路径
	i = 1;
	while((currentdir[i] != '\0') && (i < 110)){
		if(currentdir[i] != '/'){
			currentdirc[x][y] = currentdir[i];
			y ++;
		}else {
			currentdirc[x][y] = '\0';
			y = 0;
			x ++;
		}
		i++;
	}
	currentdirc[x][y] = '\0';
	currentdirclength = x + 1;

	// 分割目标路径
	dirslength = strlen(dirname);
	if(dirname[dirslength - 1] == '/'){
		dirname[dirslength - 1] = '\0';
	}
	x = 0; y = 0;i = 0;
	while((dirname[i] != '\0') && (i < 110)){
		if(dirname[i] != '/'){
			dirs[x][y] = dirname[i];
			y ++;
		}else {
			dirs[x][y] = '\0';
			y = 0;
			x ++;
		}
		i++;
	}
	dirs[x][y] = '\0';
	dirslength = x + 1;
	
	// 切换目录
	for(i = 0;i < dirslength;i ++){
		int isfind = 0;
		limit = BLOCKSIZE/sizeof(fcb);// 防止 cdir 超过一个块的大小
		while(limit--){
			if((!strcmp(cdir->filename, dirs[i])) && (cdir->attribute == 0x4)){
				isfind = 1;
				break;
			}
			cdir ++;
		}
		if(isfind == 0){
			display("have no such directory!\n");
            error(ERR4);
			return ;
		}else {
			cdir = (fcb*)(vhard + cdir->first * BLOCKSIZE);
		}
	}

	// 查找当前目录的绝对路径
	x = dirslength;i = 0;
	while((x--) > 0){
		if(!strcmp(dirs[i], ".")){
			//do nothing
		}else if(!strcmp(dirs[i], "..")){
			currentdirclength = currentdirclength > 1?currentdirclength - 1:currentdirclength;
		}else {
			if(!strcmp(dirs[i], "")){
				strcpy(currentdirc[currentdirclength ++], "");
			}else {
				strcpy(currentdirc[currentdirclength++], dirs[i]);
			}
		}
		i ++;
	}
	
	strcpy(currentdir, "");
	for(i=0;i<currentdirclength;i++){
		strcat(currentdir, "/");
		strcat(currentdir,currentdirc[i]);
	}
	

	// 填充打开文件列表
	cdir ++;off = 0;
	cdir = (fcb *)(vhard + cdir->first * BLOCKSIZE);
	openfilelist[fileopenptr].dirno = cdir->first;
	if(strcmp(currentdirc[currentdirclength-1], "root")){
		limit = BLOCKSIZE/sizeof(fcb);//avoid cdir over one blocksize
		while(limit--){
			if(!strcmp(cdir->filename, currentdirc[currentdirclength-1])){
				break;
			}
			cdir ++;
			off ++;
		}
	}
	strcpy(openfilelist[fileopenptr].filename, cdir->filename);
	openfilelist[fileopenptr].attribute = 0x4;
	openfilelist[fileopenptr].time = cdir->time;
	openfilelist[fileopenptr].date = cdir->date;
	openfilelist[fileopenptr].first = cdir->first;
	openfilelist[fileopenptr].length = cdir->length;
	openfilelist[fileopenptr].count = 1;
	openfilelist[fileopenptr].diroff = off;
	openfilelist[fileopenptr].fcbstate = 0;
	for(i=0;i<currentdirclength;i++){
		strcpy(openfilelist[fileopenptr].dir[i], currentdirc[i]);
	}
	openfilelist[fileopenptr].free = 1;
	openfilelist[fileopenptr].topenfile = 0;
	ptrcurdir = openfilelist[fileopenptr];
}

void deldir(char *dirname){
	fat * fat1, *fat2;
	fcb *dir, *ddir;
	int i = 0;
	//check the dir name
	if((!strcmp(dirname, "")) || (!strcmp(dirname, ".")) || (!strcmp(dirname, ".."))){
		display("please input directory name!\n");
        error(ERR5);
		return ;
	}
	for(i = 0; i < (int)strlen(dirname) ; i ++){
		if(dirname[i] == '/' 
			|| dirname[i] == '\\' 
			|| dirname[i] == ':' 
			|| dirname[i] == '*'
			|| dirname[i] == '?'
			|| dirname[i] == '"'
			|| dirname[i] == '<'
			|| dirname[i] == '>'
			|| dirname[i] == '|'){
            display("directopry name or file name can't contain / \\ : * ? \" < > | \n");
            error(ERR2);
            return ;
		}
	}
	
	ddir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i < BLOCKSIZE/sizeof(fcb); i ++){
		if(!strcmp(ddir->filename, dirname)){
			break;
		}
		ddir ++;
	}
	if(i == BLOCKSIZE/sizeof(fcb)){
		display("have no such directory!\n");
        error(ERR4);
		return ;
	}else {
		//delete the directory
		int all = BLOCKSIZE/sizeof(fcb);

		((fcb*)(vhard + ptrcurdir.dirno * BLOCKSIZE + ptrcurdir.diroff * sizeof(fcb)))->length --;
		fat1 = (fat *)(vhard + BLOCKSIZE);
		fat2 = (fat *)(vhard + 3*BLOCKSIZE);

		// Traverse and delete
		clear();
		push(ddir);
		while(!isempty()){
			fcb* temp;
			int all = BLOCKSIZE/sizeof(fcb) - 2;
			dir = (fcb *)pop();
			temp = dir; // 父目录
			dir = (fcb *)(vhard + dir->first * BLOCKSIZE + 2 * sizeof(fcb)); // 更新为子目录
			memset(temp, 0, sizeof(fcb));
			while((all --) > 0){
				if(strcmp(dir->filename, "")){
					if(dir->attribute == 0x4){
						//if it is a directory
						fat1 = (fat *)(vhard + BLOCKSIZE);
						fat2 = (fat *)(vhard + 3*BLOCKSIZE);
						fat1 += dir->first;
						fat2 += dir->first;
						if(fat1->id == END){
							fat1->id = FREE;
							fat2->id = FREE;
						}else {
							display("error ocurred when modify fat!\n");
                            error(ERR6);
						}
						push((void *)dir);
					}else if(dir->attribute == 0x5 ){//delete the file
						delfile(dir->filename);
					}//else if(dir->attribute == 0x5 )

				}//if(strcmp(dir->filename, ""))
				dir ++;
			}//while((all --) > 0)
		}//while((all --) > 0)
	}
}
