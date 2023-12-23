/*
 * token.c
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
#include <stdio.h>
#include <ctype.h>
#include "basic.h"

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
0x80|'(',           // B_OPENPAR
0x80|')',           // B_CLOSEPAR
0x80|'\'',          // B_REMARK
0x80|',',           // B_COMMA
0x80|':',           // B_COLON
0x80|';',           // B_SEMICOLON
0x80|'@',           // B_ARRAY = 0xa0
0x80|'\"',          // B_STR
0x80|'|','|',       // B_OR
0x80|'&','&',       // B_AND

0x80|'C','A','L','L',
0x80|'C','L','E','A','R',
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

int16_t get_number (uint8_t **text, uint8_t t)
{
    int16_t n = 0;
    switch (t) {
		case B_HEXNUM:
			// １６進数
			for (;;) {
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
				++*text;
			}
			break;

		case B_BINNUM:
			// ２進数
			while (**text == '0' || **text == '1') {
				n <<= 1;
				n |= (**text - '0');
				++*text;
			}
			break;
		
		default:
			// １０進数
			while (**text >= '0' && **text <= '9') {
				n *= 10;
				n += (**text - '0');
				++*text;
			}
            break;
    }

    return n;
}


uint8_t token (uint8_t **text)
{
    uint8_t n, *tmp;
    while(**text==' '||**text=='\t')++*text;

    switch (**text) {
        case '~':       ++*text; return B_NEG;
        case '*':       ++*text; return B_MUL;
        case '/':       ++*text; return B_DIV;
        case '%':       ++*text; return B_MOD;
        case '^':       ++*text; return B_XOR;
        case '(':       ++*text; return B_OPENPAR;
        case ')':       ++*text; return B_CLOSEPAR;
        case '\'':      ++*text; return B_REMARK;
        case ',':       ++*text; return B_COMMA;
        case ':':       ++*text; return B_COLON;
        case ';':       ++*text; return B_SEMICOLON;
        case '\"':      ++*text; return B_STR;

        case '!':
            return (++*text, (**text == '=')) ? (++*text, B_NEQ) : B_NOT;
        case '=':
            return (++*text, (**text == '=')) ? (++*text, B_EQ)  : B_EQ2;
        case '+':
            return (++*text, (**text == '+')) ? (++*text, B_INC) : B_PLUS;
        case '-':
            return (++*text, (**text == '-')) ? (++*text, B_DEC) : B_MINUS;
        case '|':
            return (++*text, (**text == '|')) ? (++*text, B_OR)  : B_BINOR;
        case '&':
            return (++*text, (**text == '&')) ? (++*text, B_AND) : B_BINAND;

        case '<':
            ++*text;
            if (**text == '=') {
                ++*text;
                n = B_LE;
                return n;
            }
            else if (**text == '<') {
                ++*text;
                if (**text == '<') {
                    ++*text;
                    n = B_LSHIFT2;
                    return n;
                }
                n = B_LSHIFT;
                return n;
            }
            n = B_LESS;
            return n;

        case '>':
            ++*text;
            if (**text == '=') {
                ++*text;
                n = B_GRE;
                return n;
            }
            else if (**text == '>') {
                ++*text;
                if (**text == '>') {
                    ++*text;
                    n = B_RSHIFT2;
                    return n;
                }
                n = B_RSHIFT;
                return n;
            }
            n = B_GR;
            return n;
    }

    if (**text >= '0' && **text <= '9') {
		if (**text == '0') {
			++*text;
			switch (**text | 0x20) {
				case 'x':	n = B_HEXNUM;	++*text;	break;
				case 'b':	n = B_BINNUM;	++*text;	break;
				default:	n = B_NUM;					break;
			}
		}
		else {
			n = B_NUM;
		}
    }
    else if (**text == '\0' || **text == '\n') {
        n = B_EOT;
    }
    else {
        uint8_t *table = basic_word;
        uint8_t *subtext = *text;
        uint8_t p;
        n = B_NEG;    // 予約語内部コードの先頭
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
        if (*subtext == '(') {
            n = B_ARRAY;
            text++; // 配列変数の'('は中間コードとして記録しないので飛ばす
        }
        else if (!(*subtext >= 'A' && *subtext <= 'Z')) {
            n = B_VAR;
        }
        else {
            n = 0;
        }
    }
    return n;
}

/*
 * 中間コード列からソースを表示する。TOL,EOTにあったら終わる。
 */
uint8_t *show_line (uint8_t *pos)
{
    uint8_t *s;
    uint16_t b;
    int16_t onflag = 0;

    while (*pos != B_EOT && *pos != B_TOL){
        switch (*pos++) {
            case B_HEXNUM:
                //~ printf ("0x%X", *((uint16_t *)pos));
                basic_putnum_hex (*((uint16_t *)pos), 4);
                ++pos;
                ++pos;
                break;
            case B_BINNUM:
                basic_putnum_bin (*((uint16_t *)pos), 16);
                ++pos;
                ++pos;
                break;
            case B_NUM:
                //~ printf ("%d", *((uint16_t *)pos));
                basic_putnum (*((uint16_t *)pos), 5);
                ++pos;
                ++pos;
                break;

            case B_STR:
                putchar ('\"');
                while (*pos != B_STR){
                    putchar (*pos++);
                }
                putchar ('\"');
                pos++;
                break;

            case B_REMARK:
                putchar ('\'');
                while (*pos != B_EOT && *pos != B_TOL){
                    putchar (*pos++);
                }
                break;

            case B_VAR:
                putchar (*pos++);
                break;

            case B_ARRAY:
                putchar (*pos++);
                putchar ('(');
                break;

            case B_ON:
                onflag = 1;
            default:
                pos--;
                s = code2word (*pos, B_NEG,   basic_word);
                if (s != NULL) {
                    // 見た目を整えるため特定のワードは直前に空白を挿入する
                    if (*pos == B_THEN || *pos == B_TO ||
                        *pos == B_OR || *pos == B_AND) putchar (' ');
                    if (onflag) {
                        if (*pos == B_GOTO || *pos == B_GOSUB ||
                            *pos == B_RESTORE) {
                            onflag = 0;
                            putchar (' ');
                        }
                    }
                    do {
						putchar (*s++ & 0x7f);
					} while ((0x80 & *s) != 0x80);
                    // ワードは後方に空白を挿入する
                    if (*pos >= B_OR) putchar (' ');
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
            case B_REMARK:
                while (**text != '\0' && **text != '\n') {
                    *pos++ = **text;
                    ++*text;
                }
                *pos++ = B_EOT;

            case B_EOT:
                pos--;
                rv = (int)(pos - buff);
                goto exit_this;

            case B_VAR:
                *pos++ = **text;
                ++*text;
                break;

            case B_ARRAY:
                *pos++ = **text;
                ++*text;    // '('を飛ばす
                ++*text;
                break;

            case B_NUM:
            case B_HEXNUM:
            case B_BINNUM:
                d = get_number (text, n);
                *((int16_t *)pos) = d;
                pos++;
                pos++;
                break;

            case B_STR:
                while (**text != '\"') {
                    *pos++ = **text;
                    ++*text;
                }
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
