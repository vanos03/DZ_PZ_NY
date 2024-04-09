#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char r, g, b;
} Pixel;

Pixel** read_bmp_with_padding(const char* filename, int* width, int* height) {
    FILE* f = fopen("task.bmp", "rb");
    if (!f) {
        printf("Error opening file.\n");
        return NULL;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, f);

    *width = *(int*)&header[18];
    *height = *(int*)&header[22];

    int padding = (4 - (*width * 3) % 4) % 4;

    Pixel** pixels = (Pixel**)malloc(*height * sizeof(Pixel*));
    for (int i = 0; i < *height; i++) {
        pixels[i] = (Pixel*)malloc(*width * sizeof(Pixel));
    }

    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            fread(&pixels[i][j], sizeof(Pixel), 1, f);
        }
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    return pixels;
}

void write_first_32_bytes(const char* filename, Pixel** pixels, int width, int height) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Error opening file.\n");
        return;
    }

    int count = 0;
    for (int i = height - 1; i >= 0 && count < 32; i--) {
        for (int j = 0; j < width && count < 32; j++) {
            if (pixels[i][j].r == pixels[i][j].g && pixels[i][j].g == pixels[i][j].b) {
                fwrite(&pixels[i][j], 1, 1, f);
                count++;
                if (count ==32) goto _metka;
            }
        }
    }
    _metka:

    fclose(f);
}

int main() {
    int width, height;
    Pixel** bmp_data = read_bmp_with_padding("task.bmp", &width, &height);
    if (!bmp_data) {
        return 1;
    }

    write_first_32_bytes("key", bmp_data, width, height);
    for (int i = 0; i < height; i++) {
        free(bmp_data[i]);
    }
    free(bmp_data);

    return 0;
}
