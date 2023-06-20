#include "kernel.h"
#include "fat16.h"
#include "stack.h"

#include <time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>


int create(char *filename){
	int i = 0;
	fcb *temp, *file;
	time_t *now;
	fat *fat1, *fat2;
	struct tm *current;
	//check the dir name
		if((!strcmp(filename, "")) || (!strcmp(filename, ".")) || (!strcmp(filename, ".."))){
			display("please input directory name!\n");
            error(ERR1);
			return -1;
		}
		for(i = 0; i < (int)strlen(filename) ; i ++){
			if(filename[i] == '/'
				|| filename[i] == '\\'
				|| filename[i] == ':'
				|| filename[i] == '*'
				|| filename[i] == '?'
				|| filename[i] == '"'
				|| filename[i] == '<'
				|| filename[i] == '>'
				|| filename[i] == '|'){
                display("file name or file name can't contain / \\ : * ? \" < > | \n");
                error(ERR2);
                return -1;
			}
		}

		temp = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
		for(i = 0;i < BLOCKSIZE/sizeof(fcb) ; i ++){
			if(!strcmp(temp->filename, filename)){
				display("already have the file or directory has the same name, please create a new name!\n");
                error(ERR3);
				return -1;
			}
			temp ++;
		}

		//create the new fcb
		file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
		for(i = 0;i< BLOCKSIZE/sizeof(fcb); i++){
			if(!strcmp(file->filename, "")){
				break;
			}
			file ++;
		}
		if(i == BLOCKSIZE/sizeof(fcb)){
			display("current directory is full, please change a directory and create new file!\n");
            error(ERR8);
		}else {
			int j = 0;
			strcpy(file->filename, filename);
			file->attribute = 0x5;
			now = (time_t *)malloc(sizeof(time_t));
			time(now);
			current = localtime(now);
			file->date = ((current->tm_year-80)<<9) + ((current->tm_mon +1)<<5) + current->tm_mday;
			file->time = (current->tm_hour<<11) + (current->tm_min<<5) +(current->tm_sec>>1);

			fat1 = (fat *)(vhard + BLOCKSIZE);
			fat2 = (fat *)(vhard + 3*BLOCKSIZE);
			for(j = 0;j < (2*BLOCKSIZE)/sizeof(fat);j++){
				if(fat1->id == FREE){
					break;
				}
				fat1 ++;
				fat2 ++;
			}

			file->first = j;
			fat1->id = END;
			fat2->id = END;
			file->length = BLOCKSIZE;
		}

		return 0;
}


int open(char *filename){
	fcb *file;
	fat *fat1;
	int limit, isfind;
	int l, i, off;
	char name[81];
	char exname[10];
	
	limit = BLOCKSIZE/sizeof(fcb);
	file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	isfind = 0;off = 0;
	while((limit --) > 0){
		// deal the file name
		l = strlen(file->filename);
		strcpy(name, file->filename);
		while(l --){
			if(name[l] == '.'){
				strcpy(exname, (char *)(name + l*sizeof(char) + 1));
				name[l] = '\0';
				break;
			}
		}

		// find 
		if((!strcmp(file->filename, filename) || !strcmp(name, filename)) && file->attribute == 0x5){
			isfind = 1;
			break;
		}
		file ++;
		off ++;
	}
	if(!isfind){
		display("file doesn't exist!\n");
        error(ERR9);
		return -1;
	}else {
		int j =0;
		//check free user open file list
		for(j = 0 ; j< MAXOPENFILE; j ++){
			if(!strcmp(openfilelist[j].filename, "")){
				fileopenptr = j;
				break;
			}else if(!strcmp(openfilelist[j].filename, filename)){
				fileopenptr = j;
				return j;
			}
		}
		strcpy(openfilelist[fileopenptr].filename, file->filename);
		openfilelist[fileopenptr].attribute = file->attribute;
		openfilelist[fileopenptr].date = file->date;
		openfilelist[fileopenptr].time = file->time;
		openfilelist[fileopenptr].dirno = ptrcurdir.first;
		openfilelist[fileopenptr].first = ((fcb*)(vhard + ptrcurdir.first * BLOCKSIZE + off * sizeof(fcb)))->first;
		openfilelist[fileopenptr].diroff = off;
		openfilelist[fileopenptr].fcbstate = 0;
		openfilelist[fileopenptr].length = file->length;
		openfilelist[fileopenptr].topenfile = 1;
		openfilelist[fileopenptr].fcbstate = 0;
		return fileopenptr;
	}
	return 0;
}

