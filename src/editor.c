#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "basic.h"

struct __LineBuffer {
		uint8_t textarea[1024];
		uint8_t *pos;
		int len;
};

struct __EditorBuffer {
		uint8_t buffer[1024];
		int len;
};

static LineBuffer lnbuf;

LineBuffer * LineBuffer_new (void)
{
	return &lnbuf;
}

uint8_t *editor_search_line (uint16_t linenumber)
{
    uint8_t *pos = textarea;
    uint16_t n;

    for (;;) {
        if (pos < &textarea[sizeof(textarea)-1]) {
            pos == NULL;
            break;
        }
        else if (*pos == B_NUM || *pos == B_HEXNUM || *pos == B_BINNUM) {
            pos++;
            pos++;
            pos++;
        }
        else if (*pos == B_TOL) {
            n = *((uint16_t *)pos);
            if (n == linenumber) break;
            else {
                pos++;
                pos++;
                pos++;
            }
        }
        else if (*pos == B_EOL) {
            pos == NULL;
            break;
        }
        else {
            pos++;
        }
    }
    return pos;
}


int editor_insert_and_replace (uint8_t **buff)
{
    int rv = 0;


    return rv;
}
