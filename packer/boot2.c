
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HEAD_SIZE 0x1000


int execute_elf(char *filename) {
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    uint32_t elf_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    uint8_t *elf = malloc(elf_size);

    if (read(fd, elf, elf_size) <= 0){
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    Elf64_Ehdr *elf_hdr = (Elf64_Ehdr *)elf;

    printf("----COUNT OF SEGMETS: %d----\n", elf_hdr->e_phnum);
    for(int i =0; i < elf_hdr->e_phnum; i++){
        Elf64_Phdr *elf_seg = (Elf64_Phdr *)(elf + elf_hdr->e_phoff + (elf_hdr->e_phentsize)*i);

        printf(" \
        type: 0x%x\n \
        число байт занимаемое сегментом в файле: 0x%x\n \
        число байт занимаемое сегментом в памяти: 0x%x\n \
        виртуальный адрес, по которому располагается первый байт сегмента в памяти: 0x%x\n \
        смещение от начала файла, по которому располагается первый байт сегмента: 0x%x\n\n\n", \
                        elf_seg->p_type, \
                        elf_seg->p_filesz, \
                        elf_seg->p_memsz, \
                        elf_seg->p_vaddr, \
                        elf_seg->p_offset
                        );
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
