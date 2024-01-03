/*
 * expression.c
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
#include <stdlib.h>
#include "basic.h"



int16_t (*_basic_func[])(uint8_t **t, int16_t *e) = {
 basic_f_reserved, // basic_f_peekw ,      // 0xbe
 basic_f_reserved, // basic_f_peek ,
 basic_f_abs ,
 basic_f_rnd ,
 basic_f_sgn ,

 basic_f_free ,       // 0xc3
 basic_f_reserved, // basic_f_time ,       // 経過時間
 basic_f_reserved, // basic_f_year ,       // 年
 basic_f_reserved, // basic_f_month ,      // 月
 basic_f_reserved, // basic_f_day ,        // 日
 basic_f_reserved, // basic_f_hour ,       // 時
 basic_f_reserved, // basic_f_min ,        // 分
 basic_f_reserved, // basic_f_second ,     // 秒
 basic_f_reserved, // basic_f_adc ,        // a/d
 basic_f_reserved, // basic_f_temp ,       // 温度
 basic_f_reserved, // basic_f_hum ,        // 湿度
 basic_f_reserved, // basic_f_press ,      // 気圧

};

int16_t _var[26];

static EditorBuffer *inter_ed;
static int16_t _b_err;

#define CHECK_ERROR if(_b_err)return n

int16_t basic_f_reserved (uint8_t **pos, int16_t *e)
{
    *e = B_ERR_NO_ERROR;
    return 0;
}

int16_t basic_f_abs (uint8_t **pos, int16_t *e)
{
    int16_t n, e0;
    n = expression (pos, B_CLOSEPAR, &e0);
    *e = e0;
    return (n < 0) ? n * -1 : n;
}

int16_t basic_f_sgn (uint8_t **pos, int16_t *e)
{
    int16_t n, e0;
    n = expression (pos, B_CLOSEPAR, &e0);
    *e = e0;
    return 	(n == 0) ? 0 :
			(n < 0) ? -1 : 1;
}


extern int16_t _basic_free_area (void);
int16_t basic_f_free (uint8_t **pos, int16_t *e)
{
    int16_t m, n, e0;

    n = expression (pos, B_CLOSEPAR, &e0);

    m = (int16_t)inter_ed->last;
    if (n == 1) {
        // 確保できる配列の要素数を返す
        m -= 5;
        m /= sizeof(int16_t);
        if (m < 0) m = 0;
    }
    *e = e0;
    return m;
}

static uint16_t _rnd_reg = 12345;   // 暫定
int16_t basic_f_rnd (uint8_t **pos, int16_t *e)
{
    uint16_t bit;
    int16_t n, e0;

    n = expression (pos, B_CLOSEPAR, &e0);
    bit = (_rnd_reg & 0001) ^
            ((_rnd_reg & 0x0004) >> 2) ^
            ((_rnd_reg & 0x0008) >> 3) ^
            ((_rnd_reg & 0x0020) >> 5);
    _rnd_reg = (_rnd_reg >> 1) | (bit << 15);

    if (n == 0) {
        e0 = B_ERR_DIVIDE_BY_ZERO;
        n = 0;
    }
    else {
        n = (int16_t)(_rnd_reg % n);
    }
    *e = e0;
    return n;
}

/*
 * 配列変数
 * １次元のみ。２次元以上は添字の事前計算で代用する。
 *
 * 中間コード                                uint8_t
 * 変数名                          v   uint8_t (アルファベット１文字）
 * 添字最大値                            l   int16_t
 * 実際のデータ（固定長
 * 末尾                       '\0'
 */
static uint8_t *array_top;  // 配列変数格納域先頭
static uint8_t *array_next; // 配列変数次回格納域先頭

void expression_init (EditorBuffer *ed)
{
    // 関数からインタープリタ側の状況にアクセスするための初期化処理
    inter_ed = ed;
}

void expression_array_init (void)
{
    array_top = NULL;
    array_next = NULL;
}

int16_t expression_array_clear (EditorBuffer *ed)
{
    //~ printf ("%p  %p\n", ed->eot, &ed->textarea[sizeof(ed->textarea)-1]);
    if (ed->eot < &ed->textarea[sizeof(ed->textarea)-1]) {
        ed->eot[1] = '\0';
        ed->last = (uint16_t)(&(ed->textarea[sizeof(ed->textarea)-1]) - ed->eot);
    }
    expression_array_init ();
}

