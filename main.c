#include <stdio.h>
#include <stdint.h>
#include "basic.h"

extern uint8_t token (uint8_t **text);
extern void __putch (uint8_t c);
extern uint8_t *code2word (uint8_t code);
extern void put_basic_word (uint8_t *s, void(*__putc)(uint8_t));

int main (int ac, char *av[])
{
    uint8_t **text;

    text = &av[1];
    uint8_t n = token (text);

    printf ("result : %02X\n", n);

    uint8_t i, *s;
    for (i = 0xa0; i <= 0xbf; i++) {
        if (i % 8 == 0) putchar ('\n');
        s = code2word (i);
        if (s == NULL) break;
        printf ("\t%02X ",i);
        put_basic_word (s, __putch);
    }

    return 0;
}

// gcc -o basic main.c token.c
