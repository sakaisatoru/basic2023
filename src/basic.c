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
 * *f = 1  行末（次行先頭）で打ち切る
 *
 * 戻り
 *  *f = 0  見つかった   !0 見つからない
 */
uint8_t *basic_search_word (uint8_t c, uint8_t *t, int16_t *f)
{
    while (*t != c) {
        switch (*t) {
            case B_EOT:         // テキスト末端
                *f = !0;
                goto exit_this;
            case B_TOL:         // 行番号 (４進める）
                if (*f) {
                    *f != 0;
                    goto exit_this;
                }
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
    *f = 0;
exit_this:
    return t;
}

static EditorBuffer *inter_ed;
int16_t _basic_free_area (void)
{
    return (int16_t)inter_ed->last;
}

int16_t basic_load_intelhex (EditorBuffer *ed)
{
    FILE *fp;
    if ((fp = fopen ("file.bas", "rt")) == NULL) {
        return B_ERR_IO_ERROR;
    }

    fclose (fp);
    return B_ERR_NO_ERROR;
}

int16_t basic_save_intelhex (EditorBuffer *ed)
{
    uint8_t *pos;
    uint16_t i, ad = 0, sum, len;
    FILE *fp;

    pos = ed->textarea;
    len = sizeof(ed->textarea) - ed->last;
    //~ printf ("ADDRESS : %04X  LEN : %02d\n", pos, len);

    if ((fp = fopen ("file.bas", "wt")) == NULL) {
        return B_ERR_IO_ERROR;
    }

    while (len > 0) {
        fprintf (fp, ":");
        //~ printf (":");
        if (len < 16) {
            // 最終データレコード
            sum = len + (ad >> 8) + ad & 0xff;
            fprintf (fp, "%02X%04X00", len, ad);
            //~ printf ("%02X%04X00", len, ad);
            //~ printf ("%02X %04X 00 ", len, ad);
            while (len-- > 0) {
                sum += *pos;
                fprintf (fp, "%02X", *pos++);
                //~ printf ("%02X", *pos++);
                //~ printf ("%02X ", *pos++);
            }
            fprintf (fp, "%02X\n", 0xff & (0-sum));
            //~ printf ("%02X\n", 0xff & (0-sum));
            break;
        }
        else {
            sum = 16 + ad >> 8 + ad & 0xff;
            fprintf (fp, "10%04X00", ad);
            //~ printf ("10%04X00", ad);
            //~ printf ("10 %04X 00 ", ad);
            for (i = 0; i < 16; i++) {
                sum += *pos;
                fprintf (fp, "%02X", *pos++);
                //~ printf ("%02X", *pos++);
                //~ printf ("%02X ", *pos++);
            }
            fprintf (fp, "%02X\n", 0xff & (0-sum));
            //~ printf ("%02X\n", 0xff & (0-sum));
            len -= 16;
            ad += 16;
        }
    }
    fprintf (fp, ":00000001FF\n");
    //~ printf (":00000001FF\n");

    fclose (fp);
    return B_ERR_NO_ERROR;
}


/*
 * ON n GOSUB, GOTO, RESTORE用サブルーチン
 * n で指定された行番号を得る
 * 戻り
 *  行番号列の最後
 *  linenum 該当する行番号、ない場合は0
 */
static uint8_t *basic_skip_number (uint8_t *t, int16_t num, uint16_t *linenum)
{
    uint16_t n = 0;
    int16_t f;
    f = 1;
    for (;;) {
        if (num <= 0) {
            t = basic_search_word (B_COLON, t, &f);
            break;
        }

        if (*t != B_NUM) {
            break;
        }

        num--;
        t++;
        if (num == 0) n = *((int16_t*)t);
        t++;
        t++;
        if (*t != B_COMMA) break;
        t++;
    }
    *linenum = n;
    return t;
}

/*
 * BASICインタープリタ本体
 * エラーコードを返す
 */
int16_t basic (EditorBuffer *ed, uint8_t *t)
{
    uint8_t *pos, c, *jmp, *tmp;
    int16_t n, n1, e, f, onflag, start, end;
    STACK *sp;

    static uint8_t tmpbuf[64], midtmp[32];  // INPUT用バッファ

    if (ed->currline == 0) {
        // 実行環境の初期化
        ed->currtop = NULL; // 実行中の行の先頭
        ed->currlen = 0;    // 実行中の行の長さ
        //~ ed->currline = 0;
        ed->readnext = NULL;// DATA 文の先頭

        stackpointer = -1;  // 要検討
        onflag      = 0;    // ON n GOTO,GOSUB,RESTORE用flag
    }

    inter_ed = ed;  // 外部からの参照用

    //~ __dump (t, 64);
    while (*t != B_EOT) {
        ed->currpos = t;
        switch (*t++) {
            default:
                return B_ERR_SYNTAX_ERROR;

            case B_LOAD:
                if (ed->currtop != NULL) return B_ERR_ILLEAGAL_FUNCTION_CALL;
                e = basic_load_intelhex (ed);
                if (e) return e;
                continue;

            case B_SAVE:
                if (ed->currtop != NULL) return B_ERR_ILLEAGAL_FUNCTION_CALL;
                e = basic_save_intelhex (ed);
                if (e) return e;
                continue;

            case B_ON:
                onflag = expression (&t, 0, &e);
                if (onflag < 1) onflag = 1;
                if (*t != B_GOSUB &&
                    *t != B_GOTO && *t != B_RESTORE) return B_ERR_SYNTAX_ERROR;
                continue;

            case B_INPUT:
                if (ed->currtop == NULL) return B_ERR_ILLEAGAL_FUNCTION_CALL;
                if (*t == B_STR) {
                    // show prompt
                    t++;
                    while (*t != B_STR && *t != B_TOL && *t != B_EOT) putchar (*t++);
                    if (*t != B_STR) return B_ERR_SYNTAX_ERROR;
                    t++;
                }
                if (*t != B_COMMA) return B_ERR_SYNTAX_ERROR;
                t++;
                if (*t != B_VAR)   return B_ERR_SYNTAX_ERROR;
                t++;
                c = *t++ - 'A';
                for (;;) {
                    fgets (tmpbuf, sizeof(tmpbuf)-1,stdin);
                    tmp = tmpbuf;
                    n1 = str2mid (&tmp, midtmp, sizeof(midtmp));
                    if (n1 < 0) {
                        return B_ERR_SYNTAX_ERROR;
                    }
                    tmp = midtmp;
                    n = expression (&tmp, 0, &e);
                    if (e) {
                        printf ("??INPUT\n");
                    }
                    else {
                        _var[c] = n;
                        break;
                    }
                }
                continue;

            case B_REMARK:
                if (ed->currtop != NULL) {
                    t = ed->currtop + ed->currlen;
                    continue;
                }
                return B_ERR_SYNTAX_ERROR;  // direct modeでのremarkはエラー

            case B_DATA:
                // 行末まで飛ばす
                f = 1;
                t = basic_search_word (B_COLON, t, &f);
                continue;

            case B_RESTORE:
                if (onflag) {
                    t = basic_skip_number (t, onflag, &n);
                    onflag = 0;
                    if (n == 0) continue;   // 該当する行番号がない
                }
                else {
                    if (*t != B_NUM) {
                        return B_ERR_SYNTAX_ERROR;
                    }
                    t++;
                    n = *((int16_t*)t);
                    t++;
                    t++;
                }
                jmp = EditorBuffer_search_line (ed, (uint16_t)n,
                                ((ed->currline == 0)? NULL: /* direct mode */
                                (ed->currline < n)? t : NULL), &f);
                if (f) return B_ERR_UNDEFINED_LINE;
                f = 0;
                ed->readnext = basic_search_word (B_DATA, jmp, &f);
                if (!f) {
                    ed->readnext++;
                }
                else {
                    ed->readnext = NULL;
                }
                continue;

            case B_READ:
                t--;
                do {
                    if (ed->readnext == NULL) return B_ERR_NO_DATA;
                    t++;
                    if (*t != B_VAR) return B_ERR_SYNTAX_ERROR;
                    t++;
                    c = (*t++ - 'A');
                    tmp = ed->readnext;
                    n = expression (&tmp, 0, &e);
                    if (e) return e;
                    if (*tmp == B_COMMA) {
                        tmp++;
                    }
                    else {
                        // 次のDATA文を探す
                        f = 0;
                        tmp = basic_search_word (B_DATA, tmp, &f);
                        if (!f) {
                            tmp++;
                        }
                        else {
                            tmp = NULL;
                        }
                    }
                    ed->readnext = tmp;
                    _var[c] = n;
                } while (*t == B_COMMA);
                continue;

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

            case B_RUN:
                t = EditorBuffer_get_textarea (ed);
                ed->currtop = t;
                ed->currline = *((int16_t*)(t+1));
                ed->currlen = *(t+3);
                f = 0;
                ed->readnext = basic_search_word (B_DATA, ed->currtop, &f);
                if (!f) {
                    ed->readnext++;
                }
                else {
                    ed->readnext = NULL;
                }
                continue;

            case B_STOP:
                ed->breakpoint = t;
                return B_ERR_BREAK_IN;

            case B_CONT:
                if (ed->currline == 0) {
                    // break in 以外ではエラー
                    return B_ERR_ILLEAGAL_FUNCTION_CALL;
                }
                t = ed->breakpoint;
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
                if (stack_check () > 0) {
                    return B_ERR_STACK_OVER_FLOW;
                }
                if (onflag) {
                    t = basic_skip_number (t, onflag, &n);
                    onflag = 0;
                    if (n == 0) continue;   // 該当する行番号がない
                }
                else {
                    if (*t != B_NUM) {
                        return B_ERR_SYNTAX_ERROR;
                    }
                    t++;
                    n = *((int16_t*)t);
                    t++;
                    t++;
                }
                jmp = EditorBuffer_search_line (ed, (uint16_t)n,
                                ((ed->currline == 0)? NULL: /* direct mode */
                                (ed->currline < n)? t : NULL), &f);
                if (f == 1) {
                    //~ __dump (t, 64);
                    return B_ERR_UNDEFINED_LINE;
                }
                stack_push (STACK_TYPE_GOSUB, 0, 0, t);
                t = jmp;
                continue;

            case B_GOTO:
                if (onflag) {
                    t = basic_skip_number (t, onflag, &n);
                    onflag = 0;
                    if (n == 0) continue; // 該当する行番号がない
                }
                else {
                    if (*t != B_NUM) {
                        return B_ERR_SYNTAX_ERROR;
                    }
                    t++;
                    n = *((int16_t*)t);
                    t++;
                    t++;
                }
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
                continue;

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
                t--;
                t--;
                n = expression (&t, 0, &e); // 単行演算子の処理
                if (e) return e;
                continue;

            case B_COLON:
            case B_SEMICOLON:
                continue;

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
                continue;

            case B_LIST:
                start = 0;
                end = 32767;
                if (*t == B_NUM) {
                    t++;
                    start = *((int16_t*)t);
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
                        end = *((int16_t*)t);
                        t++;
                        t++;
                    }
                }
                pos = EditorBuffer_search_line (ed, start, NULL, &f);
                n = 0;
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
                    if (!(++n & 7)) {
                        printf ("-- press enter key ---");getchar ();
                    }
                }
                continue;
        }
    }
    return B_ERR_NO_ERROR;
}