int16_t expression_array_setup (EditorBuffer *ed, uint8_t var, int16_t arraysize)
{
    int16_t i, e;
    uint8_t *tmp;

    if (array_top == NULL) {
        array_top = ed->eot;
        array_top++;    // ソースコードの直後に配置する
        array_next = array_top;
        *array_next = '\0';
        ed->last--;     // 末尾の'\0'分を減じる
    }
    if (expression_array_search (var, 0, &e) != NULL) {
        return B_ERR_DUPLICATE;
    }
    // ヘッダ 4
    if (ed->last < (arraysize * sizeof(int16_t) + 4)) return B_ERR_OUT_OF_MEMORY;

    *array_next++ = B_ARRAY;
    *array_next++ = var;
    *((int16_t*)array_next) = arraysize;
    array_next += (2 + arraysize * sizeof(int16_t));
    *array_next = '\0';
    ed->last -= (4 + arraysize * sizeof(int16_t));
    return B_ERR_NO_ERROR;
}

/*
 * 配列変数名と添字から実際の格納ポインタを返す
 * 添字が溢れていたり、配列が未定義の場合は *e にエラーコードを入れて
 * NULL を返す。
 */
int16_t *expression_array_search (uint8_t var, int16_t index, int16_t *e)
{
    uint8_t *pos;
    int16_t i;

    if (array_top != NULL) {
        pos = array_top;
        while (*pos == B_ARRAY) {
            i = *((int16_t*)&pos[2]);
            if (index < 0) index += i;  // 添字が負数の場合は末端から参照する
            if (pos[1] == var) {
                if (index >= 0 && index < i) {
                    pos += 4;
                    pos += (sizeof(int16_t) * index);
                    *e = B_ERR_NO_ERROR;
                    return (int16_t*)pos;
                }
                else {
                    *e = B_ERR_INDEX_ERROR;
                    return NULL;
                }
            }
            else {
                pos += 4;
                pos += sizeof(int16_t) * i;
            }
        }
    }
    *e = B_ERR_UNDEFINED_VARIABLE;
    return NULL;
}


int16_t factor (uint8_t **pos)
{
    int16_t n = 0, n1 = 0, e, *v;
    uint8_t c;

    switch (**pos) {
        case B_ARRAY:
            ++*pos;
            c = **pos;  // 変数名
            ++*pos;
            n = expression (pos, B_CLOSEPAR, &e);
            if (e) {
                _b_err = e;
                break;
            }

            v = expression_array_search (c, n, &e);
            if (v == NULL) {
                _b_err = e;
                break;
            }
            n = *v;
            break;

        default:
            if (**pos >= B_F_PEEKW && **pos <= B_F_PRESS) {
                // 関数の処理
                c = **pos - B_F_PEEKW;
                ++*pos;
                if (**pos == B_OPENPAR) {
                    ++*pos;
                    n = _basic_func[c](pos, &e);
                    _b_err = e;
                }
                else {
                    _b_err = B_ERR_SYNTAX_ERROR;
                }
            }
            break;

        case B_OPENPAR:
            ++*pos;
            n = expression (pos, B_CLOSEPAR, &e);
            _b_err = e;
            break;

        case B_VAR:
            ++*pos;
            c = **pos - 'A';
            if (c > 25) {
                _b_err = B_ERR_SYNTAX_ERROR;    // Error exit
                break;
            }
            ++*pos;
            switch (**pos) {
                case B_INC:
                    _var[c]++;
                    ++*pos;
                    break;
                case B_DEC:
                    _var[c]--;
                    ++*pos;
                    break;
                default:
                    break;
            }
            n = _var[c];
            break;

        case B_NOT:
            ++*pos;
            n = !factor (pos);
            break;
        case B_NEG:
            ++*pos;
            n = ~factor (pos);
            break;
        case B_PLUS:
            ++*pos;
            n = factor (pos);
            break;
        case B_MINUS:
            ++*pos;
            n = -factor (pos);
            break;
        case B_NUM:
        case B_HEXNUM:
        case B_BINNUM:
            ++*pos;
            n = **((int16_t **)pos);
            ++*pos;
            ++*pos;
            break;
    }
    return n;
}


/*
 * 乗除余
 */
int16_t term6 (uint8_t **pos)
{
    int16_t n, n1;
    uint8_t c;

    n = factor (pos);   CHECK_ERROR;
    while (**pos == B_MUL || **pos == B_DIV || **pos == B_MOD) {
        c = **pos;
        ++*pos;
        n1 = factor (pos);   CHECK_ERROR;
        switch (c) {
            case B_MUL:
                n *= n1;
                break;
            case B_DIV:
                if (n1 == 0) {
                    _b_err = B_ERR_DIVIDE_BY_ZERO;
                }
                else {
                    n /= n1;
                }
                break;
            case B_MOD:
                if (n1 == 0) {
                    _b_err = B_ERR_DIVIDE_BY_ZERO;
                }
                else {
                    n %= n1;
                }
                break;
            default:
                break;
        }
    }
    return n;
}

