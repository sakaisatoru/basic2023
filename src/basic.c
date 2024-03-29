/*
 * basic.c
 *
 * Copyright 2023 endeavor wako <endeavor2wako@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include "basic.h"


extern int16_t _var[26];

enum {
    STACK_TYPE_NONE = 0x00,
    STACK_TYPE_FOR = 1,
    STACK_TYPE_GOSUB,
};
typedef struct {
            uint8_t type;       // スタックを使っている構文
            //~ uint8_t var;        // 変数（添字）
            int16_t *var_p;     // 変数（格納先）
            int16_t value;      // FOR のカウンター終了値
            uint8_t *address;   // 分岐先アドレス
        } STACK;
#define STACKSIZE   16
static STACK stack[STACKSIZE];
static int stackpointer;

void basic_printf (uint8_t *fmt, ...)
{
    va_list ap;
    int16_t keta = 0, n;
    uint8_t(*__putnum_sub_)(int16_t *n) = NULL;

    va_start (ap, fmt);

    if (fmt != NULL) {
        while (*fmt != '\0') {
            if (*fmt == '%') {
                fmt++;
                if (*fmt == '%') {
                    putchar (*fmt++);
                    continue;
                }
                if (*fmt == '+') {
                    keta |= PUTNUM_SIGN;
                    fmt++;
                }
                if (*fmt == '0') {
                    keta |= PUTNUM_ZERO;
                    fmt++;
                }
                keta |= get_number (&fmt,10);

                switch (*fmt) {
                    default:
                        fmt++;
                        continue;
                    case 'c':
                        n = (int16_t)va_arg (ap, int);
                        putchar ((int8_t)(n & 0xff));
                        fmt++;
                        continue;
                    case 'd':
                        __putnum_sub_ = __putnum_sub_dec;
                        break;
                    case 'x':
                        __putnum_sub_ = __putnum_sub_hex;
                        break;
                    case 'b':
                        __putnum_sub_ = __putnum_sub_bin;
                        break;
                }
                n = (int16_t)va_arg (ap, int);
                basic_putnum (n, keta, __putnum_sub_);
                fmt++;
            }
            else {
                putchar (*fmt++);
            }
        }
    }
exit_this:
    va_end (ap);
}


void basic_puts (uint8_t *pos)
{
    while (*pos != '\0') putchar (*pos++);
}


uint8_t __putnum_sub_dec (int16_t *n)
{
    uint8_t c = '0' + *n % 10;
    *n = *n / 10;
    return c;
}

uint8_t __putnum_sub_hex (int16_t *n)
{
    uint8_t c = *n & 0xf;
    c += ((c > 9)? 'A'-10:'0');
    *n = *n >> 4;
    return c;
}

uint8_t __putnum_sub_bin (int16_t *n)
{
    uint8_t c = ('0' + (*n & 0x1));
    *n = *n >> 1;
    return c;
}

/*
 * 数値書式制御つき表示
 *
 * %[符号][ゼロフィル]表示桁数
 *  符号 + あるいは省略
 *  ゼロフィル 上位桁を 0 で埋める
 *  表示桁数
 * 例） PRINT "%+08d", 12345  -> +0012345
 */
void basic_putnum (int16_t n, int16_t keta, uint8_t(*__putnum_sub)(int16_t *n))
{
    uint8_t buf[18], *p;
    int16_t f = 0, ctrl, minus;

    ctrl = (keta & 0xff00);
    keta = (keta & 0x00ff);
    if (keta > 0) f = 1;
    keta--;
    if (keta > sizeof(buf)-2) keta = sizeof(buf)-2;
    buf[keta+1]='\0';
    if (n == (int16_t)32768) n = 0;
    minus = (n == 0)? 0 : (n < 0)? -1:1;
    if (minus == -1) n *= -1;

    do {
        buf[keta--] = __putnum_sub (&n);
    } while (n != 0 && keta >= 0);
    if (f && (ctrl & PUTNUM_ZERO)) {
        while (keta >= 0) {
            buf[keta--] = '0';
        }
        if (minus == -1 || (ctrl & PUTNUM_SIGN)) keta++;
    }
    if (minus == -1) {
        buf[keta--] = '-';
    } else if (ctrl & PUTNUM_SIGN) {
        buf[keta--] = '+';
    }
    while (f && keta >= 0) buf[keta--] = ' ';
    keta++;
    p = &buf[keta];
    while (*p != '\0') putchar (*p++);
}


