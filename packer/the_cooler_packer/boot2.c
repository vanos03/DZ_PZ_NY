
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#define PAGE_SIZE 0x1000
#define FREE(arg) free(arg);arg=NULL;
#define CHECK_MMAP_ERR(mem) if(mem==MAP_FAILED){perror("mmap");exit(EXIT_FAILURE);}
#define PRINT_ERR(ERR) do{fprintf(stderr, ERR);exit(EXIT_FAILURE);}while(0);

#define PAGE_ALIGN  (1<<12)
#define ROUNDUP(x,b) (((unsigned long)x+b-1)&(~(b-1)))
#define PAGEUP(x) ROUNDUP(x, PAGE_ALIGN)
#define ROUNDDOWN(x,b) ((unsigned long)x&(~(b-1)))
#define PAGEDOWN(x) ROUNDDOWN(x, PAGE_ALIGN)


int alight_to_page_size(int size){
    while(size % 4096 != 0) size++;
    return size;
}

int execute_elf(char *filename, char **argv) {
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1) PRINT_ERR("Failed open file\n");

    uint32_t elf_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    uint8_t *elf = malloc(elf_size);

    if (read(fd, elf, elf_size) <= 0) PRINT_ERR("Failed read file\n");
    
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *) elf;
    Elf64_Phdr *phdr = (Elf64_Phdr *) (elf + ehdr->e_phoff);

    if (ehdr->e_type != ET_EXEC) PRINT_ERR("File type not ET_EXEC\n");
    
    for(size_t i = 0; i < ehdr->e_phnum; i++, phdr++) {
        if (phdr->p_type == PT_LOAD){
            void *seg_addr = NULL;
            void *load_addr = NULL;
            unsigned long round_len = 0;
            unsigned int seg_flags = 0;

            load_addr = (void*)PAGEDOWN(phdr->p_vaddr);
            round_len = phdr->p_memsz + (phdr->p_vaddr % PAGE_SIZE);
            round_len = PAGEUP(round_len);

            seg_addr = mmap(load_addr, round_len, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
            CHECK_MMAP_ERR(seg_addr)

            load_addr = (void*)phdr->p_vaddr;
            printf("i = %d\nload_addr = %x\n", i, load_addr);
            memcpy(load_addr, elf + phdr->p_offset, phdr->p_filesz);

            if(phdr->p_flags & PF_R) seg_flags |= PROT_READ;
            if(phdr->p_flags & PF_W) seg_flags |= PROT_WRITE;
            if(phdr->p_flags & PF_X) seg_flags |= PROT_EXEC;

            mprotect(seg_addr, round_len, seg_flags);

            // printf(" \
            // type: 0x%x\n \
            // число байт занимаемое сегментом в файле: 0x%x\n \
            // число байт занимаемое сегментом в памяти: 0x%x\n \
            // виртуальный адрес, по которому располагается первый байт сегмента в памяти: 0x%x\n \
            // смещение от начала файла, по которому располагается первый байт сегмента: 0x%x\n\n\n", \
            //                 phdr->p_type, \
            //                 phdr->p_filesz, \
            //                 phdr->p_memsz, \
            //                 phdr->p_vaddr, \
            //                 phdr->p_offset
            //                 );
        }
    }
    void *start_addr = (void*)ehdr->e_entry;
    printf("start_addr = %x\n", start_addr);
    
    #define SET_STACK(sp) __asm__ __volatile__ ("movq %0, %%rsp"::"r"(sp))
    #define JMP(addr) __asm__ __volatile__ ("jmp *%0"::"r"(addr))

    SET_STACK(argv-1);
    JMP(start_addr);


    // uint32_t max_vaddr = 0;
    // for(int i =0; i < elf_hdr->e_phnum; i++){
    //     Elf64_Phdr *elf_seg = (Elf64_Phdr *)(elf + elf_hdr->e_phoff + (elf_hdr->e_phentsize)*i);
    //     max_vaddr = MAX(max_vaddr, elf_seg->p_vaddr+elf_seg->p_memsz);
    // }

    // void *seg_mem = mmap(0, max_vaddr, PROT_WRITE,  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    // MMAP_ERR(seg_mem);
    // printf("e_shnum: %d\ne_shoff: 0x%x\ne_shsize: %d\ne_shstrndx: 0x%x\ne_entry: 0x%x\ne_type: %d\n", 
    //     elf_hdr->e_shnum, elf_hdr->e_shoff, elf_hdr->e_shentsize, elf_hdr->e_shstrndx, elf_hdr->e_entry, elf_hdr->e_type);

    // printf("\t----COUNT OF SEGMETS: %d----\n", elf_hdr->e_phnum);
    // for(int i = 0; i < elf_hdr->e_phnum; i++){
    //     printf("segment #%d\n", i);
    //     Elf64_Phdr *elf_seg = (Elf64_Phdr *)(elf + elf_hdr->e_phoff + (elf_hdr->e_phentsize)*i);

    //     printf(" \
    //         type: 0x%x\n \
    //         число байт занимаемое сегментом в файле: 0x%x\n \
    //         число байт занимаемое сегментом в памяти: 0x%x\n \
    //         виртуальный адрес, по которому располагается первый байт сегмента в памяти: 0x%x\n \
    //         смещение от начала файла, по которому располагается первый байт сегмента: 0x%x\n\n\n", \
    //                         elf_seg->p_type, \
    //                         elf_seg->p_filesz, \
    //                         elf_seg->p_memsz, \
    //                         elf_seg->p_vaddr, \
    //                         elf_seg->p_offset
    //                         );

    //     if (elf_seg->p_type == PT_LOAD){
            
    //         // printf("alight size: %d\n", alight_to_page_size(elf_seg->p_memsz));
    //         // void *seg_mem = mmap(elf_seg->p_vaddr, alight_to_page_size(elf_seg->p_memsz), PROT_WRITE,  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    //         // MMAP_ERR(seg_mem);

    //         uint8_t *seg = malloc(elf_seg->p_memsz);
    //         memcpy(seg, elf + elf_seg->p_offset, elf_seg->p_filesz);
    //         memcpy(seg_mem, seg, elf_seg->p_memsz);
    //         // mprotect(seg_mem, elf_size, PROT_READ | PROT_EXEC);

    //         FREE(seg);
    //     }
    // }
    // printf("\t----MPROTECT----\n");
    // mprotect(seg_mem, elf_size, PROT_READ | PROT_EXEC);

    // void (*func)() = (void (*)())(elf_hdr->e_entry);
    // printf("\t----START_CODE----\n");
    // func();

    // // printf("\t----MUNMAP----\n");
    // // if (munmap(seg_mem, elf_size) == -1) {
    // //     perror("munmap");
    // //     exit(EXIT_FAILURE);
    // // }

    FREE(elf);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <elf-файл>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    execute_elf(argv[1], argv);

    return 0;
}
