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

extern LineBuffer *LineBuffer_new (void);
extern int LineBuffer_console (LineBuffer *ln, EditorBuffer *ed);
extern uint8_t *LineBuffer_get_midbuffer (LineBuffer *ln);
extern EditorBuffer *EditorBuffer_new (void);

extern uint8_t basic_word[];
extern uint8_t operator_word[];



uint8_t linebuff[256];
uint8_t textbuff[128];

int main (int ac, char *av[])
{
    uint8_t *text, *pos;
    int rv;
    LineBuffer *ln;
    EditorBuffer *ed;

    //~ if (ac < 2) {
        //~ puts ("usage:\nbasic text");
        //~ return 0;
    //~ }
    ed = EditorBuffer_new ();
    ln = LineBuffer_new ();
    rv = LineBuffer_console (ln, ed);

exit_this:
    return 0;
}

// gcc -o basic main.c token.c
