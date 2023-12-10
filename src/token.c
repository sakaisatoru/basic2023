/*
 * 字句解析
 */
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "basic.h"
#if 0
static uint8_t basic_word[] = {
0x80|'~',           // B_NEG = 0x81
0x80|'!',           // B_NOT
0x80|'+','+',       // B_INC
0x80|'-','-',       // B_DEC
0x80|'*',           // B_MUL
0x80|'/',           // B_DIV
0x80|'%',           // B_MOD
0x80|'+',           // B_PLUS
0x80|'-',           // B_MINUS
0x80|'<','<',       // B_LSHIFT
0x80|'>','>',       // B_RSHIFT
0x80|'<','<','<',   // B_LSHIFT2
0x80|'>','>','>',   // B_RSHIFT2
0x80|'<','=',       // B_LE
0x80|'>','=',       // B_GRE

0x80|'<',           // B_LESS = 0x90
0x80|'>',           // B_GR
0x80|'=','=',       // B_EQ
0x80|'=',           // B_EQ2
0x80|'!','=',       // B_NEQ
0x80|'|',           // B_BINOR
0x80|'^',           // B_XOR
0x80|'&',           // B_BINAND
0x80|'|','|',       // B_OR
0x80|'&','&',       // B_AND
0x80|'(',           // B_OPENPAR
0x80|')',           // B_CLOSEPAR
0x80|'\'',          // B_REMARK
0x80|',',           // B_COMMA
0x80|':',           // B_COLON
0x80|';',           // B_SEMICOLON
0x80|'@',           // B_ARRAY = 0xa0
0x80|'\"',          // B_STR
#endif

static uint8_t basic_word[] = {
//~ 0x80|'B','R','E','A','K',
0x80|'C','A','L','L',
0x80|'C','L','E','A','R',
//~ 0x80|'C','O','N','T','I','N','U','E',
0x80|'C','O','N','T',
0x80|'D','A','T','A',
0x80|'D','I','M',
0x80|'E','N','D',
0x80|'F','O','R',
0x80|'G','O','S','U','B',
0x80|'G','O','T','O',
0x80|'I','F',
0x80|'I','N','P','U','T',
0x80|'L','E','T',
0x80|'L','I','S','T',
0x80|'L','O','A','D',
0x80|'N','E','W',
0x80|'N','E','X','T',
0x80|'O','N',
0x80|'P','O','K','E','W',
0x80|'P','O','K','E',
0x80|'P','R','I','N','T',
0x80|'R','E','A','D',
0x80|'R','E','S','T','O','R','E',
0x80|'R','E','T','U','R','N',
0x80|'R','U','N',
0x80|'S','A','V','E',
0x80|'S','T','O','P',
0x80|'T','H','E','N',
0x80|'T','O',
0x80|'P','E','E','K','W',
0x80|'P','E','E','K',
0x80|'A','B','S',
0x80|'R','N','D',
0x80|'F','R','E','E',
0x80|'T','I','M','E',
0x80
};


/*
 * 予約語を表示する
 */
void __putch (uint8_t c)
{
    c &= 0x7f;
    putchar (c);
}

void put_basic_word (uint8_t *s, void(*__putc)(uint8_t))
{
    do {
        __putc (*s++);
    } while ((0x80 & *s) != 0x80);
}


/*
 * 中間コードから予約語を得る
 */
uint8_t *code2word (uint8_t code, uint8_t topcode, uint8_t *s)
{
    if (code < topcode) return NULL;

    code -= topcode;

    while (code > 0) {
        s++;
        while ((0x80 & *s) != 0x80) s++;
        if ((*s & 0x7f) == 0x00) {
            s = NULL;
            break;
        }
        code--;
    }

    return s;
}

int16_t get_number (uint8_t **text)
{
    int16_t n;

    n = 0;
    if (**text == '0') {
        ++*text;
        switch (**text) {
            case 'X':
            case 'x':
                // １６進数
                for (;;) {
                    ++*text;
                    if (**text >= '0' && **text <= '9') {
                        n <<= 4;
                        n |= (**text - '0');
                    }
                    else if (**text >= 'A' && **text <= 'F') {
                        n <<=4;
                        n |= (**text - 'A' + 10);
                    }
                    else {
                        break;
                    }
                }
                return n;

            case 'B':
            case 'b':
                // ２進数
                for (;;) {
                    ++*text;
                    if (**text == '0' && **text == '1') {
                        n <<= 1;
                        n |= (**text - '0');
                    }
                    else {
                        break;
                    }
                }
                return n;

            default:
                break;
        }
    }

    // １０進数
    while (**text >= '0' && **text <= '9') {
        n *= 10;
        n += (**text - '0');
        ++*text;
    }
    return n;
}


uint8_t token (uint8_t **text)
{
    uint8_t n, *tmp;
    while(**text==' '||**text=='\t')++*text;

    switch (**text) {
        case '~':       n = B_NEG;      ++*text;return n; // 0x81
        case '*':       n = B_MUL;      ++*text;return n;
        case '/':       n = B_DIV;      ++*text;return n;
        case '%':       n = B_MOD;      ++*text;return n;
        case '^':       n = B_XOR;      ++*text;return n;
        case '(':       n = B_OPENPAR;  ++*text;return n;
        case ')':       n = B_CLOSEPAR; ++*text;return n;
        case '\'':      n = B_REMARK;   ++*text;return n;
        case ',':       n = B_COMMA;    ++*text;return n;
        case ':':       n = B_COLON;    ++*text;return n;
        case ';':       n = B_SEMICOLON; ++*text;return n;
        case '@':       n = B_ARRAY;    ++*text;return n;
        case '\"':      n = B_STR;      ++*text;return n;

        case '!':
            if (*(*text+1) == '=') {++*text; n = B_NEQ};
            else n = B_NOT;
            ++*text;
            return n;
        case '+':
            if (*(*text+1) == '+') {++*text; n = B_INC};
            else n = B_PLUS;
            ++*text;
            return n;
        case '-':
            if (*(*text+1) == '-') {++*text; n = B_DEC};
            else n = B_PLUS;
            ++*text;
            return n;

        case '<':
            tmp = *text;
            n = (tmp[1] == '<') ?
                    ((tmp[2] == '<') ? B_LSHIFT2 : B_LSHIFT) :
                (tmp[1] == '=') ? B_LE : B_LESS;
            return n;

        case '>':
            tmp = *text;
            n = (tmp[1] == '>') ?
                    ((tmp[2] == '>') ? B_RSHIFT2 : B_RSHIFT) :
                (tmp[1] == '=') ? B_GRE : B_GR;
            return n;

        case '=':
            tmp = *text;
            n = (tmp[1] == '=') ? B_EQ:B_EQ2;
            return n;

        case '|':
            tmp = *text;
            n = (tmp[1] == '|') ? B_OR:B_BINOR;
            return n;

        case '&':
            tmp = *text;
            n = (tmp[1] == '&') ? B_AND:B_BINAND;
            return n;
    }

    if (**text >= '0' && **text <= '9') {
        n = (*(*text+1) == 'X') ? B_HEXNUM :
            (*(*text+1) == 'B') ? B_BINNUM : B_NUM;
    }
    else if (**text == '@') {
        n = B_ARRAY;
    }
    else if (**text == '\0' || **text == '\n') {
        n = B_EOT;
    }
    else {
        uint8_t *table = basic_word;
        uint8_t *subtext = *text;
        uint8_t p;
        n = B_CALL;    // 予約語内部コードの先頭
        while (*table != 0x80) {
            if ((0x7f & *table) == **text) {
                p = **text;
                table++;    ++*text;
                while (*table == **text) {
                    table++;    ++*text;
                }
                if ((0x80 & *table) == 0x80) {
                    if (isalpha(p)) {
                        // 最初の１文字がアルファベットで始まっている場合は、記号を区切り文字とする
                        if (!isalpha(**text)) return n;
                    }
                    else {
                        // 記号の時はアルファベット、数字、空白を区切りとする
                        if (isalnum(**text) || **text == ' ') return n;
                    }
                }
            }
            n++;
            *text = subtext;
            table++;
            while ((*table & 0x80) == 0x00) table++;
        }
        // 未定義だった
        *text = subtext;
        subtext++;
        n = (!(*subtext >= 'A' && *subtext <= 'Z')) ? B_VAR : 0;
    }
    return n;
}

/*
 * 中間コード列からソースを表示する。TOL,EOTにあったら終わる。
 */
uint8_t *show_line (uint8_t *pos)
{
    uint8_t *s;

    while (*pos != B_EOT || *pos != B_TOL){
        switch (*pos) {
            case B_HEXNUM:
                ++pos;
                printf ("0X%X", *((uint16_t *)*pos));
                ++pos;
                ++pos;
                break;
            case B_BINNUM:
                ++pos;
                uint16_t b = *((uint16_t *)*pos);
                printf ("0B");
                for (int i=0; i<16; i++) {
                    putchar ((b & 0x8000) ? '1':'0');
                    b <<= 1;
                }
                ++pos;
                ++pos;
                break;

            case B_NUM:
                ++pos;
                printf ("%d", *((uint16_t *)pos));
                ++pos;
                ++pos;
                break;

            case B_STR:
                ++pos;
                putchar ('\"');
                while (*pos != B_STR){
                    putchar (*pos);
                    ++pos;
                }
                putchar ('\"');
                pos++;
                break;

            case B_REMARK:
                ++pos;
                putchar ('\'');
                while (*pos != B_EOT){
                    putchar (*pos);
                    ++pos;
                }
                break;

            case B_VAR:
                ++pos;
                putchar (*pos);
                pos++;
                break;

            default:
                s = code2word (*pos, B_NEG,   basic_word);
                if (s != NULL) {
                    if (*pos == B_THEN || *pos == B_TO) putchar (' ');
                    put_basic_word (s, __putch);
                    if (*pos >= B_CALL) putchar (' ');
                    pos++;
                    break;
                }
                goto exit_this;
        }
    }
exit_this:
    putchar ('\n');
    return pos;
}

/*
 * 文字列を中間コードに変換しバッファに返す
 * 得られた中間コード列の長さを返す
 * -1 : バッファオーバー
 * -2 : 未定義語が出現した
 */
int16_t str2mid (uint8_t **text, uint8_t *buff, int16_t buffsize)
{
    int16_t rv = 0;

    uint8_t *pos = buff;
    uint8_t n;
    int16_t d;

    while ((n = token (text)) != 0) {
        *pos++ = n;
        switch (n) {
            case B_EOT:
                pos--;
                rv = (int)(pos - buff);
                goto exit_this;

            case B_VAR:
                *pos++ = **text;
                ++*text;
                break;

            case B_NUM:
            case B_HEXNUM:
            case B_BINNUM:
                d = get_number (text);
                *((int16_t *)pos) = d;
                pos++;
                pos++;
                break;

            case B_STR:
                do {
                    *pos++ = **text;
                    ++*text;
                }while (**text != '\"');
                *pos++ = B_STR;
                ++*text;
                break;

            default:
                break;
        }
        if (pos >= &buff[buffsize-1]) {
            rv = -1;
            break;
        }
    }
    rv = -2;
exit_this:
    return rv;
}
