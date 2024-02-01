#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#define BOOT_SIZE 16456
#define OUTPUT_FILENAME "pack"
#define BOOT_FILENAME "boot2"
#define MAX_FILE_SIZE 65535

uint16_t *read_f(char *filename, uint16_t *buf){
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    
    if (read(fd, buf, size) <= 0){
        perror("read");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return size;
}

int main(int argc, char **argv){

    uint16_t *boot_code = malloc(MAX_FILE_SIZE);
    uint16_t *elf_code = malloc(MAX_FILE_SIZE);

    memset(boot_code, 0, MAX_FILE_SIZE);
    memset(elf_code, 0, MAX_FILE_SIZE);

    int boot_size = read_f(argv[1], boot_code);
    int elf_size = read_f(argv[2], elf_code);

    int fd_elf = open(OUTPUT_FILENAME, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
    if (fd_elf == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("size boot %d\nsize elf %d\n", boot_size, elf_size);
    uint16_t *code = malloc(boot_size+elf_size);
    
    memcpy(code, boot_code, boot_size);
    memcpy(code+boot_size, elf_code, elf_size);

    write(fd_elf, code, boot_size+elf_size);
    close(fd_elf);
    // sleep(1);

    printf("Create \"%s\"\n", "pack");
    // system ("./pack");
    return 0;

}
