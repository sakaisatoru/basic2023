#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "basic.h"

extern uint8_t token (uint8_t **text);
extern void __putch (uint8_t c);
extern uint8_t *code2word (uint8_t code);
extern void put_basic_word (uint8_t *s, void(*__putc)(uint8_t));
extern int16_t get_number (uint8_t **text);

int main (int ac, char *av[])
{
    uint8_t **text;

    if (ac < 2) {
        puts ("usage:\nbasic text");
        return 0;
    }

    text = &av[1];

    uint8_t n;
    int16_t d;
    while ((n = token (text)) != 0) {
        printf ("%02X ", n);
        switch (n) {
            case B_VAR:
                putchar (**text);
                ++*text;
                break;

            case B_NUM:
                d = get_number (text);
                printf ("[%d]",d);
                break;

            case B_STR:
                do {
                    putchar (**text);
                    ++*text;
                }while (**text != '\"');
                ++*text;
                break;

            default:
                break;
        }
        putchar ('\n');
    }

    uint8_t i, *s;
    for (i = 0xa0; i <= 0xbf; i++) {
        if (i % 4 == 0) putchar ('\n');
        s = code2word (i);
        if (s == NULL) break;
        printf ("%02X ",i);
        put_basic_word (s, __putch);
        putchar ('\t');if (strlen(s)<=5) putchar ('\t');
    }
    putchar ('\n');

    return 0;
}

// gcc -o basic main.c token.c
