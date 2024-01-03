
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

void execute_elf(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }

    void *elf_data = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (elf_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Определение точки входа (entry point) из ELF-заголовка
    Elf64_Ehdr *elf_header = (Elf64_Ehdr *)elf_data;
    Elf64_Addr entry_point = elf_header->e_entry;

    // Закрытие файлового дескриптора, так как данные уже загружены в память
    close(fd);

    // Выполнение программы, начиная с точки входа
    void (*func)() = (void (*)())entry_point;
    func();

    // Отмена отображения файла из памяти
    if (munmap(elf_data, file_stat.st_size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <elf-файл>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    execute_elf(argv[1]);

    return 0;
}
