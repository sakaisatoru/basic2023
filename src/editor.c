#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "basic.h"

/*
    BASIC 中間コードの格納状態

    B_TOL 行番号:uint16_t 行長さ:uint8_t 中間コード列
    B_TOL 行番号:uint16_t 行長さ:uint8_t 中間コード列
    B_TOL 行番号:uint16_t 行長さ:uint8_t 中間コード列
    B_EOT
    
	行削除
	B_TOL word byte ...     B_TOL			B_EOT
	^						^
	dest					source			転送量 (int)(B_EOT - B_TOL(source)+1)
	
	行挿入
	挿入内容
	B_NUM word 中間コード列       長さは 字句変換側で求めておく (len)

	B_TOL word byte ...     			B_EOT
	^						^
	source					dest(source+len)  転送量 (int)(B_EOT - dest)+1)
	
	
	 
		
*/




struct __LineBuffer {
        uint8_t inputbuffer[256];   // 入力バッファ
        uint8_t *pos;
        int len;
        uint8_t wordbuff[128];      // 中間コード格納バッファ
        int wordlen;
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
    return &editerbuf;
}

/*
 * １行分の入力を受付け、中間コードに変換する
 */
int LineBuffer_console (LineBuffer *ln)
{
	uint8_t *text;
	int rv;
	
	fgets (ln->inputbuffer, sizeof(ln->inputbuffer)-1, stdin);
    ln->pos = strchr (ln->inputbuffer, '\n');
    if (ln->pos != NULL) *(ln->pos) = '\0';
    else ln->inputbuffer[sizeof(ln->inputbuffer)-1] = '\0';
    
	//~ printf ("debuf : %s\n",linebuff);
	text = ln->inputbuffer;
	if (rv = str2mid (&text, ln->wordbuff, sizeof(ln->wordbuff))) {
		// error
		if (rv == 1) {
			printf ("buffer overflow.");
		}
		else {
			printf ("undefined word.");
		}
	}
	return rv;
}


#if 0
/*
 * 指定行を探す
 *
 * 見つかった場合　：行の先頭 　E_TOLを返す。さらに *cdx　は 0
 * 見つからなかった：次行の先頭 E_TOLを返す。さらに *cdx　は 1
 */
uint8_t *editor_search_line (EditorBuffer *ed, uint16_t linenumber, int *cdx)
{
    uint8_t *pos = ed->textarea;
    uint16_t n;
    *cdx = 0;
    for (;;) {
        if (*pos == E_EOT) {
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
        pos += n;
    }
    return pos;
}

/*
 * 中間コード列の挿入・置換を行う
 *
 */
int editor_insert_and_replace (EditorBuffer *ed, LineBuffer *ln)
{
    int rv = 0;
    uint16_t n;
    uint8_t *wp;
    uint8_t *dest, *sorce;

    wp = ln->wordbuff;
    wp++;
    n = *((uint16_t *)wp);

    dest = editor_search_line (ed, n, &rv);
    if (rv == 0) {
        // 置換
        // 既存行を削除する
        source = wp = dest;
        wp++;
        wp++;
        wp++;
        source += *wp;
        int ln = (int)(ed->eot - source);
        memmove (dest, source, ln);
        ed->eot -= ln;
    }

    if (rv == 1) {
        // 挿入
        memmove (dest + ln->wordlen, dest, ln->wordlen);
    }
    return rv;
}
#endif