void __dump (uint8_t *pos, int16_t bytes)
{
    uint8_t *c;

    c = pos;
    basic_puts (" address  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");
    for (int i = 0; i < bytes; i++) {
        if (i % 16 == 0) {
            basic_printf ("%x: ", pos);
        }
        basic_printf ("%02x ", *pos++);
        if (i % 16 == 15) {
            basic_puts ("  ");
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

STACK *stack_push (uint8_t type, int16_t *var, int16_t value, uint8_t *address)
{
    stackpointer++;
    if (stackpointer >= STACKSIZE) return NULL;
    stack[stackpointer].type    = type;
    stack[stackpointer].var_p   = var;
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
        if (len < 16) {
            // 最終データレコード
            sum = len + (ad >> 8) + ad & 0xff;
            fprintf (fp, "%02X%04X00", len, ad);
            while (len-- > 0) {
                sum += *pos;
                fprintf (fp, "%02X", *pos++);
            }
            fprintf (fp, "%02X\n", 0xff & (0-sum));
            break;
        }
        else {
            sum = 16 + ad >> 8 + ad & 0xff;
            fprintf (fp, "10%04X00", ad);
            for (i = 0; i < 16; i++) {
                sum += *pos;
                fprintf (fp, "%02X", *pos++);
            }
            fprintf (fp, "%02X\n", 0xff & (0-sum));
            len -= 16;
            ad += 16;
        }
    }
    fprintf (fp, ":00000001FF\n");

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
 * 変数の格納位置をポインタで返す
 */
int16_t *basic_search_variable_address (uint8_t **t, int16_t *e)
{
	//~ uint8_t *t = t0;
	int16_t *r = NULL;

    int16_t n1, e0, *p;
    uint8_t c;

	
    if (**t == B_VAR) {
        ++*t;
        c = **t - 'A';
        ++*t;
        r = &_var[c];
    }
    else if (**t == B_ARRAY) {
        ++*t;
        c = **t; ++*t;
        n1 = expression (t, B_CLOSEPAR, &e0);    // 添字の処理
        if (e0) {
            *e = e0;
            goto exit_this;
        }

        r = expression_array_search (c, n1, &e0);
        if (e0) {
            *e = e0;
            goto exit_this;
        }
    }
    else {
        *e = B_ERR_SYNTAX_ERROR;
    }
exit_this:
	return r;
}


/*
 * 変数の書き換えを行う
 * LET文への対応は不可
 * t : 中間コード列
 * n : 代入する数値
 * e : エラーコード
 *
 * 戻り値
 * 中間コード列へのポインタを返す
 * e にエラーコードを入れて戻す
 */
uint8_t *basic_write_variable (uint8_t *t, int16_t n, int16_t *e)
{
    int16_t n1, e0, *p;
    uint8_t c;

	p = basic_search_variable_address (&t, &e0);
	if (p == NULL) {
		*e = e0;
	}
	else {
		*p = n;
	}
	return t;
#if 0
    if (*t == B_VAR) {
        t++;
        c = (*t++ - 'A');
        _var[c] = n;
    }
    else if (*t == B_ARRAY) {
        t++;
        c = *t++;
        n1 = expression (&t, B_CLOSEPAR, &e0);    // 添字の処理
        if (e0) {
            *e = e0;
            goto exit_this;
        }

        p = expression_array_search (c, n1, &e0);
        if (e0) {
            *e = e0;
            goto exit_this;
        }
        *p = n;
    }
    else {
        *e = B_ERR_SYNTAX_ERROR;
    }
exit_this:
    return t;
#endif
}

/*
 * BASICインタープリタ本体
 * エラーコードを返す
 */
int16_t basic (EditorBuffer *ed, LineBuffer *ln)
{
    uint8_t *pos, c, *jmp, *tmp, *t, *t_save;
    int16_t n, n1, e, f, onflag, *var_p;
    STACK *sp;

    t = ln->wordbuff;
    if (ed->currtop == NULL) {
        // 実行環境の初期化
        ed->currlen = 0;    // 実行中の行の長さ
        ed->currline = 0;   // 実行中の行番号
        ed->currpos = NULL;
        ed->readnext = NULL;// DATA 文の先頭
        ed->breakpoint = NULL;
        stackpointer = -1;  // 要検討
        onflag      = 0;    // ON n GOTO,GOSUB,RESTORE用(n値,flag兼用)
    }

    //~ __dump (t, 64);
    while (*t != B_EOT) {
        if (ed->currtop != NULL) ed->currpos = t;
        switch (*t++) {
            case B_CALL:
            case B_POKEW:
            case B_POKE:
            default:
                return B_ERR_SYNTAX_ERROR;

            case B_SWAP:
				{
					int16_t *left = basic_search_variable_address (&t, &e);
					if (left == NULL) return e;
					if (*t != B_COMMA) return B_ERR_SYNTAX_ERROR;
					t++;
					int16_t *right = basic_search_variable_address (&t, &e);
					if (right == NULL) return e;
					n = *left; *left = *right; *right = n;
				}
				continue;
				
            case B_DIM:
                if (t[0] == B_ARRAY) {
                    c = t[1];   // 変数名
                    t++; t++;
                    n = expression (&t, B_CLOSEPAR, &e);
                    if (e) return e;
                    e = expression_array_setup (ed, c, n);
                    if (e) return e;
                }
                continue;

            case B_CLEAR:
                n = expression (&t, 0, &e);
                if (e) return e;
                switch (n) {
                    default:
                        // 全ての変数を初期化する
                        memset (_var, 0, sizeof(_var));
                    case 2:
                        // 配列変数を消去する
                        expression_array_clear (ed);
                        break;
                    case 1:
                        // 単純変数のみ初期化する
                        memset (_var, 0, sizeof(_var));
                        break;
                }
                continue;

            case B_LOAD:
                if (ed->currtop != NULL) return B_ERR_ILLEAGAL_FUNCTION_CALL;
                EditorBuffer_new ();    // init の代用
                stackpointer = -1;
                expression_array_init ();
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
                t--;
                do {
                    t++;
                    // プロンプトの表示
                    if (*t == B_STR) {
                        t++;
                        while (*t != B_STR && *t != B_TOL && *t != B_EOT) putchar (*t++);
                        if (*t != B_STR) return B_ERR_SYNTAX_ERROR;
                        t++;
                        if (*t != B_COMMA) return B_ERR_SYNTAX_ERROR;
                        t++;
                    }
                    // 入力まち
                    for (;;) {
                        fgets (ln->inputbuffer, sizeof(ln->inputbuffer)-1,stdin);
                        tmp = ln->inputbuffer;
                        n1 = str2mid (&tmp, ln->wordbuff, sizeof(ln->wordbuff));
                        if (n1 < 0) {
                            return B_ERR_SYNTAX_ERROR;
                        }
                        tmp = ln->wordbuff;
                        n = expression (&tmp, 0, &e);
                        if (e) {
                            basic_puts ("??INPUT\n");
                        }
                        else {
                            break;
                        }
                    }

                    t = basic_write_variable (t, n, &e);
                    if (e) return e;
                } while (*t == B_COMMA);
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
                                ((ed->currtop == NULL)? NULL: /* direct mode */
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

                    t++;
                    t = basic_write_variable (t, n, &e);
                    if (e) return e;
               } while (*t == B_COMMA);
                continue;

            case B_NEW:
                if (ed->currtop != NULL) {
                    // 実行中に呼び出された
                    return B_ERR_ILLEAGAL_FUNCTION_CALL;
                }
                EditorBuffer_new ();    // init の代用
                stackpointer = -1;
                expression_array_init ();
                return B_ERR_NO_ERROR;

            case B_END:
                ed->currtop = ed->currpos = NULL;
                ed->currline = 0;
                return B_ERR_NO_ERROR;

            case B_RUN:
                t = ed->textarea;
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
                if (ed->breakpoint != NULL) {
                    // すでにstop文を実行してダイレクトモードに
                    // 抜けている場合はエラー
                    return B_ERR_ILLEAGAL_FUNCTION_CALL;
                }
                // CONTに備えてed->currtopを保持する
                ed->breakpoint = t;
                ed->breakline = ed->currline;
                return B_ERR_BREAK_IN;

            case B_CONT:
                if (ed->breakpoint == NULL) {
                    // break in 以外ではエラー
                    return B_ERR_ILLEAGAL_FUNCTION_CALL;
                }
                t = ed->breakpoint;
                ed->breakpoint = NULL;
                ed->breakline = 0;
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
                                ((ed->currtop == NULL)? NULL: /* direct mode */
                                (ed->currline < n)? t : NULL), &f);
                if (f == 1) {
                    //~ __dump (t, 64);
                    return B_ERR_UNDEFINED_LINE;
                }
                stack_push (STACK_TYPE_GOSUB, NULL, 0, t);
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
                                ((ed->currtop == NULL)? NULL: /* direct mode */
                                (ed->currline < n)? t : NULL), &f);
                if (f == 1) {
                    return B_ERR_UNDEFINED_LINE;
                }
                t = jmp;
                continue;

            case B_NEXT:
                if (stack_check ()) return B_ERR_NEXT_WITHOUT_FOR;
                sp = stack_tos ();
                if (sp->type != STACK_TYPE_FOR) return B_ERR_NEXT_WITHOUT_FOR;
                if (*(sp->var_p) >= sp->value) {
                    stack_pop ();
                }
                else {
                    ++*(sp->var_p);
                    t = sp->address;
                }
                continue;

            case B_FOR:
                if (stack_check () > 0) return B_ERR_STACK_OVER_FLOW;
                if (*t != B_VAR && *t != B_ARRAY) return B_ERR_SYNTAX_ERROR;
				var_p = basic_search_variable_address (&t, &e);
                if (var_p == NULL)      return e;
                //~ t++;
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
                stack_push (STACK_TYPE_FOR, var_p, n1, t);
                *var_p = n;
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
                if (*t == B_VAR || *t == B_ARRAY) {
                    continue;
                }
                return B_ERR_SYNTAX_ERROR;

            case B_VAR:
			case B_ARRAY:
				t--;
				t_save = t;
				var_p = basic_search_variable_address (&t, &e);
				if (var_p == NULL) return e;
				if (*t == B_EQ2) {
					t++;
					n = expression (&t, 0, &e);
					if (e) return e;
					*var_p = n;
					continue;
				}
				t = t_save;
				n = expression (&t, 0, &e); // 単行演算子の処理
				if (e) return e;
				continue;

            case B_COLON:
            case B_SEMICOLON:
                continue;

            case B_PRINT:
                for (uint8_t *print_save = NULL;;) {
                    if (*t == B_COMMA) {
                        t++;
                        continue;
                    }
                    else if (*t == B_SEMICOLON ||
                             *t == B_COLON ||
                             *t == B_TOL ||
                             *t == B_EOT) {
                            if (print_save != NULL) {
                                while ( *print_save != B_STR &&
                                        *print_save != B_TOL &&
                                        *print_save != B_EOT) putchar (*print_save++);
                            }
                            if (*t != B_SEMICOLON) putchar ('\n');
                            break;
                    }
                    else if (*t == B_STR) {
                            t++;
                            print_save = t;
                            // 一旦文字列を読み飛ばす
                            while (*t != B_EOT && *t != B_TOL) {
                                if (*t++ == B_STR) break;
                            }
                            continue;
                    }
                    else {
                        n = expression (&t, 0, &e);
                        if (e) return e;
                        if (print_save != NULL) {
                            // 式の直前に文字列がある場合は書式指定の有無をチェックする
                            uint8_t(*__putnum_sub_)(int16_t *n) = NULL;
                            for (;;) {
                                if (*print_save == B_STR || *print_save == B_EOT || *print_save == B_TOL) {
                                    print_save = NULL;
                                    break;
                                }
                                if (print_save[0] == '%') {
                                    if (print_save[1] == '%') {
                                        putchar (print_save[1]);
                                        print_save++;print_save++;
                                        continue;
                                    }
                                    print_save++;
                                    int16_t keta = 0;
                                    if (*print_save == '+') {
                                        keta |= PUTNUM_SIGN;
                                        print_save++;
                                    }
                                    if (*print_save == '0') {
                                        keta |= PUTNUM_ZERO;
                                        print_save++;
                                    }
                                    if (*print_save == 'c') {
                                        __putnum_sub_ = __putnum_sub_dec;
                                        putchar ((int8_t)(n & 0xff));
                                        print_save++;
                                        break;
                                    }
                                    keta |= get_number (&print_save, 10);
                                    switch (*print_save) {
                                        default:
                                            return B_ERR_FORMAT_ERROR;
                                        case 'd':
                                            __putnum_sub_ = __putnum_sub_dec;
                                            break;
                                        case 'x':
                                            __putnum_sub_ = __putnum_sub_hex;
                                            break;
                                        case 'b':
                                            __putnum_sub_ = __putnum_sub_bin;
                                            break;
                                    }
                                    basic_putnum (n, keta, __putnum_sub_);
                                    print_save++;
                                    break;
                                }
                                putchar (*print_save++);
                            }
                            if (__putnum_sub_ == NULL) basic_putnum (n, 0, __putnum_sub_dec);
                        }
                        else {
                            basic_putnum (n, 0, __putnum_sub_dec);
                        }
                    }
                }
                continue;

            case B_LIST:
                {
                    int16_t curr = 0, end = 32767, listflag = 0;
                    if (*t == B_NUM) {
                        t++;
                        curr = *((int16_t*)t);
                        t++;
                        t++;
                        if (*t != B_COMMA) {
                            end = curr;
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
                        else {
                            listflag = 1;
                        }
                    }
                    pos = EditorBuffer_search_line (ed, curr, NULL, &f);
                    n = 0;
                    while (*pos != B_EOT) {
                        if (*pos == B_TOL) {
                            ++pos;
                            curr = *((uint16_t *)pos);
                            if (curr > end) break;
                            basic_printf ("%d ", curr);
                            ++pos;++pos;
                            ++pos;
                            continue;
                        }
                        pos = show_line (pos);
                        if (listflag && !(++n & 7)) {
                            basic_puts ("-- press enter key ---");getchar ();
                        }
                    }
                }
                continue;
        }
    }
    return B_ERR_NO_ERROR;
}
