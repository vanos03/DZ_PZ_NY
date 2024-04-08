#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>

#define FILENAME "task.bmp"

// struct bmp_header{
//     uint16_t format;
//     uint32_t file_size;
//     uint32_t bf_reserved;
//     uint32_t start_off;
//     int bi_size;
//     int bi_width;
//     int bi_height;
// } bmp_hdr;



int main(){
    int fd = open(FILENAME, O_RDONLY);

    uint32_t file_size;
    uint32_t start_off;
    uint32_t width;
    uint32_t height;

    lseek(fd, 2, SEEK_SET);
    read(fd, &file_size, 4);

    lseek(fd, 10, SEEK_SET);
    read(fd, &start_off, 4);

    lseek(fd, 18, SEEK_SET);
    read(fd, &width, 4);

    lseek(fd, 22, SEEK_SET);
    read(fd, &height, 4);

    int size = height*(width)*3;
    uint8_t buf[size];

    printf("%x %d %d %d\n", start_off, file_size, width, height);
    int rd = 0;

    lseek(fd, start_off, SEEK_SET);
   
    rd = read(fd, buf, size);

    uint8_t forwr[32];
    int j=0;
    // width++;

    int fd_out = open("key", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        perror("open");
        return -1; 
    }

    int curr_wr = 0;

    for (int i = height-1; i > height-5; i--){
        for (int j = 0; j < (width*3)+1; j+=3){
            
            // printf("%x", buf[i*(width*3) + j]);
            if ((buf[i*(width*3)+j] == buf[i*(width*3)+1+j]) && (buf[i*(width*3)+j] == buf[i*(width*3)+2+j])){
                forwr[curr_wr] = buf[i*(width*3)+j];
                write(fd_out, &buf[i*(width*3)+j], 1);
                curr_wr++; 
                if (curr_wr==32) goto _metka;
            }
        }
    }

    _metka:
    printf("\n");
     for (int i = 0; i < 32; i++)
        printf("%x ", forwr[i]);
    printf("\n");

    close(fd);
    close(fd_out);

}