#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(){
    unsigned int key[8];
    unsigned int buf = 0;
    unsigned int buf_size = 0;
    unsigned int block = 0;
    unsigned int block_encr = 0;
    unsigned int a = 134775813;
     
    

    int fd = open("sample_key.dat", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1; 
    }
    if (read(fd, key, 32) <= 0) {
        perror("read");
        close(fd); 
        return -1; 
    }
    close(fd);

    int fd_in = open("sample.txt", O_RDONLY);
    if (fd_in == -1) {
        perror("open");
        return -1; 
    }

    int fd_out = open("out", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        perror("open");
        return -1; 
    }

    int i = 0;
    int count_read = 0;
    printf("key[0]: %x\n", key[0]);
    // key[0] = 0x4b617370;
    while((count_read = read(fd_in, &block_encr, sizeof(unsigned int))) > 0){
        if(i == 0){
            block = key[i % 8] ^ 0;
        }
        else{
            block ^= key[i % 8];
        }

        block = block*134775813+1;
        block_encr ^=block;

        write(fd_out, &block_encr, sizeof(unsigned int));
        i++;
        printf("block: %x block_crypt: %x read: %d\n", block, block_encr, count_read);
    }
    close(fd_in);
    close(fd_out);

    return 0;

}