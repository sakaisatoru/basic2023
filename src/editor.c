/*
 * editor.c
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "basic.h"


void EditorBuffer_show_error_message (EditorBuffer *ed, int16_t err);
void EditorBuffer_delete_line (EditorBuffer *ed, uint16_t linenumber);
int16_t EditorBuffer_insert_and_replace (EditorBuffer *ed, LineBuffer *ln);

/*
    BASIC 中間コードの格納状態

    B_TOL 行番号:uint16_t 行長さ:uint8_t 中間コード列 ※行の長さには B_TOL,行番号,行の長さを含む
    B_TOL 行番号:uint16_t 行長さ:uint8_t 中間コード列
    B_TOL 行番号:uint16_t 行長さ:uint8_t 中間コード列
    B_EOT

    行削除
    B_TOL word byte ...     B_TOL           B_EOT
    ^                       ^
    dest                    source          転送量 (int)(B_EOT - B_TOL(source)+1)

    行挿入
    挿入内容
    B_NUM word 中間コード列       長さは 字句変換側で求めておく (len)

    B_TOL word byte ...                 B_EOT
    ^                       ^
    source                  dest(source+len)  転送量 (int)(B_EOT - dest)+1)

*/



static LineBuffer lnbuf;
static EditorBuffer editorbuf;

void EditorBuffer_start_message (EditorBuffer *ed)
{
    basic_printf ( PACKAGE_STRING"\n"
                   "by endeavor_wako since 2023\n"
                   "%d bytes free.\n", ed->last);
    //~ basic_putnum (ed->last, 0, __putnum_sub_dec);
    //~ basic_puts (" bytes free.\n");
}

LineBuffer *LineBuffer_new (void)
{
    return &lnbuf;
}

uint8_t *LineBuffer_get_midbuffer (LineBuffer *ln)
{
    return ln->wordbuff;
}

EditorBuffer *EditorBuffer_new (void)
{
    editorbuf.last = sizeof(editorbuf.textarea) - 1;
    editorbuf.eot = editorbuf.textarea;
    editorbuf.eot[0] = B_EOT;
    editorbuf.eot[1] = '\0';    // 配列の初期化（暫定）
    editorbuf.currtop = NULL;   // 実行中の行の先頭
    editorbuf.currlen = 0;      // 実行中の行の長さ
    editorbuf.currline = 0;     // 実行中の行番号
    editorbuf.breakline = 0;
    editorbuf.breakpoint = NULL;
    editorbuf.readnext = NULL;
    return &editorbuf;
}



/*
 * １行分の入力を受付け、中間コードに変換する
 */
void LineBuffer_console (LineBuffer *ln, EditorBuffer *ed)
{
    uint8_t *text;
    int16_t err, len, mode;

    expression_init (ed);
    expression_array_init ();
    for (;;) {
        basic_puts ("OK\n");
        fgets (ln->inputbuffer, sizeof(ln->inputbuffer)-1, stdin);
        ln->pos = strchr (ln->inputbuffer, '\n');
        if (ln->pos != NULL) *(ln->pos) = '\0';
        else ln->inputbuffer[sizeof(ln->inputbuffer)-1] = '\0';

        ln->pos = ln->inputbuffer;
        len = str2mid (&ln->pos, ln->wordbuff, sizeof(ln->wordbuff));
        if (len == -1) {
            EditorBuffer_show_error_message (ed, B_ERR_BUFFER_OVER_FLOW);
        }
        else if (len == -2) {
            EditorBuffer_show_error_message (ed, B_ERR_SYNTAX_ERROR);
        }
        else {
            if (ln->wordbuff[0] == B_NUM) {
                expression_array_clear (ed);// 定義済の配列は全て消去される
                if (ln->wordbuff[3] == B_EOT) {
                    // 行削除
                    text = &ln->wordbuff[1];
                    EditorBuffer_delete_line (ed, *((uint16_t *)text));
                }
                else {
                    ln->wordlen = len;
                    EditorBuffer_insert_and_replace (ed, ln);
                }
            }
            else {
                err = basic (ed, ln);
                if (err) {
                    EditorBuffer_show_error_message (ed, err);
                    ed->currline = 0;
                }
                if (err != B_ERR_BREAK_IN) {
                    // STOP 以外は正常終了も含めて実行終了とする
                    ed->currtop = NULL;
                }
            }
        }
    }
}


