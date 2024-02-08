
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HEAD_SIZE 0x1000
#define FREE(arg) free(arg);arg=NULL;
#define MMAP_ERR(mem) if(mem==MAP_FAILED){perror("mmap");exit(EXIT_FAILURE);}
#define MAX(x,y) (((x)>(y))?(x):(y))



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
    uint32_t max_vaddr = 0;
    for(int i =0; i < elf_hdr->e_phnum; i++){
        Elf64_Phdr *elf_seg = (Elf64_Phdr *)(elf + elf_hdr->e_phoff + (elf_hdr->e_phentsize)*i);
        max_vaddr = MAX(max_vaddr, elf_seg->p_vaddr+elf_seg->p_memsz);
    }

    void *seg_mem = mmap(0, max_vaddr, PROT_WRITE,  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    MMAP_ERR(seg_mem);
    printf("e_entry: %d\n", elf_hdr->e_entry);

    printf("\t----COUNT OF SEGMETS: %d----\n", elf_hdr->e_phnum);
    for(int i =0; i < elf_hdr->e_phnum; i++){
        printf("i = %d\n", i);
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
        // puts("---");
        // if (elf_seg->p_type == PT_LOAD){
        uint8_t *seg = malloc(elf_seg->p_memsz);
            // puts("memcpy 1");
        memcpy(seg, elf + elf_seg->p_offset, elf_seg->p_filesz);
            // printf("elf_s = : %d\taddr + mem = : %d\n", elf_size, seg_mem + elf_seg->p_vaddr + elf_seg->p_memsz);
            // puts("memcpy 2");
        memcpy(seg_mem + elf_seg->p_vaddr, seg, elf_seg->p_memsz);
            // puts("---");
            FREE(seg);
        // }
    }
    printf("\t----MPROTECT----\n");
    mprotect(seg_mem, elf_size, PROT_READ | PROT_EXEC);

    void (*func)() = (void (*)())(seg_mem + elf_hdr->e_entry);
    printf("\t----START_CODE----\n");
    func();

    printf("\t----MUNMAP----\n");
    if (munmap(seg_mem, elf_size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    FREE(elf);
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
