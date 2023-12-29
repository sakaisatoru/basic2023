/*
 * basic.h
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
#include <stdint.h>

enum {
    B_NEG = 0x81,   // ~
    B_NOT,          // !
    B_INC,          // ++
    B_DEC,          // --

    B_MUL,          // *
    B_DIV,          // /
    B_MOD,          // %

    B_PLUS,         // +
    B_MINUS,        // -

    B_LSHIFT,       // <<
    B_RSHIFT,       // >>
    B_LSHIFT2,      // <<<
    B_RSHIFT2,      // >>>

    B_LE,           // <=
    B_GRE,          // >=
    B_LESS,         // <    0x90
    B_GR,           // >
    B_EQ,           // ==
    B_EQ2,          // =
    B_NEQ,          // !=

    B_BINOR,        // |
    B_XOR,          // ^
    B_BINAND,       // &

    B_OPENPAR,      // (
    B_CLOSEPAR,     // )
    B_REMARK,       // '    0x9c
    B_COMMA,        // ,
    B_COLON,        // :
    B_SEMICOLON,    // ;    0x9f

    B_ARRAY,        // @    内部コード 配列
    B_STR,          // "    内部コード 文字列

    B_OR,           // ||   B_OR以降のワードはソース表示時に後ろに空白を挿入する
    B_AND,          // &&   B_OR, B_AND, B_THEN, B_TOは前にも空白を挿入する

    B_CALL,         // 0xa2
    B_CLEAR,
    B_CONT,
    B_DATA,
    B_DIM,
    B_END,
    B_FOR,
    B_GOSUB,
    B_GOTO,
    B_IF,
    B_INPUT,
    B_LET,
    B_LIST,
    B_LOAD,

    B_NEW,          // 0xb0
    B_NEXT,
    B_ON,
    B_POKEW,
    B_POKE,
    B_PRINT,
    B_READ,
    B_RESTORE,
    B_RETURN,
    B_RUN,
    B_SAVE,
    B_STOP,
    B_THEN,
    B_TO,

    B_F_PEEKW,      // 0xbe
    B_F_PEEK,
    B_F_ABS,
    B_F_RND,

    B_F_FREE,       // 0xc2
    B_F_TIME,       // 経過時間
    B_F_YEAR,       // 年
    B_F_MONTH,      // 月
    B_F_DAY,        // 日
    B_F_HOUR,       // 時
    B_F_MIN,        // 分
    B_F_SECOND,     // 秒
    B_F_ADC,        // A/D
    B_F_TEMP,       // 温度
    B_F_HUM,        // 湿度
    B_F_PRESS,      // 気圧

    B_NUM,          // 内部コード 数値
    B_HEXNUM,       // 内部コード 16進
    B_BINNUM,       // 内部コード 2進     0xd0
    B_VAR,          // 内部コード 変数

                    //0xd2
    B_EOT,          // end of text
    B_TOL,          // top of line
};

enum {
    B_MODE_STARTRUN = 1,
    B_MODE_CONT,
};

enum {
    B_ERR_NO_ERROR = 0,
    B_ERR_BREAK_IN,
    B_ERR_SYNTAX_ERROR,
    B_ERR_ILLEAGAL_FUNCTION_CALL,
    B_ERR_NEXT_WITHOUT_FOR,
    B_ERR_RETURN_WITHOUT_GOSUB,
    B_ERR_OUT_OF_MEMORY,
    B_ERR_STACK_OVER_FLOW,
    B_ERR_UNDEFINED_LINE,
    B_ERR_BUFFER_OVER_FLOW,
    B_ERR_NO_DATA,
    B_ERR_IO_ERROR,
    B_ERR_UNDEFINED_VARIABLE,
    B_ERR_INDEX_ERROR,
    B_ERR_DUPLICATE,
    B_ERR_DIVIDE_BY_ZERO,
    B_ERR_FORMAT_ERROR,

    B_ERR_BAD_ERROR_CODE
};

struct __LineBuffer {
        uint8_t     inputbuffer[256];   // 入力バッファ
        uint8_t    *pos;
        //~ int len;
        uint8_t     wordbuff[128];      // 中間コード格納バッファ
        uint8_t     wordlen;
};

struct __EditorBuffer {
        uint8_t     textarea[1024*16];
        uint8_t    *eot;        // 末尾
        uint16_t    last;       // 残り容量
        uint8_t    *currtop;    // 実行中の行の先頭
        uint8_t    *currpos;    // 実行中の行の語の位置
        uint8_t     currlen;    // 実行中の行の長さ
        int16_t     currline;   // 実行中の行番号
        int16_t     breakline;  // 現在の停止行
        uint8_t    *breakpoint; // 現在の停止位置
        uint8_t    *readnext;   // READで次に読み込むデータの位置

};

int16_t basic_f_reserved (uint8_t **, int16_t *);   // ダミー関数

int16_t basic_f_peekw (uint8_t **, int16_t *);      // 0xbe
int16_t basic_f_peek (uint8_t **, int16_t *);
int16_t basic_f_abs (uint8_t **, int16_t *);
int16_t basic_f_rnd (uint8_t **, int16_t *);

int16_t basic_f_free (uint8_t **, int16_t *);       // 0xc2
int16_t basic_f_time (uint8_t **, int16_t *);       // 経過時間
int16_t basic_f_year (uint8_t **, int16_t *);       // 年
int16_t basic_f_month (uint8_t **, int16_t *);      // 月
int16_t basic_f_day (uint8_t **, int16_t *);        // 日
int16_t basic_f_hour (uint8_t **, int16_t *);       // 時
int16_t basic_f_min (uint8_t **, int16_t *);        // 分
int16_t basic_f_second (uint8_t **, int16_t *);     // 秒
int16_t basic_f_adc (uint8_t **, int16_t *);        // a/d
int16_t basic_f_temp (uint8_t **, int16_t *);       // 温度
int16_t basic_f_hum (uint8_t **, int16_t *);        // 湿度
int16_t basic_f_press (uint8_t **, int16_t *);      // 気圧








typedef struct __LineBuffer     LineBuffer;
typedef struct __EditorBuffer   EditorBuffer;

int16_t       basic (EditorBuffer *ed, LineBuffer *ln);
int16_t       expression (uint8_t **pos, uint8_t endcode, int16_t *e);
void          __dump (uint8_t *pos, int16_t bytes);

uint8_t      *show_line (uint8_t *pos);
int16_t       str2mid (uint8_t **text, uint8_t *buff, int16_t buffsize);

LineBuffer   *LineBuffer_new (void);
void          LineBuffer_console (LineBuffer *ln, EditorBuffer *ed);
uint8_t      *LineBuffer_get_midbuffer (LineBuffer *ln);
EditorBuffer *EditorBuffer_new (void);
uint8_t      *EditorBuffer_search_line (EditorBuffer *ed, uint16_t linenumber, uint8_t *pos, int16_t *cdx);
void          EditorBuffer_start_message (EditorBuffer *ed);

void          expression_array_init (void);
int16_t       expression_array_setup (EditorBuffer *ed, uint8_t var, int16_t arraysize);
int16_t      *expression_array_search (uint8_t var, int16_t index, int16_t *e);

enum {
    PUTNUM_SIGN = 0x100,
    PUTNUM_ZERO = 0x200
};
void basic_putnum (int16_t n, int16_t keta, uint8_t(*__putnum_sub)(int16_t *n));
uint8_t __putnum_sub_dec (int16_t *n);
uint8_t __putnum_sub_hex (int16_t *n);
uint8_t __putnum_sub_bin (int16_t *n);
int16_t get_number (uint8_t **text, uint8_t t);

void basic_puts (uint8_t *);