void close(int fid){
	if(fid < 0 || fid > MAXOPENFILE){
		display("illegal file id(%d)!\n", fid);
        error(ERR10);
		return ;
	}

	if(!strcmp(openfilelist[fid].filename, "")){
		display("you havn't open file whose fid is %d!", fid);
        error(ERR11);
		return ;
	}

	strcpy(openfilelist[fid].filename, "");
	openfilelist[fid].attribute = 0x0;
	openfilelist[fid].date = 0;
	openfilelist[fid].time = 0;
	openfilelist[fid].dirno = 0;
	openfilelist[fid].first = 0;
	openfilelist[fid].diroff = 0;
	openfilelist[fid].fcbstate = 0;
	openfilelist[fid].length = 0;
	openfilelist[fid].topenfile = 0;
	openfilelist[fid].fcbstate = 0;
	fileopenptr = -1;
}

int write(int fid){
	char text[BLOCKSIZE + 1];
	fcb * file = (fcb *)(vhard + openfilelist[fid].dirno * BLOCKSIZE + openfilelist[fid].diroff * sizeof(fcb));
	int point;
	int len;
	char c;
	if(openfilelist[fid].attribute != 0x5){
		display("have no opened file or the file you opened can't write!\n");
        error(ERR12);
		return -1;
	}
	display("Edit Mode:\n");
	point = 0;
	openfilelist[fid].length = 0;
	while((c = getchar()) != EOF){
		if(point == BLOCKSIZE){
			if(dowrite(fid, text, BLOCKSIZE, 3) == -1){
				display("write file error!\n");
                error(ERR13);
				return -1;
			}
			openfilelist[fid].length += point;
			point = 0;
		}
		text[point ++] = c;
	}
	//len = strlen(text);
	openfilelist[fid].length += point;
	file->length = openfilelist[fid].length;
	if(dowrite(fid, text, point, 3) == -1){
		display("write file error!\n");
        error(ERR13);
		return -1;
	}
	display("\n");

}

