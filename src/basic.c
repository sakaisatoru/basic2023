#include <stdint.h>
#include <stdio.h>
#include "basic.h"

extern uint8_t *show_line (uint8_t *pos);
extern uint8_t *EditorBuffer_get_textarea (EditorBuffer *ed);

static void __dump (EditorBuffer *ed)
{
	uint8_t *pos;
	
	pos = EditorBuffer_get_textarea (ed);
	printf (" address  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n"); 
	for (int i = 0; i < 1024; i++) {
		if (i % 16 == 0) printf ("%X: ", pos);
		printf ("%02X ", *pos++);
		if (i % 16 == 15) putchar ('\n');
	}
}

 
int basic (EditorBuffer *ed, uint8_t *t)
{
    uint8_t *pos;
    while (*t != B_EOT) {
        switch (*t) {
            case B_LIST:
				//~ __dump (ed);
                pos = EditorBuffer_get_textarea (ed);
                while (*pos != B_EOT) {
                    //~ printf ("%x\n", pos);

                    if (*pos == B_TOL) {
                        ++pos;
                        printf ("%d ", *((uint16_t *)pos));
                        ++pos;++pos;
                        ++pos;
                        continue;
                    }
                    //~ printf ("%x\n", pos);
                    pos = show_line (pos);
                    //~ printf ("%x\n", pos);
                    //~ getchar();
                }
                t++;
                break;
        }
    }
}