/*
 * 加減
 */
int16_t term5 (uint8_t **pos)
{
    int16_t n, n1;
    uint8_t c;

    n = term6 (pos);   CHECK_ERROR;
    while (**pos == B_PLUS || **pos == B_MINUS) {
        c = **pos;
        ++*pos;
        n1 = term6 (pos);   CHECK_ERROR;
        switch (c) {
            case B_PLUS:
                n += n1;
                break;
            case B_MINUS:
                n -= n1;
                break;
            default:
                break;
        }
    }
    return n;
}

/*
 * シフト
 */
int16_t term4 (uint8_t **pos)
{
    int16_t n, n1;
    uint8_t c;

    n = term5 (pos);   CHECK_ERROR;
    // 算術シフトは未実装
    while (**pos == B_LSHIFT || **pos == B_RSHIFT) {
        c = **pos;
        ++*pos;
        n1 = term5 (pos);   CHECK_ERROR;
        switch (c) {
            case B_LSHIFT:
                n <<= n1;
                break;
            case B_RSHIFT:
                n >>= n1;
                break;
            default:
                break;
        }
    }
    return n;
}

/*
 * 比較
 */
int16_t term3 (uint8_t **pos)
{
    int16_t n, n1;
    uint8_t c;

    n = term4 (pos);   CHECK_ERROR;
    while (**pos == B_LE ||**pos == B_GRE ||**pos == B_LESS ||**pos == B_GR ) {
        c = **pos;
        ++*pos;
        n1 = term4 (pos);   CHECK_ERROR;
        switch (c) {
            case B_LE:
                n = (n <= n1)? 1: 0;
                break;
            case B_LESS:
                n = (n < n1)? 1: 0;
                break;
            case B_GR:
                n = (n > n1)? 1: 0;
                break;
            case B_GRE:
                n = (n >= n1)? 1: 0;
                break;
            default:
                break;
        }
    }
    return n;
}

/*
 * ==, !=
 */
int16_t term2 (uint8_t **pos)
{
    int16_t n, n1;
    uint8_t c;

    n = term3 (pos);   CHECK_ERROR;
    while (**pos == B_EQ || **pos == B_NEQ) {
        c = **pos;
        ++*pos;
        n1 = term3 (pos);   CHECK_ERROR;
        switch (c) {
            case B_EQ:
                n = (n == n1) ? 1: 0;
                break;
            case B_NEQ:
                n = (n != n1) ? 1: 0;
                break;
            default:
                break;
        }
    }
    return n;
}

/*
 * bit and, or, xor
 */
int16_t term1 (uint8_t **pos)
{
    int16_t n, n1;
    uint8_t c;

    n = term2 (pos);   CHECK_ERROR;
    while (**pos == B_BINAND || **pos == B_BINOR || **pos == B_XOR) {
        c = **pos;
        ++*pos;
        n1 = term2 (pos);   CHECK_ERROR;
        switch (c) {
            case B_BINAND:
                n &= n1;
                break;
            case B_BINOR:
                n |= n1;
                break;
            case B_XOR:
                n ^= n1;
                break;
            default:
                break;
        }
    }
    return n;
}

int16_t expression (uint8_t **pos, uint8_t endcode, int16_t *e)
{
    int16_t n, n1;
    uint8_t c;
    *e = _b_err = B_ERR_NO_ERROR;

    n = term1 (pos); if (_b_err) {*e = _b_err; return n;}
    while (**pos == B_OR || **pos == B_AND) {
        c = **pos;
        ++*pos;
        n1 = term1 (pos); if (_b_err) {*e = _b_err; return n;}
        if (c == B_OR) {
            n = (n != 0) | (n1 != 0);
        }else if (c == B_AND) {
            n = (n != 0) & (n1 != 0);
        }
    }

    if (**pos != endcode) {
        if (endcode != B_CLOSEPAR) {
            // 終端に閉じカッコが要求されていない場合は、区切り子でも
            // 正常終了とする
            switch (**pos) {
                case B_COLON:
                case B_SEMICOLON:
                case B_COMMA:
                case B_TOL:
                case B_EOT:
                    return n;
            }
        }
        *e = B_ERR_SYNTAX_ERROR;
        return n;
    }
    ++*pos;
    return n;
}

