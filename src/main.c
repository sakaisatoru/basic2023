#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "basic.h"

extern uint8_t token (uint8_t **text);
extern void __putch (uint8_t c);
extern uint8_t *code2word (uint8_t code, uint8_t topcode, uint8_t *s);
extern void put_basic_word (uint8_t *s, void(*__putc)(uint8_t));
extern int16_t get_number (uint8_t **text);

extern uint8_t basic_word[];
extern uint8_t operator_word[];


uint8_t textarea[1024];

int main (int ac, char *av[])
{
    uint8_t **text, *pos;

    if (ac < 2) {
        puts ("usage:\nbasic text");
        return 0;
    }

    text = &av[1];
    pos = textarea;

    uint8_t n;
    int16_t d;
    while ((n = token (text)) != 0) {
        *pos++ = n;
        //~ printf ("%02X ", n);
        switch (n) {
            case B_VAR:
                *pos++ = **text;
                //~ putchar (**text);
                ++*text;
                break;

            case B_NUM:
            case B_HEXNUM:
            case B_BINNUM:
                d = get_number (text);
                *((int16_t *)pos) = d;
                pos++;
                pos++;
                //~ printf ("[%d]",d);
                break;

            case B_STR:
                do {
                    *pos++ = **text;
                    //~ putchar (**text);
                    ++*text;
                }while (**text != '\"');
                *pos++ = B_STR;
                ++*text;
                break;

            default:
                break;
        }
        if (pos >= &textarea[1023]) {
            puts ("out of memory.");
            break;
        }
    }
    *pos = B_EOL;

    uint8_t *s;
    pos = textarea;

    while (*pos != B_EOL){
        switch (*pos) {
            case B_HEXNUM:
                pos++;
                printf ("0X%X", *((uint16_t *)pos));
                pos++;
                break;
            case B_BINNUM:
                pos++;
                uint16_t b = *((uint16_t *)pos);
                printf ("0B");
                for (int i=0; i<16; i++) {
                    putchar ((b & 0x8000) ? '1':'0');
                    b <<= 1;
                }
                pos++;
                break;
            case B_NUM:
                pos++;
                printf ("%d", *((uint16_t *)pos));
                pos++;
                break;

            case B_STR:
                pos++;
                putchar ('\"');
                while (*pos != B_STR){
                    putchar (*pos++);
                }
                putchar ('\"');
                break;

            case B_REMARK:
                pos++;
                putchar ('\'');
                while (*pos != B_EOL){
                    putchar (*pos++);
                }
                break;

            case B_VAR:
                pos++;
                putchar (*pos);
                break;

            case B_COMMA:
                putchar (',');
                break;
            case B_COLON:
                putchar (':');
                break;
            case B_SEMICOLON:
                putchar (';');
                break;

            default:
                s = code2word (*pos, B_BREAK, basic_word);
                if (s != NULL) {
                    putchar (' ');
                    put_basic_word (s, __putch);
                    putchar (' ');
                    break;
                }

                s = code2word (*pos, B_NEG,   operator_word);
                if (s != NULL) {
                    put_basic_word (s, __putch);
                    break;
                }

                printf ("%02X : undefind code.", *pos);
                goto exit_this;
        }
        pos++;
    }
    putchar ('\n');
exit_this:
    return 0;
}

// gcc -o basic main.c token.c
