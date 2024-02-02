#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define N 256   // 2^8

void swap(unsigned char *a, unsigned char *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int KSA(char *key, unsigned char *S) {

    int len = strlen(key);
    int j = 0;

    for(int i = 0; i < N; i++)
        S[i] = i;

    for(int i = 0; i < N; i++) {
        j = (j + S[i] + key[i % len]) % N;

        swap(&S[i], &S[j]);
    }

    return 0;
}

int PRGA(unsigned char *S, unsigned char *plaintext, char *ciphertext) {

    int i = 0;
    int j = 0;

    for(size_t n = 0, len = strlen(plaintext); n < len; n++) {
        i = (i + 1) % N;
        j = (j + S[i]) % N;

        swap(&S[i], &S[j]);
        int rnd = S[(S[i] + S[j]) % N];

        ciphertext[n] = rnd ^ plaintext[n];

    }

    return 0;
}

int RC4(char *key, unsigned char *plaintext, char *ciphertext) {

    unsigned char S[N];

    KSA(key, S);
    PRGA(S, plaintext, ciphertext);

    return 0;
}

int main(int argc, char* argv[]) {

    FILE* fin = fopen(argv[2], "rb");
    FILE* fout = fopen(argv[3], "w");

    fseek(fin, 0l, SEEK_END);
    int s = ftell(fin);
    fseek(fin, -s, SEEK_END);
    printf("%d\n", s);


    if (fin != NULL) {
        unsigned char *ciphertext = malloc(s);
        unsigned char *txt = malloc(s);

            fread(txt, sizeof(char), s, fin);
            txt[s + 1] = '\0';
            //printf("%s\n", txt);
            RC4(argv[1], txt, ciphertext);
            fwrite(ciphertext, sizeof(char), s, fout);


        fclose(fin);

        fclose(fout);
        //free(ciphertext);
    }
    else {
        printf("%s\n", "error");
    }

    return 0;
}
