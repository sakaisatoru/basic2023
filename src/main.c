#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "basic.h"

int main (int ac, char *av[])
{
    LineBuffer *ln;
    EditorBuffer *ed;

    ed = EditorBuffer_new ();
    ln = LineBuffer_new ();
    EditorBuffer_start_message (ed);
    LineBuffer_console (ln, ed);

    return 0;
}