int dowrite(int fid, char *text, int len, char wstyle){
	char *p;
	fat * fat1, *fat2;
	if(len >= BLOCKSIZE){//if previous block is used up
		int i = 0;
		fat *temp = (fat *)(vhard + BLOCKSIZE);
		// find the last block
		fat1 = (fat *)(vhard + BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		i = openfilelist[fid].first;
		while(fat1->id != END){
			if(fat1->id == FREE){
				display("fat error!\n");
                error(ERR14);
				return -1;
			}
			i = fat1->id;
			fat1 = (fat *)(vhard + BLOCKSIZE + fat1->id * sizeof(fat));
			fat2 = (fat *)(vhard + 3 * BLOCKSIZE + fat1->id * sizeof(fat));
		}
		//fill the end block
		p = (char *)(vhard + i * BLOCKSIZE);
		strcpy(p, text);
		// find a free block
		for(i = 0; i < 2*BLOCKSIZE/sizeof(fat); i++){
			if(temp->id == FREE){
				break;
			}
			temp ++;
		}
		fat1->id = i;
		fat2->id = i;
		fat1 = (fat *)(vhard + BLOCKSIZE + i * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + i * sizeof(fat));
		fat1->id = END;
		fat2->id = END;
	}else{// the text length is shorter than one block
		int i = 0;
		fat *temp = (fat *)(vhard + BLOCKSIZE);
		// find the last block
		fat1 = (fat *)(vhard + BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + openfilelist[fid].first * sizeof(fat));
		i = openfilelist[fid].first;
		while(fat1->id != END){
			if(fat1->id == FREE){
				display("fat error!\n");
                error(ERR14);
				return -1;
			}
			i = fat1->id;
			fat1 = (fat *)(vhard + BLOCKSIZE + fat1->id * sizeof(fat));
			fat2 = (fat *)(vhard + 3 * BLOCKSIZE + fat1->id * sizeof(fat));
		}
		//fill the end block
		p = (char *)(vhard + i * BLOCKSIZE);
		strcpy(p, text);
	}
	return 0;
}


int read(int fid){
	char *text = (char *)malloc(2*BLOCKSIZE);
	fcb *file;
	fat *fat1;
	int i = 0;
	int len = openfilelist[fileopenptr].length;

	if(fid > MAXOPENFILE){
		if(fileopenptr > MAXOPENFILE){
			display("file resrouce id is error , may be you have not open it");
            error(ERR15);
		}
		fid = fileopenptr;
	}
	if(openfilelist[fileopenptr].attribute != 0x5){
		display("have no opened file or the file you opened can't write!\n");
        error(ERR16);
		return -1;
	}

	if(openfilelist[fileopenptr].length < BLOCKSIZE){
		len = openfilelist[fileopenptr].length;
		text = (char *)(vhard + openfilelist[fileopenptr].first * BLOCKSIZE);
		i = 0;
		display("Read Mode | file length:%d\n", len);
		while((len --) > 0){
			putchar(text[i ++]);
		}
		display("\n");
	}else {
		int id = 0;
		len = openfilelist[fileopenptr].length;
		fat1 = (fat*)(vhard + BLOCKSIZE + openfilelist[fileopenptr].first * sizeof(fat));
		text = (char *)(vhard + openfilelist[fileopenptr].first * BLOCKSIZE);
		for(i = 0;i <= BLOCKSIZE; i++){
			putchar(text[i]);
		}
		while(fat1->id != END){
			int limit;
			if(fat1->id == FREE){
				display("fat error!\n");
                error(ERR14);
				return -1;
			}
			len -= BLOCKSIZE;
			limit = len < BLOCKSIZE?len:BLOCKSIZE;
			text = (char *)(vhard + fat1->id * BLOCKSIZE);
			for(i = 0;i<= limit; i++){
				putchar(text[i]);
			}
			fat1 = (fat *)(vhard + BLOCKSIZE + fat1->id * sizeof(fat));
		}
	}
	return 0;
}


void delfile(char *filename){
	fat * fat1, *fat2;
	fcb *file1;
	char *file2, name[100], exname[20];
	int isfind = 0;
	int i = 0;
	//check the file name
	if((!strcmp(filename, "")) || (!strcmp(filename, ".")) || (!strcmp(filename, ".."))){
		display("please input file name!\n");
        error(ERR17);
		return ;
	}
	for(i = 0; i < (int)strlen(filename) ; i ++){
		if(filename[i] == '/' 
			|| filename[i] == '\\' 
			|| filename[i] == ':' 
			|| filename[i] == '*'
			|| filename[i] == '?'
			|| filename[i] == '"'
			|| filename[i] == '<'
			|| filename[i] == '>'
			|| filename[i] == '|'){
            display("directopry name or file name can't contain / \\ : * ? \" < > | \n");
            error(ERR2);
            return ;
		}
	}
	
	file1 = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
	for(i = 0;i < BLOCKSIZE/sizeof(fcb); i ++){
		int l = strlen(file1->filename);
		strcpy(name, file1->filename);
		while(l --){
			if(name[l] == '.'){
				strcpy(exname, (char *)(name + l*sizeof(char) + 1));
				name[l] = '\0';
				break;
			}
		}
		// find 
		if((!strcmp(file1->filename, filename) || !strcmp(name, filename)) && file1->attribute == 0x5){
			isfind = 1;
			break;
		}
		file1 ++;
	}
	//delete data in fat
	fat1 = (fat *)(vhard + BLOCKSIZE + file1->first * sizeof(fat));
	fat2 = (fat *)(vhard + 3 * BLOCKSIZE + file1->first * sizeof(fat));
	while(fat1->id != END){
		int id = fat1->id;
		if(fat1->id == FREE){
			display("fat error!\n");
            error(ERR14);
			return ;
		}
		file2 = (char *)(vhard + fat1->id * BLOCKSIZE);
		memset(file2,0,BLOCKSIZE);
		fat1->id = FREE;
		fat2->id = FREE;
		fat1 = (fat *)(vhard + BLOCKSIZE + id * sizeof(fat));
		fat2 = (fat *)(vhard + 3 * BLOCKSIZE + id * sizeof(fat));
	}
	fat1->id = FREE;
	fat2->id = FREE;
	file2 = (char *)(vhard + file1->first * BLOCKSIZE);
	memset(file2,0,BLOCKSIZE);
	// delete fcb
	memset(file1, 0, sizeof(fcb));
}

void move(char *src, char *dest) {
    int srcIndex = open(src);

    if (srcIndex == -1) {
        display("Source file doesn't exist.\n");  // 源文件不存在
        error(ERR18);
        return;
    }


    // 获取源文件的路径和目标文件的路径
    char srcDir[MAXOPENFILE][80];
    int srcDirLength = 0;
    char destDir[MAXOPENFILE][80];
    int destDirLength = 0;

    int i = 0;
    int x = 0, y = 0;

    // 分割源文件路径
    while (src[i] != '\0' && i < 110) {
        if (src[i] != '/') {
            srcDir[x][y] = src[i];
            y++;
        } else {
            srcDir[x][y] = '\0';
            y = 0;
            x++;
        }
        i++;
    }
    srcDir[x][y] = '\0';
    srcDirLength = x + 1;

    i = 0;
    x = 0, y = 0;

    // 分割目标文件路径
    while (dest[i] != '\0' && i < 110) {
        if (dest[i] != '/') {
            destDir[x][y] = dest[i];
            y++;
        } else {
            destDir[x][y] = '\0';
            y = 0;
            x++;
        }
        i++;
    }
    destDir[x][y] = '\0';
    destDirLength = x + 1;

    // 切换到目标目录
    fcb *cdir = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);  // 添加 cdir 的定义
    for (i = 0; i < destDirLength-1; i++) {
        int isFind = 0;
        int limit = BLOCKSIZE / sizeof(fcb);  // 防止 cdir 超过一个块的大小
        while (limit--) {
            if ((!strcmp(cdir->filename, destDir[i])) && (cdir->attribute == 0x4)) {
                isFind = 1;
                break;
            }
            cdir++;
        }
        if (isFind == 0) {
            display("Destination directory doesn't exist.\n");  // 目标目录不存在
            close(srcIndex);
            error(ERR19);
            return;
        } else {
            cdir = (fcb *)(vhard + cdir->first * BLOCKSIZE);
        }
    }

    // 在目标目录中查找一个空闲的目录项
    int limit = BLOCKSIZE / sizeof(fcb);
    fcb *destFile = NULL;  // 添加 destFile 的定义
    while (limit--) {
        if (strcmp(cdir->filename, "") == 0) {
            destFile = cdir;
            break;
        }
        cdir++;
    }

    // 检查目标目录是否已满
    if (limit < 0 || destFile == NULL) {
        display("Destination directory is full. Cannot move file.\n");  // 目标目录已满，无法移动文件
        close(srcIndex);
        error(ERR20);
        return;
    }

    fcb *srcFile = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE + openfilelist[srcIndex].diroff * sizeof(fcb));

    // 更新目标目录项为源文件的详细信息
    strcpy(destFile->filename, destDir[x]);
    destFile->attribute = srcFile->attribute;
    destFile->date = srcFile->date;
    destFile->time = srcFile->time;
    destFile->first = srcFile->first;
    destFile->length = srcFile->length;

    // 清空源目录项
    strcpy(srcFile->filename, "");
    srcFile->attribute = 0x0;
    srcFile->date = 0;
    srcFile->time = 0;
    srcFile->first = 0;
    srcFile->length = 0;

    close(srcIndex);

    display("File moved successfully.\n");
}

