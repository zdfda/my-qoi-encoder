#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#pragma pack(push,1)
struct bmp
{
    uint16_t type;          
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
    uint32_t size2;          
    int32_t  width;
    int32_t  height;        
    uint16_t planes;
    uint16_t bit_count;
}c;
int write_n=0;
char write_buf[512]={0};
void my_write(int fd,const void* buf,size_t n)
{
    if(write_n+n>=512){write(fd,write_buf,write_n);write_n=0;}
    memcpy(write_buf+write_n,buf,n);
    write_n+=n;
}
void bmp_pass(const char* path)
{
    int fd=open(path,O_RDONLY);
    char buf[64]={0};
    sprintf(buf,"%s.pass",path);
    if(fd<0){perror("open");return;}
    lseek(fd,0,SEEK_SET);
    read(fd,&c,sizeof(c));
    if(c.type!= 0x4D42){printf("file in't bmp\n");close(fd);return;}
    if(c.bit_count<24){printf("bit_count less\n");close(fd);return;}
    int fd2=open(buf,O_WRONLY|O_CREAT,0777);
    if (fd2<0){perror("open out");close(fd);return;}
    my_write(fd2,"ZCJ1",4);//前四个type
    char* a=(char*)&c.width;
    my_write(fd2,a,4);//表宽
    a=(char*)&c.height;
    my_write(fd2,a,4);//表长
    a=(char*)&c.offset;
    my_write(fd2,a,4);//表偏移量
    a=(char*)&c.bit_count;
    my_write(fd2,a,2);//表位深
    lseek(fd,c.offset,SEEK_SET);
    //开始处理
    if(c.bit_count==24)
    {
        uint8_t buf[c.width][3];
        uint8_t b=4-(3*c.width%4);
        if(b==4){b=0;}
        uint8_t last_buf[3]={0};
        uint8_t ax_buf[16][3]={0};
        int32_t n=-1;
        uint8_t t;
        uint16_t t2;
        for(int j=0;j<abs(c.height);j++)
        {
            read(fd,buf,sizeof(buf));
            int cnt=0;
            for(int i=0;i<c.width;i++)
            {

                int Bz=buf[i][0]-last_buf[0];int Gz=buf[i][1]-last_buf[1];int Rz=buf[i][2]-last_buf[2];
                if(buf[i][0]==last_buf[0]&&buf[i][1]==last_buf[1]&&buf[i][2]==last_buf[2])
                {
                    n++; 
                    continue;
                }
                if(n!=-1)
                {      
                    t=0b11111111;
                    while(n>0b111111){n-=0b1000000;my_write(fd2,&t,1);}
                    t=0b11000000+n;
                    my_write(fd2,&t,1);
                    n=-1;
                }
                if(((Bz>-3)&&Bz<2)&&((Gz>-3)&&Gz<2)&&((Rz>-3)&&Rz<2))
                {
                    t=0b01000000+(Rz+2)+(Gz+2)*4+(Bz+2)*16;
                    my_write(fd2,&t,1);
                }
                else if(((Bz>-9)&&Bz<8)&&((Gz>-17)&&Gz<16)&&((Rz>-17)&&Rz<16))//一般蓝色在三原色中比较平缓,给4bit,其他5bit
                { 
                    t2=0b1000000000000000+Rz+16+(Gz+16)*32+(Bz+8)*1024;
                    t2=((t2>>8)&(0xFF))|((t2<<8)&(0xFF00));//大小端转化
                    my_write(fd2,&t2,2);
                }
                else
                {
                    int m=0;
                    for(;m<16;m++)
                    {
                        if((ax_buf[m][0]==buf[i][0])&&(ax_buf[m][1]==buf[i][1])&&(ax_buf[m][2]==buf[i][2]))
                        {
                            break;
                        }
                    }
                    if(m==16)//全部没有命中
                    {
                        t=0b00100000;
                        my_write(fd2,&t,1);
                        my_write(fd2,&buf[i][0],1);
                        my_write(fd2,&buf[i][1],1);
                        my_write(fd2,&buf[i][2],1);
                    }
                    else{t=0b00000000+m;my_write(fd2,&t,1);}
                }
                last_buf[0]=buf[i][0];last_buf[1]=buf[i][1];last_buf[2]=buf[i][2];
                ax_buf[cnt][0]=last_buf[0];ax_buf[cnt][1]=last_buf[1];ax_buf[cnt][2]=last_buf[2];
                cnt++;
                cnt=cnt%16;
            }
            if(n!=-1)
            {      
                t=0b11111111;
                while(n>0b111111){n-=0b1000000;my_write(fd2,&t,1);}
                t=0b11000000+n;
                my_write(fd2,&t,1);
                n=-1;
            }
            lseek(fd,b,SEEK_CUR);
        }
    }
    if (write_n > 0) { write(fd2, write_buf, write_n); write_n = 0; }
    long bmp_end=lseek(fd,0,SEEK_END);
    long pass_end=lseek(fd2,0,SEEK_END);
    printf("原BMP文件:%ld字节(%.2fMB)\n",bmp_end,bmp_end/1024.0/1024.0);
    printf("现:%ld字节(%.2fMB)\n",pass_end,pass_end/1024.0/1024.0);
    printf("压缩比%.2f,节省空间%.2f%%\n",(double)bmp_end/pass_end,(double)(bmp_end-pass_end)/bmp_end*100);
    close(fd2);
    close(fd);
}    

int main(int argc,char* argv[])
{
    if(argc!=2){printf("./...  path");return 0;}
    bmp_pass(argv[1]);
}