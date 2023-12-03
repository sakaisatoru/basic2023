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
extern int show_line (uint8_t **pos);
extern int str2mid (uint8_t **text, uint8_t *buff, int buffsize);

extern uint8_t basic_word[];
extern uint8_t operator_word[];



uint8_t linebuff[256];
uint8_t textbuff[128];

int main (int ac, char *av[])
{
    uint8_t *text, *pos;
    int rv;

    if (ac < 2) {
        puts ("usage:\nbasic text");
        return 0;
    }
    text = &av[1];

    for (;;) {
        fgets (linebuff, sizeof(linebuff)-1, stdin);
        pos = strchr (linebuff, '\n');
        if (pos != NULL) *pos = '\0';
        else linebuff[sizeof(linebuff)-1] = '\0';
        //~ printf ("debuf : %s\n",linebuff);
        text = linebuff;
        if (rv = str2mid (&text, textbuff, sizeof(textbuff))) {
            // error
            if (rv == 1) {
                printf ("buffer overflow.");
            }
            else {
                printf ("undefined word.");
            }
            break;
        }

        //~ putchar ('\n');
        //~ pos = textbuff;
        //~ while (*pos != B_EOL) {
            //~ printf ("%02X ", *pos++);
        //~ }
        //~ putchar ('\n');

        pos = textbuff;
        if (*pos == B_NUM) {
            // 先頭が数値だったら編集モードにはいる
            if (editor_insert_and_replace (&pos)) {
                printf ("out of memory");
            }
            break;
        }

        if (show_line (&pos)) {
            puts ("undefind function");
            break;
        }
    }
exit_this:
    return 0;
}

// gcc -o basic main.c token.c
