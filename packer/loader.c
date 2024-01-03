#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

typedef void (*EntryPoint)();

void executeElf(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Определение размера файла
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Чтение содержимого файла в память
    void *mem = mmap(NULL, size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fd, 0);
    if (mem == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Закрытие файла после отображения в память
    close(fd);

    // Выполнение ELF-файла
    EntryPoint entryPoint = (EntryPoint)mem;
    entryPoint();

    // Отключение отображения в память после выполнения
    munmap(mem, size);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <elf_filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    executeElf(argv[1]);

    return 0;
}