void cross_check() {
    FILE *myfsys;
    block0 *boot;
    fat *fat1, *fat2;
    int totalBlocks, usedBlocks;
    float usagePercentage;

    // 打开文件系统文件
    myfsys = fopen("myfsys", "r");
    if (myfsys == NULL) {
        display("Error opening file.\n");
        error(ERR21);
        return;
    }

    // 读取引导块
    boot = (block0 *)malloc(sizeof(block0));
    fread(boot, sizeof(block0), 1, myfsys);
    fclose(myfsys);

    fat1 = (fat *)(vhard + BLOCKSIZE);
    fat2 = (fat *)(vhard + 3 * BLOCKSIZE);

    totalBlocks = boot->size / boot->blocksize;
    usedBlocks = 0;

    int cross_num[1 << 16] = {0};
    // 计算已使用的块数
    for (int i = 6; i < totalBlocks; i++) {
        cross_num[fat1->id]++;
        fat1++;
    }

    for (int i = 0; i < (1 << 16); i++) {
        if (cross_num[i] > 0) {
            display("index:%x \t\t num:%d\n", i, cross_num[i]);
        }
    }

    int index[1000] = {0};
    int num = 0;
    for (int i = 1; i < (1 << 16) - 1; i++) {
        if (cross_num[i] > 1) {
            index[num++] = i;
        }
    }

    fcb *file; // 定义指向fcb结构体的指针变量file
    char type[11]; // 定义长度为11的字符数组type
    int i = BLOCKSIZE / sizeof(fcb); // 计算每个磁盘块中fcb结构体的数量，并赋值给变量i
    char crossFiles[1000][(int)(BLOCKSIZE) / sizeof(fcb)][11];
    int T[1000] = {0};
    unsigned short file_first;
    file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE); // 计算当前目录在虚拟硬盘中的地址，并将其赋值给file

    // 遍历和删除
    fcb *ddir, *dir;
    ddir = (fcb *)(vhard + 5 * BLOCKSIZE);

    clear();
    push(ddir);
    while (!isempty()) {
        fcb *temp;
        int all = BLOCKSIZE / sizeof(fcb) - 2;
        dir = (fcb *)pop();
        temp = dir; // 父目录
        dir = (fcb *)(vhard + dir->first * BLOCKSIZE + 2 * sizeof(fcb)); // 更新为子目录
        while ((all--) > 0) {
            if (strcmp(dir->filename, "")) {
                if (dir->attribute == 0x4) {
                    // 如果是目录
                    push((void *)dir);
                } else if (dir->attribute == 0x5) {
                    // 文件
                    fat1 = (fat *)(vhard + BLOCKSIZE + dir->first * sizeof(fat));
                    while (fat1->id != END) {
                        for (int x = 0; x < 1000; x++) {
                            if (index[x] == 0)
                                break;

                            int block_index = index[x];
                            int cur_block_index = fat1->id;
                            if (cur_block_index == block_index) {
                                strcpy(crossFiles[block_index][T[block_index]++], dir->filename);
                                break;
                            }
                        }
                        fat1 = (fat *)(vhard + BLOCKSIZE + (fat1->id - 1) * sizeof(fat));
                    }
                }
            }
            dir++;
        }
    }

    for (int z = 0; z < 1000; z++) {
        if (T[z] > 0) {
            display("cross in block[%x]: ", z);
            for (int q = 0; q < BLOCKSIZE / sizeof(fcb); q++) {
                display("%s ", crossFiles[z][q]);
            }
            display("\n");
        }
    }

    free(boot);
}


