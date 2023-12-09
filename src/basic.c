#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include "basic.h"

extern uint8_t *show_line (uint8_t *pos);
extern uint8_t *EditorBuffer_get_textarea (EditorBuffer *ed);
extern uint8_t *editor_search_line (EditorBuffer *ed, uint16_t linenumber, uint8_t *pos, int *cdx);

extern int16_t _var[26];

enum {
    STACK_TYPE_NONE = 0x00,
    STACK_TYPE_FOR = 1,
    STACK_TYPE_GOSUB,
};
typedef struct {
            uint8_t type;       // スタックを使っている構文
            uint8_t var;        // 変数（添字）
            int16_t value;      // FOR のカウンター終了値
            uint8_t *address;   // 分岐先アドレス
        } STACK;
#define STACKSIZE   16
STACK stack[STACKSIZE];
int stackpointer;

 void __dump (uint8_t *pos, int16_t bytes)
{
    uint8_t *c;

    c = pos;
    printf (" address  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");
    //~ for (int i = 0; i < bytes; i++) {
    for (int i = 0; i < bytes; i++) {
        if (i % 16 == 0) printf ("%X: ", pos);
        printf ("%02X ", *pos++);
        if (i % 16 == 15) {
            printf ("  ");
            for (int j = 0; j < 16; j++) {
                putchar ((*c >= 0x20 && *c <= 0x7f)? *c:'.');
                c++;
            }
            c = pos;
            putchar ('\n');
        }
    }
}

/*
 * スタックが有効であれば 0を返す
 */
int stack_check (void)
{
    return (stackpointer < 0) ? -1 :
        (stackpointer >= STACKSIZE) ? 1: 0;
}

STACK *stack_tos (void)
{
    return (stackpointer < 0) ? NULL : &stack[stackpointer];
}

STACK *stack_push (uint8_t type, uint8_t var, int16_t value, uint8_t *address)
{
    stackpointer++;
    if (stackpointer >= STACKSIZE) return NULL;
    stack[stackpointer].type = type;
    stack[stackpointer].var  = var;
    stack[stackpointer].value   = value;
    stack[stackpointer].address = address;
    return &stack[stackpointer];
}

STACK *stack_pop (void)
{
    return (stackpointer < 0) ? NULL: &stack[stackpointer--];
}

int basic (EditorBuffer *ed, uint8_t *t)
{
    uint8_t *pos, c, *currtop, currlen, *jmp ;
    int16_t n, n1, currline;
    STACK *sp;

    currtop = NULL; // 実行中の行の先頭
    currlen = 0;    // 実行中の行の長さ
    currline = 0;

    stackpointer = -1;
    __dump (t, 64);
    while (*t != B_EOT) {
        switch (*t++) {
            case B_RETURN:
                if (stack_check ()) {
                    puts ("return without gosub.");
                    return 0;
                }
                sp = stack_tos ();
                if (sp->type != STACK_TYPE_GOSUB) {
                    puts ("return without gosub.");
                    return 0;
                }
                sp = stack_pop ();
                t = sp->address;
                continue;

            case B_GOSUB:
                puts ("GOSUB!");
                if (*t != B_NUM) {
                    puts ("syntax error.");
                    return 0;
                }
                if (stack_check () > 0) {
                    puts ("stack over flow.");
                    return 0;
                }
                puts ("hoge");
                t++;
                n = *((int16_t*)t);
                t++;
                t++;
                jmp = editor_search_line (ed, (uint16_t)n,
                                                        ((currline < n)? t : NULL), &n1);
                if (n1 == 1) {
                    puts ("undefined line.");
                    return 0;
                }
                stack_push (STACK_TYPE_GOSUB, 0, 0, t);
                t = jmp;
                continue;

            case B_GOTO:
                puts ("GOTO!");
                if (*t != B_NUM) {
                    puts ("syntax error.");
                    return 0;
                }
                t++;
                n = *((int16_t*)t);
                t++;
                t++;
                jmp = editor_search_line (ed, (uint16_t)n,
                                                        ((currline < n)? t : NULL), &n1);
                if (n1 == 1) {
                    puts ("undefined line.");
                    return 0;
                }
                t = jmp;
                continue;

            case B_NEXT:
                if (stack_check ()) {
                    puts ("next without for.");
                    return 0;
                }
                sp = stack_tos ();
                if (sp->type != STACK_TYPE_FOR) {
                    puts ("next without for.");
                    return 0;
                }
                if (_var[sp->var] >= sp->value) {
                    stack_pop ();
                }
                else {
                    _var[sp->var]++;
                    t = sp->address;
                }
                continue;

            case B_FOR:
                if (stack_check () > 0) {
                    puts ("stack over flow.");
                    return 0;
                }
                if (*t != B_VAR) {
                    puts ("syntax error.\n");
                    return 0;
                }
                t++;
                if (!isalpha(*t)) {
                    puts ("syntax error.\n");
                    return 0;
                }
                c = (*t-'A');
                t++;
                if (*t == B_EQ2) {
                    t++;
                    n = expression (&t, B_TO);
                }
                else {
                    puts ("syntax error.\n");
                    return 0;
                }
                n1 = expression (&t, 0);
                stack_push (STACK_TYPE_FOR, c, n1, t);
                _var[c] = n;
                continue;

            case B_EOT:
                break;

            case B_TOL:
                // 行番号
                t--;
                currtop = t;
                t++;
                currline = *((int16_t *)t);
                t++;
                t++;
                currlen = *t++;
                continue;

            case B_IF:
                n = expression (&t, B_THEN);
                if (n == 0) {
                    if (currtop == NULL) {
                        // ダイレクトモードを終わる
                        return 0;
                    }
                    t = currtop + currlen;
                }
                continue;

            case B_LET:
                if (*t == B_VAR) {
                    continue;
                }
                puts ("syntax error.\n");
                return 0;

            case B_VAR:
                // LET省略
                if (!isalpha(*t)) {
                    puts ("syntax error.\n");
                    return 0;
                }
                c = (*t-'A');
                t++;
                if (*t == B_EQ2) {
                    t++;
                    n = expression (&t, 0);
                    _var[c] = n;
                    continue;
                }
                puts ("syntax error.\n");
                return 0;

            case B_COLON:
            case B_SEMICOLON:
                break;

            case B_PRINT:
                for (;;) {
                    if (*t == B_COMMA) {
                        t++;
                        continue;
                    }
                    else if (*t == B_SEMICOLON) {
                            break;
                    }
                    else if (*t == B_COLON ||
                             *t == B_TOL ||
                             *t == B_EOT) {
                            putchar ('\n');
                            break;
                    }
                    else if (*t == B_STR) {
                            t++;
                            while (*t != B_EOT && *t != B_TOL) {
                                if (*t == B_STR) {
                                    t++;
                                    break;
                                }
                                putchar (*t++);
                            }
                    }
                    else {
                        n = expression (&t, 0);
                        printf ("%d",n);
                    }
                }
                break;

            case B_LIST:
                //~ __dump (EditorBuffer_get_textarea (ed), 64);  getchar();
                pos = EditorBuffer_get_textarea (ed);
                while (*pos != B_EOT) {
                    if (*pos == B_TOL) {
                        ++pos;
                        printf ("%d ", *((uint16_t *)pos));
                        ++pos;++pos;
                        ++pos;
                        continue;
                    }
                    pos = show_line (pos);
                }
                break;
        }
    }
}
