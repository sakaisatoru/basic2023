#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "basic.h"

extern int str2mid (uint8_t **text, uint8_t *buff, int buffsize);

uint8_t *editor_search_line (EditorBuffer *ed, uint16_t linenumber, uint8_t *pos, int *cdx);
void editor_delete_line (EditorBuffer *ed, uint16_t linenumber);
int editor_insert_and_replace (EditorBuffer *ed, LineBuffer *ln);

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

struct __LineBuffer {
        uint8_t inputbuffer[256];   // 入力バッファ
        uint8_t *pos;
        int len;
        uint8_t wordbuff[128];      // 中間コード格納バッファ
        uint8_t wordlen;
};

struct __EditorBuffer {
        uint8_t textarea[1024];
        uint8_t *eot;           // 末尾
        int     last;           // 残り容量
};

static LineBuffer lnbuf;
static EditorBuffer editerbuf;

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
    editerbuf.last = sizeof(editerbuf.textarea) - 1;
    editerbuf.eot = &editerbuf.textarea[0];
    editerbuf.textarea[0] = B_EOT;
    return &editerbuf;
}

uint8_t *EditorBuffer_get_textarea (EditorBuffer *ed)
{
    return ed->textarea;
}

/*
 * １行分の入力を受付け、中間コードに変換する
 */
int LineBuffer_console (LineBuffer *ln, EditorBuffer *ed)
{
    uint8_t *text;
    int rv;

    for (;;) {
        fgets (ln->inputbuffer, sizeof(ln->inputbuffer)-1, stdin);
        ln->pos = strchr (ln->inputbuffer, '\n');
        if (ln->pos != NULL) *(ln->pos) = '\0';
        else ln->inputbuffer[sizeof(ln->inputbuffer)-1] = '\0';

        ln->pos = ln->inputbuffer;
        rv = str2mid (&ln->pos, ln->wordbuff, sizeof(ln->wordbuff));
        if (rv == -1) {
            printf ("buffer overflow.");
        }
        else if (rv == -2) {
            printf ("undefined word.");
        }
        else {
            if (ln->wordbuff[0] == B_NUM) {
                if (ln->wordbuff[3] == B_EOT) {
                    text = &ln->wordbuff[1];
                    editor_delete_line (ed, *((uint16_t *)text));
                }
                ln->wordlen = rv;
                editor_insert_and_replace (ed, ln);
            }
            else {
				basic (ed, ln->wordbuff);
			}
        }
    }
    return rv;
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
uint8_t *editor_search_line (EditorBuffer *ed, uint16_t linenumber, uint8_t *pos, int *cdx)
{
    uint16_t n;

    *cdx = 0;
    if (pos == NULL) pos = ed->textarea;
    for (;;) {
        if (*pos == B_EOT) {
            *cdx = 1;
            break;
        }
        pos++;
        n = *((uint16_t*)pos);
        if (n >= linenumber) {
            pos--;
            if (n > linenumber) {
                *cdx = 1;
            }
            break;
        }
        pos++;
        pos++;
        n = (uint16_t)*pos;
        pos += (n - 3);
    }
    return pos;
}

/*
 * 指定行を削除する
 */
void editor_delete_line (EditorBuffer *ed, uint16_t linenumber)
{
    uint8_t *dest, *source;
    uint16_t len;
    int cdx;

    dest = editor_search_line (ed, linenumber, NULL, &cdx);
    if (cdx == 1) return; // undefind

printf ("現在の空き容量 : %d\n", ed->last);
    source = dest + *(dest + 3);
    len = (uint16_t)(ed->eot - dest);
printf ("source : %04X   dest : %04X   length : %d\n",
            source, dest, len);
    memmove (dest, source, len + 1);
    ed->last += len;
    ed->eot -= len;
printf ("現在の空き容量 : %d\n", ed->last);
}


/*
 * 中間コード列の挿入・置換を行う
 *
 */
int editor_insert_and_replace (EditorBuffer *ed, LineBuffer *ln)
{
    int rv = 0;
    uint16_t n;
    uint8_t *dest, *source;

    if (ed->last < ln->wordlen +1) return -1;   // out of memory.

    n = *((uint16_t *)&ln->wordbuff[1]);

    dest = editor_search_line (ed, n, NULL, &rv);
    if (rv == 0) {
        // 置換
        // 既存行を削除する
        source = dest + *(dest + 3);
        n = (uint16_t)(ed->eot - dest);
        memmove (dest, source, n + 1);
        ed->last += n;
        ed->eot -= n;
    }

    // 挿入
    memmove (dest + ln->wordlen +1, dest, ln->wordlen+1);
    dest[0] = B_TOL;
    dest[1] = ln->wordbuff[1];
    dest[2] = ln->wordbuff[2];
    dest[3] = ln->wordlen + 1;
    memmove (dest + 4, &ln->wordbuff[3], ln->wordlen - 3);
    ed->last -= (ln->wordlen+1);

    return 0;
}

