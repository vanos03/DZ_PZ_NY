
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

int execute_elf(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    int elf_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char *elf_code = malloc(elf_size);
    memset(elf_code, 0, elf_size);

    if (read(fd, elf_code, elf_size) != elf_size){
        // perror("read");
        // exit(EXIT_FAILURE);
    }
    
    void *elf_mem = mmap(0, elf_size, \
        PROT_READ | PROT_EXEC | PROT_WRITE | MAP_ANONYMOUS, MAP_PRIVATE, fd, 0);
    
    if (elf_mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // while(1){
    //     char buf[10];
    //     memcpy(buf, elf_code, 10);
    //     printf("%s", buf);
    //     elf_code+=10;
    // }


    mprotect(elf_mem, elf_size, PROT_READ | PROT_EXEC);
    
    Elf64_Ehdr *elf_header = (Elf64_Ehdr *)elf_mem;
    // Elf64_Addr entry_point = elf_header->e_entry+elf_mem;
    Elf64_Addr entry_point = elf_mem + 4096;

    // printf("точка входа в программу: %x\
    // начало выделенной области памяти: %x\
    // размер эльф файла: %d\n",\
    // entry_point, elf_mem, elf_size);

    void (*func)() = (void (*)())(entry_point);
    func();

    if (munmap(elf_mem, elf_size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <elf-файл>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    execute_elf(argv[1]);

    return 0;
}
