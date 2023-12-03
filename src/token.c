/*
 * 字句解析
 */

#include <stdint.h>
#include <stdio.h>
#include "basic.h"

static uint8_t word[] = {
0x80|'B','R','E','A','K',
0x80|'C','A','L','L',
0x80|'C','O','N','T','I','N','U','E',
0x80|'C','O','N','T',
0x80|'E','N','D',
0x80|'F','O','R',
0x80|'F','R','E','E',
0x80|'G','O','S','U','B',
0x80|'G','O','T','O',
0x80|'I','F',
0x80|'I','N','P','U','T',
0x80|'L','E','T',
0x80|'L','I','S','T',
0x80|'L','O','A','D',
0x80|'N','E','W',
0x80|'N','E','X','T',
0x80|'P','O','K','E','W',
0x80|'P','O','K','E',
0x80|'P','R','I','N','T',
0x80|'R','E','T','U','R','N',
0x80|'R','U','N',
0x80|'S','A','V','E',
0x80|'S','T','O','P',
0x80|'P','E','E','K','W',
0x80|'P','E','E','K',
0x80|'A','B','S',
0x80|'T','I','M','E',
0x80
};

#define SKPSPC  while(**text==' '||**text=='\t')++*text

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
uint8_t *code2word (uint8_t code)
{
    uint8_t *s;
    if (code < 0xa0) return NULL;

    code -= 0xa0;
    s = word;

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

uint8_t operator (uint8_t **text)
{
    uint8_t n;

    SKPSPC;
    switch ((n = **text, ++*text, n)) {
        case '~':
            n = B_NEG;  break;
        case '!':
            n = (**text == '=') ? (++*text,B_NEQ) : B_NOT;    break;
        case '+':
            n = (**text == '+') ? (++*text,B_INC) : B_PLUS;   break;
        case '-':
            n = (**text == '-') ? (++*text,B_DEC) : B_MINUS;  break;
        case '*':
            n = B_MUL;  break;
        case '/':
            n = B_DIV;  break;
        case '%':
            n = B_MOD;  break;
        case '<':
            n = (**text == '=') ? (++*text,B_LE) :
                (**text == '<') ? ((*(*text+1) == '<') ?
                    (++*text,++*text,B_LSHIFT2) : (++*text,B_LSHIFT)) :
                    B_LESS; break;
        case '>':
            n = (**text == '=') ? (++*text,B_GRE) :
                (**text == '>') ? ((*(*text+2) == '>') ?
                    (++*text,++*text,B_RSHIFT2) : (++*text,B_RSHIFT)) :
                    B_GR; break;
        case '=':
            n = (**text == '=') ? (++*text,B_EQ) : B_EQ2;   break;
        case '|':
            n = (**text == '|') ? (++*text,B_OR) : B_BINOR; break;
        case '^':
            n = B_XOR;  break;
        case '&':
            n = (**text == '&') ? (++*text,B_AND) : B_BINAND; break;
        case '(':
            n = B_OPENPAR;  break;
        case ')':
            n = B_CLOSEPAR; break;
        case '\'':
            n = B_REMARK;   break;
        case '\"':
            n = B_STR;      break;
        case ',':
            n = B_COMMA;    break;
        case ':':
            n = B_COLON;    break;
        case ';':
            n = B_SEMICOLON;break;
        case '\n':
            n = B_EOL;      break;
        default:
            --*text;
            n = 0;
            break;
    }
    return n;
}

uint8_t token (uint8_t **text)
{
    uint8_t n;
    SKPSPC;

    if (**text >= '0' && **text <= '9') {
        n = B_NUM;
    }
    else if (**text == '@') {
        n = B_ARRAY;
    }
    else if (**text >= 'A' && **text <= 'Z') {
        uint8_t *table = word;
        uint8_t *subtext = *text;
        n = B_BREAK;    // 予約語内部コードの先頭
        while (*table != 0x80) {
            if ((0x7f & *table) == **text) {
                table++;    ++*text;
                while (*table == **text) {
                    table++;    ++*text;
                }
                if (((0x80 & *table) == 0x80) && !(**text >= 'A' && **text <= 'Z')) {
                    return n;
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
    else {
        n = operator (text);
    }
    return n;
}


