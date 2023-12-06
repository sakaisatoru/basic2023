#include <stdint.h>
#include <stdio.h>
#include "basic.h"

extern int show_line (uint8_t **pos);
extern uint8_t *EditorBuffer_get_textarea (EditorBuffer *ed);

int basic (EditorBuffer *ed, uint8_t *t)
{
    uint8_t *pos;
    while (*t != B_EOT) {
        switch (*t) {
            case B_LIST:
                pos = EditorBuffer_get_textarea (ed);
                while (*pos != B_EOT) {
                    if (*pos == B_TOL) {
                        pos++;
                        printf ("%d ", *((uint16_t *)pos));
                        pos++;pos++;
                        pos++;
                        continue;
                    }
                    show_line (&pos);
                }
                break;
        }
    }
}
