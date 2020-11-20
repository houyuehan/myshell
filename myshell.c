#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#define MAX_CMD 100
#define MAX_DIR_NAME 100
#define MAXX 100

void eval(char *cmdstring)
{
    char *argv[MAX_CMD];
    char buf[MAX_CMD];
    strcpy(buf,cmdstring);
    //
    parseline(buf,argv);
    if (argv[0]==NULL)
    {
        return ;
    }
    if (buildin_command(argv)) return;
    int pid=fork();
    if(pid==0)
    {
        if (execvp(argv[0],argv)<0)
        {
            printf("%s:command error.\n",argv[0]);
            exit(0);
        }
    }
    wait(pid);
}
//根据空格分割字符串
int argc;
int parseline(char *buf,char**argv)
{
    while(*buf==' ')
    {
        buf++;
    }

    int delim=0;
    argc=0;
    while(*buf!='\n')
    {
        while(buf[delim]!='\n' && buf[delim]!=' ')
        {
            delim++;
        }
        if (buf[delim]=='\n')
        {
            buf[delim]='\0';
            argv[argc++]=buf;
            break;
        }
        buf[delim]='\0';
        argv[argc++]=buf;

        buf+=delim+1;
        delim=0;
        while(*buf==' ')
        {
            buf++;
        }
    }
    argv[argc]=NULL;
    return 0;
}


//bulidin_command函数判断命令是否是内置命令,如果是,则执行.不是则返回false.
//**argv为二维数组
int buildin_command(char **argv)
{
//pwd 查看当前工作目录
/*strcmp函数是string compare(字符串比较)的缩写，
*用于比较两个字符串并根据比较结果返回整数。
*基本形式为strcmp(str1,str2)，若str1=str2，则返回零；
*若str1<str2，则返回负数；若str1>str2，则返回正数。
*/
    if (strcmp(argv[0],"pwd")==0)
    {
        char buf[MAX_DIR_NAME];
	//getcwd()会将当前工作目录的绝对路径复制到参数buf所指的内存空间中,第二个参数为buffer的空间大小。
        printf("%s\n",getcwd(buf,sizeof(buf)));
        return 1;
    }
//cd切换工作目录
    if (strcmp(argv[0],"cd")==0)
    {
	//chdir(const char *path); 是C语言中的一个系统调用函数（同cd），用于改变当前工作目录，其参数为Path 目标目录，可以是绝对目录或相对目录。
	//成功返回0，失败返回1
        if (chdir(argv[1])==0)
        {
            printf("修改成功\n");
        }
        else
        {
            printf("myselous:cd:%s\n",argv[1]);
        }
        return 1;
    }

    if (strcmp(argv[0],"exit")==0)
    {
        exit(0);
    }
//echo输出
    if (strcmp(argv[0],"echo")==0)
    {
        int x=1;
	//argc为输入的字符串的个数，依次将echo后面的字符串打印
        while (x<argc)
        {
            printf("%s ",argv[x]);
            x++;
        }
	//puts()函数用来向标准输出设备（屏幕）输出字符串并换行
        puts("");
        return 1;
    }
//mkdir创建新目录
    if (strcmp(argv[0],"mkdir")==0)
    {
	//缺少目录名
        if(argc<=1)
        {
            printf("mkdir:Usage:dirstr\n");
            exit(1);
        }
        char data[MAXX];
        if ((strcmp(argv[1],".")==0) || strcmp(argv[1],"/")==0)
            return ;
	//access(const char * pathname, int mode)，检查调用进程是否可以对指定的文件执行某种操作。
	//成功执行时，返回0，失败返回-1。
        if (access(argv[1],F_OK)==0)
        {
            return ;
        }
        else
        {
	    //字符串复制。把c复制到a:strcpy(a,c);
	    //目标目录+
            strcpy(data,argv[1]);
	    //获取上层目录
            dirname(argv[1]);
        }
	//如果失败返回错误提示
        if (mkdir(data,0777)==-1)
        {
            printf("mkdir error");
            exit(1);
        }
        return 1;
    }
//cat查看文件内容
    if (strcmp(argv[0],"cat")==0)
    {
	//指向文件结构
        FILE *fp;
	    //cat后有参数
            while(--argc>0)
            {
		//打开文件
                if ((fp=fopen(*++argv,"r"))==NULL)
                {
		    printf("no such file %s",*argv);
                    return 1;
                }
                else
                {
                    int c;
		    //int getc(FILE *stream)从文件读取字符
	            //直到输入缓存中的内容为空时停止读取
                    while ((c=getc(fp))!=EOF)
                        putc(c,stdout);
		    //关闭文件
                    fclose(fp);
                }
            }
        //}
        return 1;
    }
//cp复制文件
    if (strcmp(argv[0],"cp")==0)
    {
        if (argc<3)
        {
            printf("传入参数少，应输入./main filename destname\n");
            return 1;
        }
        int fd;
        int buf_fd;
        int buf_size;
        char buf_addr[100]= {0};
        buf_fd=open(argv[1],O_RDWR);//读写打开
	//lseek移动文件读写位置lseek(int fildes, off_t offset, int whence);
	//SEEK_SET 参数offset 即为新的读写位置.
        //SEEK_CUR 以目前的读写位置往后增加offset 个位移量.
    	//SEEK_END 将读写位置指向文件尾后再增加offset 个位移量. 当whence 值为SEEK_CUR 或SEEK_END 时, 参数offet 允许负值的出现.
	//计算缓冲区大小
        buf_size=lseek(buf_fd,0,SEEK_END);
//printf("buf_size is :%d\n",buf_size);
        lseek(buf_fd,-buf_size,SEEK_CUR);
	//read()读出原缓冲区内容到目的缓冲区
	//read(int fd, void * buf, size_t count);
	//read()会把参数fd所指的文件传送count 个字节到buf 指针所指的内存中。
        read(buf_fd,buf_addr,buf_size);
//printf("buf_addr is :%s\n",buf_addr);
	//打开新的文件，拥有全部控制权限
        fd=open(argv[2],O_CREAT|O_RDWR|O_TRUNC);
        if (fd==1)
        {
            printf("failed");
            return 1;
        }
        else
        {
	    //write (int fd, const void * buf, size_t count); 
	    //write()会把参数buf所指的内存写入count个字节到参数fd所指的文件内。
            write(fd,buf_addr,buf_size);
            printf("复制成功\n");
        }
	//关闭文件流
        close(fd);
        close(buf_fd);
        return 1;
    }
//rm删除文件
    if (strcmp(argv[0],"rm")==0)
    {
        if (argc!=2)
            printf("wrong\n");
	//remove删除文件
        if (remove(argv[1])!=0)
	    //error(s) 用来将上一个函数发生错误的原因输出到标准设备(stderr)。
            perror("remove"),exit(-1);
        printf("remove %s success\n",basename(argv[1]));
        return 1;
    }

}

//一个死循环,一直在等待这用户输入命令.主要的工作都在eval这个函数里面.
int main (int argc,char *argv[])
{
    char cmdstring[MAX_CMD];
    int n;
    while(1)
    {
	//在 prinf（）；后加上fflush(stdout); 强制马上输出，避免错误。
        printf("myshell>");
        fflush(stdout);
	//read
        if ((n=read(0,cmdstring,MAX_CMD))<0)
        {
            printf("error");
        }
        eval(cmdstring);
    }
    return 0;
}