void change_last_block(char *filename, unsigned short id) {
    fcb *temp;
    int isExist = 0;
    temp = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);

    // 遍历当前目录查找指定文件
    for (int i = 0; i < BLOCKSIZE / sizeof(fcb); i++) {
        if (!strcmp(temp->filename, filename)) {
            isExist = 1;
        }
        temp++;
    }
    // 文件不存在则创建
    if (isExist == 0)
        create(filename);

    fcb *file;
    fat *fat1, *lastFat;
    int limit, isFind;
    int l, off;
    char name[81];
    char exname[10];
    limit = BLOCKSIZE / sizeof(fcb);
    file = (fcb *)(vhard + ptrcurdir.first * BLOCKSIZE);
    isFind = 0;
    off = 0;

    while ((limit--) > 0) {
        // 处理文件名
        l = strlen(file->filename);
        strcpy(name, file->filename);
        while (l--) {
            if (name[l] == '.') {
                strcpy(exname, (char *)(name + l * sizeof(char) + 1));
                name[l] = '\0';
                break;
            }
        }

        // 查找指定文件
        if ((!strcmp(file->filename, filename) || !strcmp(name, filename)) && file->attribute == 0x5) {
            // 找到了
            fat1 = (fat *)(vhard + BLOCKSIZE + file->first * sizeof(fat));
            while (fat1->id != END) {
                lastFat = fat1;
                fat1 = (fat *)(vhard + BLOCKSIZE + (fat1->id - 1) * sizeof(fat));
            }
            lastFat->id = id;
            fat1->id = END;
            return;
        }

        file++;
        off++;
    }
}

