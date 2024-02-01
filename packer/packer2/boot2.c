#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#define OFFSET 16456
#define FILENAME "elf"

int main(int argc, char **argv){
    int fd = open(argv[0], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    int elf_size = lseek(fd, 0, SEEK_END) ;
    lseek(fd, OFFSET, SEEK_SET);

    uint8_t *elf_code = malloc(elf_size);
    memset(elf_code, 0, elf_size);

    if (read(fd, elf_code, elf_size) <= 0){
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(fd);

    int fd_elf = open(FILENAME, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
    if (fd_elf == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }    

    printf("Create \"%s\"", FILENAME);
    if (write(fd_elf, elf_code, elf_size) <= 0){
        perror("write");
        close(fd_elf);
        exit(EXIT_FAILURE);
    }
    close(fd_elf);
    sleep(1);
    // system ("./elf");
    return 0;

}