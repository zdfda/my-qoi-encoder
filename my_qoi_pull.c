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
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_pels;
    int32_t  y_pels;
    uint32_t clr_used;
    uint32_t clr_important;
}c;
struct pass_head
{
    char magic[4];
    int32_t width;
    int32_t height;
    int32_t offset;
    uint16_t bit_count;
}h;
#pragma pack(pop)

void pass_out(const char* path)
{
    int fd=open(path,O_RDONLY);
    if(fd<0){perror("open");return;}
    read(fd,&h,sizeof(h));
    if(h.magic[0]!='Z'||h.magic[1]!='C'||h.magic[2]!='J'||h.magic[3]!='1')
    {
        printf("magic error\n");close(fd);return;
    }
    int w=h.width;
    int bh=h.height;
    if(bh<0){bh=-bh;}
    char buf[64]={0};
    sprintf(buf,"%s.bmp",path);
    int fd2=open(buf,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(fd2<0){perror("open out");close(fd);return;}
    int row=(3*w+3)/4*4;
    c.type=0x4D42;
    c.size=row*bh+54;
    c.reserved1=0;
    c.reserved2=0;
    c.offset=54;
    c.size2=40;
    c.width=w;
    c.height=h.height;
    c.planes=1;
    c.bit_count=24;
    c.compression=0;
    c.image_size=row*bh;
    c.x_pels=0;
    c.y_pels=0;
    c.clr_used=0;
    c.clr_important=0;
    write(fd2,&c,sizeof(c));

    uint8_t last_buf[3]={0};
    uint8_t ax_buf[16][3]={0};
    uint8_t t;
    uint16_t t2;
    int Bz,Gz,Rz,n,m;

    for(int j=0;j<bh;j++)
    {
        uint8_t out_row[row];
        memset(out_row,0,row);
        int cnt=0;
        int i=0;
        while(i<w)
        {
            read(fd,&t,1);
            if(t==0b00100000)
            {
                read(fd,&last_buf[0],1);
                read(fd,&last_buf[1],1);
                read(fd,&last_buf[2],1);
                out_row[3*i]=last_buf[0];
                out_row[3*i+1]=last_buf[1];
                out_row[3*i+2]=last_buf[2];
                i++;
                ax_buf[cnt][0]=last_buf[0];
                ax_buf[cnt][1]=last_buf[1];
                ax_buf[cnt][2]=last_buf[2];
                cnt++;
                cnt=cnt%16;
            }
            else if((t&0b11000000)==0b11000000)
            {
                if(t==0b11111111){n=64;}
                else{n=(t&0b00111111)+1;}
                for(int k=0;k<n;k++)
                {
                    out_row[3*i]=last_buf[0];
                    out_row[3*i+1]=last_buf[1];
                    out_row[3*i+2]=last_buf[2];
                    i++;
                }
            }
            else if((t&0b11000000)==0b01000000)
            {
                Bz=((t>>4)&0b11)-2;
                Gz=((t>>2)&0b11)-2;
                Rz=(t&0b11)-2;
                last_buf[0]=last_buf[0]+Bz;
                last_buf[1]=last_buf[1]+Gz;
                last_buf[2]=last_buf[2]+Rz;
                out_row[3*i]=last_buf[0];
                out_row[3*i+1]=last_buf[1];
                out_row[3*i+2]=last_buf[2];
                i++;
                ax_buf[cnt][0]=last_buf[0];
                ax_buf[cnt][1]=last_buf[1];
                ax_buf[cnt][2]=last_buf[2];
                cnt++;
                cnt=cnt%16;
            }
            else if((t&0b11000000)==0b10000000)
            {
                uint8_t b2;
                read(fd,&b2,1);
                t2=((uint16_t)t<<8)|b2;
                Bz=((t2>>10)&0b00011111)-8;
                Gz=((t2>>5)&0b00011111)-16;
                Rz=(t2&0b00011111)-16;
                last_buf[0]=last_buf[0]+Bz;
                last_buf[1]=last_buf[1]+Gz;
                last_buf[2]=last_buf[2]+Rz;
                out_row[3*i]=last_buf[0];
                out_row[3*i+1]=last_buf[1];
                out_row[3*i+2]=last_buf[2];
                i++;
                ax_buf[cnt][0]=last_buf[0];
                ax_buf[cnt][1]=last_buf[1];
                ax_buf[cnt][2]=last_buf[2];
                cnt++;
                cnt=cnt%16;
            }
            else if((t&0b11110000)==0)
            {
                m=t&0b00001111;
                last_buf[0]=ax_buf[m][0];
                last_buf[1]=ax_buf[m][1];
                last_buf[2]=ax_buf[m][2];
                out_row[3*i]=last_buf[0];
                out_row[3*i+1]=last_buf[1];
                out_row[3*i+2]=last_buf[2];
                i++;
                ax_buf[cnt][0]=last_buf[0];
                ax_buf[cnt][1]=last_buf[1];
                ax_buf[cnt][2]=last_buf[2];
                cnt++;
                cnt=cnt%16;
            }
        }
        write(fd2,out_row,row);
    }
    close(fd);
    close(fd2);
}

int main(int argc,char* argv[])
{
    if(argc!=2){printf("./... path");return 0;}
    pass_out(argv[1]);
}