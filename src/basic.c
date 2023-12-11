#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "basic.h"


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
static STACK stack[STACKSIZE];
static int stackpointer;

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
    stack[stackpointer].type    = type;
    stack[stackpointer].var     = var;
    stack[stackpointer].value   = value;
    stack[stackpointer].address = address;
    return &stack[stackpointer];
}

STACK *stack_pop (void)
{
    return (stackpointer < 0) ? NULL: &stack[stackpointer--];
}

/*
 * 指定された中間コードを探す
 */
uint8_t *basic_search_word (uint8_t c, uint8_t *t)
{
    while (*t != c) {
        switch (*t) {
            case B_EOT:         // テキスト末端
                t = NULL;
                goto exit_this;
            case B_TOL:         // 行番号 (４進める）
                t++;
            case B_NUM:         // 数値 (３進める）
            case B_BINNUM:
            case B_HEXNUM:
                t++;
            case B_VAR:         // 変数（２進める）
                t++;
            default:            //　１進める
                t++;
                break;
        }
    }
exit_this:
    return t;
}

/*
 * BASICインタープリタ本体
 * エラーコードを返す
 */
int16_t basic (EditorBuffer *ed, uint8_t *t)
{
    uint8_t *pos, c, *jmp, *tmp;
    int16_t n, n1;
    int16_t e, f;
    uint16_t start, end;
    STACK *sp;

    ed->currtop = NULL; // 実行中の行の先頭
    ed->currlen = 0;    // 実行中の行の長さ
    ed->currline = 0;

    stackpointer = -1;
    //~ __dump (t, 64);
    while (*t != B_EOT) {
        switch (*t++) {
            default:
                return B_ERR_SYNTAX_ERROR;

            case B_REMARK:
                if (ed->currtop != NULL) {
                    t = ed->currtop + ed->currlen;
                    continue;
                }
                return B_ERR_SYNTAX_ERROR;  // direct modeでのremarkはエラー

            case B_READ:
                if (*t != B_VAR) return B_ERR_SYNTAX_ERROR;
                if (ed->readnext == NULL) {
                    tmp = basic_search_word (B_DATA, ed->textarea);
                    if (tmp == NULL) return B_ERR_NO_DATA;
                    tmp++;
                    ed->readnext = tmp;
                }
                c = (*t - 'A');
                t++;
                tmp = ed->readnext;
                __dump (tmp, 64);
                n = expression (&tmp, B_COMMA, &e);
                if (e) return e;
                if (*tmp == B_COMMA) tmp++;
                ed->readnext = tmp;
                _var[c] = n;
                break;

            case B_NEW:
                if (ed->currtop != NULL) {
                    // 実行中に呼び出された
                    return B_ERR_ILLEAGAL_FUNCTION_CALL;
                }
                EditorBuffer_new ();
                stackpointer = -1;
                return B_ERR_NO_ERROR;

            case B_END:
                return B_ERR_NO_ERROR;

            case B_STOP:
                return 0;

            case B_RUN:
                t = EditorBuffer_get_textarea (ed);
                ed->currtop = t;
                ed->currline = *((int16_t*)(t+1));
                ed->currlen = *(t+3);
                continue;

            case B_RETURN:
                if (stack_check ()) {
                    return B_ERR_RETURN_WITHOUT_GOSUB;
                }
                sp = stack_tos ();
                if (sp->type != STACK_TYPE_GOSUB) {
                    return B_ERR_RETURN_WITHOUT_GOSUB;
                }
                sp = stack_pop ();
                t = sp->address;
                continue;

            case B_GOSUB:
                if (*t != B_NUM) {
                    return B_ERR_SYNTAX_ERROR;
                }
                if (stack_check () > 0) {
                    return B_ERR_STACK_OVER_FLOW;
                }
                t++;
                n = *((int16_t*)t);
                t++;
                t++;
                jmp = EditorBuffer_search_line (ed, (uint16_t)n,
                                ((ed->currline == 0)? NULL: /* direct mode */
                                (ed->currline < n)? t : NULL), &f);
                if (f == 1) {
                    return B_ERR_UNDEFINED_LINE;
                }
                stack_push (STACK_TYPE_GOSUB, 0, 0, t);
                t = jmp;
                continue;

            case B_GOTO:
                if (*t != B_NUM) {
                    return B_ERR_SYNTAX_ERROR;
                }
                t++;
                n = *((int16_t*)t);
                t++;
                t++;
                jmp = EditorBuffer_search_line (ed, (uint16_t)n,
                                ((ed->currline == 0)? NULL: /* direct mode */
                                (ed->currline < n)? t : NULL), &f);
                if (f == 1) {
                    return B_ERR_UNDEFINED_LINE;
                }
                t = jmp;
                continue;

            case B_NEXT:
                if (stack_check ()) {
                    return B_ERR_NEXT_WITHOUT_FOR;
                }
                sp = stack_tos ();
                if (sp->type != STACK_TYPE_FOR) {
                    return B_ERR_NEXT_WITHOUT_FOR;
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
                    return B_ERR_STACK_OVER_FLOW;
                }
                if (*t != B_VAR) {
                    return B_ERR_SYNTAX_ERROR;
                }
                t++;
                if (!isalpha(*t)) {
                    return B_ERR_SYNTAX_ERROR;
                }
                c = (*t-'A');
                t++;
                if (*t == B_EQ2) {
                    t++;
                    n = expression (&t, B_TO, &e);
                    if (e) return e;
                }
                else {
                    return B_ERR_SYNTAX_ERROR;
                }
                n1 = expression (&t, 0, &e);
                if (e) return e;
                stack_push (STACK_TYPE_FOR, c, n1, t);
                _var[c] = n;
                continue;

            case B_EOT:
                break;

            case B_TOL:
                // 行番号
                t--;
                ed->currtop = t;
                t++;
                ed->currline = *((int16_t *)t);
                t++;
                t++;
                ed->currlen = *t++;
                continue;

            case B_IF:
                n = expression (&t, B_THEN, &e);
                if (e) return e;
                if (n == 0) {
                    if (ed->currtop == NULL) {
                        // ダイレクトモードを終わる
                        return B_ERR_NO_ERROR;
                    }
                    t = ed->currtop + ed->currlen;
                }
                continue;

            case B_LET:
                if (*t == B_VAR) {
                    continue;
                }
                return B_ERR_SYNTAX_ERROR;

            case B_VAR:
                // LET省略
                if (!isalpha(*t)) {
                    return B_ERR_SYNTAX_ERROR;
                }
                c = (*t-'A');
                t++;
                if (*t == B_EQ2) {
                    t++;
                    n = expression (&t, 0, &e);
                    if (e) return e;
                    _var[c] = n;
                    continue;
                }
                return B_ERR_SYNTAX_ERROR;

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
                        n = expression (&t, 0, &e);
                        if (e) return e;
                        printf ("%d",n);
                    }
                }
                break;

            case B_LIST:
                start = 0;
                end = 65535;
                if (*t == B_NUM) {
                    t++;
                    start = *((uint16_t*)t);
                    t++;
                    t++;
                    if (*t != B_COMMA) {
                        end = start;
                    }
                }
                if (*t == B_COMMA) {
                    t++;
                    if (*t == B_NUM) {
                        t++;
                        end = *((uint16_t*)t);
                        t++;
                        t++;
                    }
                }
                pos = EditorBuffer_search_line (ed, start, NULL, &f);
                //~ __dump (pos, 64);getchar();
                while (*pos != B_EOT) {
                    if (*pos == B_TOL) {
                        ++pos;
                        start = *((uint16_t *)pos);
                        if (start > end) break;
                        printf ("%d ", start);
                        ++pos;++pos;
                        ++pos;
                        continue;
                    }
                    pos = show_line (pos);
                    //~ getchar();
                }
                break;
        }
    }
    return B_ERR_NO_ERROR;
}