/*
 * 指定行を探す
 *
 * 見つかった場合　：行の先頭 　E_TOLを返す。さらに *cdx　は 0
 * 見つからなかった：次行の先頭 E_TOLを返す。さらに *cdx　は 1
 *
 * linenumber : 検索する行
 * pos : 検索開始位置 NULL の場合はバッファ先頭から探す
 * cdx : 検索結果
 */
uint8_t *EditorBuffer_search_line (EditorBuffer *ed, uint16_t linenumber, uint8_t *p, int16_t *cdx)
{
    uint16_t n;
    uint8_t *pos;

    *cdx = 0;
    pos = (p == NULL) ? ed->textarea : p;

    for (;;) {
        switch (*pos) {
            default:
                pos++;
                break;
            case B_EOT:
                *cdx = 1;
                goto exit_this;
            case B_NUM: case B_HEXNUM: case B_BINNUM:
                pos++;  pos++;  pos++;
                break;
            case B_TOL:
                pos++;
                n = *((uint16_t*)pos);
                if (n >= linenumber) {
                    pos--;
                    if (n != linenumber) {
                        *cdx = 1;
                    }
                    goto exit_this;
                }
                n = (uint16_t)(pos[2]-1);
                pos += n;
                break;
        }
    }
exit_this:
    return pos;
}

/*
 * 指定行を削除する
 */
void EditorBuffer_delete_line (EditorBuffer *ed, uint16_t linenumber)
{
    uint8_t *dest, *source;
    uint16_t len, l;
    int16_t cdx;

    dest = EditorBuffer_search_line (ed, linenumber, NULL, &cdx);
    if (cdx == 1) {
        EditorBuffer_show_error_message (ed, B_ERR_UNDEFINED_LINE);
        return; // undefind
    }

//~ printf ("現在の空き容量 : %d\n", ed->last);
    l = *(dest + 3);
    source = dest + l;
    len = (uint16_t)(ed->eot - source);
//~ printf ("source : %04X   dest : %04X   length : %d\n",
            //~ source, dest, len);
    memmove (dest, source, len + 1);
    ed->last += l;
    ed->eot -= len;
//~ printf ("現在の空き容量 : %d\n", ed->last);
}


/*
 * 中間コード列の挿入・置換を行う
 *
 */
int16_t EditorBuffer_insert_and_replace (EditorBuffer *ed, LineBuffer *ln)
{
    int16_t rv = 0;
    uint16_t n, len, l;
    uint8_t *dest, *source;

    if (ed->last < ln->wordlen +1) return -1;   // out of memory.

    n = *((uint16_t *)&ln->wordbuff[1]);

    dest = EditorBuffer_search_line (ed, n, NULL, &rv);
    if (rv == 0) {
        // 置換
        // 既存行を削除する
        l = (uint16_t)dest[3];
        source = dest + l;
        len = (uint16_t)(ed->eot - source);
        memmove (dest, source, len + 1);
        ed->last += l;
        ed->eot -= l;
    }

    // 挿入
    memmove (dest + ln->wordlen +1, dest, (ed->eot - dest + 1));
    ed->eot += (ln->wordlen +1);
    dest[0] = B_TOL;
    dest[1] = ln->wordbuff[1];
    dest[2] = ln->wordbuff[2];
    dest[3] = ln->wordlen + 1;
    memmove (&dest[4], &ln->wordbuff[3], ln->wordlen - 3);
    ed->last -= (ln->wordlen+1);

    return 0;
}

/*
 * エラーメッセージを表示する
 */
void EditorBuffer_show_error_message (EditorBuffer *ed, int16_t err)
{
    static uint8_t *errmessage[] = {
        "No error",
        "Break",
        "Syntax error",
        "Illeagal function call",
        "Next without for",
        "Return without gosub",
        "Out of memory",
        "Stack over flow",
        "Undefined line",
        "Buffer over flow",
        "No DATA corresponding to READ",
        "I/O error",
        "Undefined Variable",
        "Index error",
        "Duplicate define symbol",
        "Divide by zero",
        "Format Error",
        "Out of memory",
    };

    if (err <= B_ERR_NO_ERROR || err >= B_ERR_BAD_ERROR_CODE) {
        basic_puts (errmessage[B_ERR_BAD_ERROR_CODE]);
    }
    else {
        basic_puts (errmessage[err]);
    }
    if (ed->currline != 0) {
        basic_printf (" in %d\n", ed->currline);
    }
    else {
        basic_puts (".\n");
    }
    if (ed->breakpoint == NULL && ed->currpos != NULL) {
        show_line (ed->currpos);
    }
}
