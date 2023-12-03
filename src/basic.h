enum {
B_NEG = 0x81,      // ~
B_NOT,      // !
B_INC,      // ++
B_DEC,      // --

B_MUL,      // *
B_DIV,      // /
B_MOD,       // %

B_PLUS,     // +
B_MINUS,    // -

B_LSHIFT,   // <<
B_RSHIFT,   // >>
B_LSHIFT2,  // <<<
B_RSHIFT2,  // >>>

B_LE,       // <=
B_GRE,      // >=
B_LESS,     // <
B_GR,       // >
B_EQ,       // ==
B_EQ2,      // =
B_NEQ,      // !=

B_BINOR,    // |
B_XOR,      // ^
B_BINAND,   // &

B_OR,       // ||
B_AND,      // &&

B_OPENPAR,  // (
B_CLOSEPAR, // )
B_REMARK,   // '
};


enum {
B_BREAK = 0xa0,
B_CALL,
B_CONTINUE,
B_CONT,
B_END,
B_FOR,
B_FREE,
B_GOSUB,

B_GOTO,
B_IF,
B_INPUT,
B_LET,
B_LIST,
B_LOAD,
B_NEW,
B_NEXT,

B_POKEW,
B_POKE,
B_PRINT,
B_RETURN,
B_RUN,
B_SAVE,
B_STOP,
B_THEN,
B_TO,
B_F_PEEKW,

B_F_PEEK,
B_F_ABS,
B_F_TIME,   // 経過時間
B_F_YEAR,   // 年
B_F_MONTH,  // 月
B_F_DAY,    // 日
B_F_HOUR,   // 時
B_F_MIN,    // 分
// 0xc2
B_F_SECOND, // 秒
B_F_ADC,    // A/D
B_F_TEMP,   // 温度
B_F_HUM,    // 湿度
B_F_PRESS,  // 気圧

//0xc7
B_NUM,      // 内部コード 数値
B_HEXNUM,      // 内部コード 数値
B_BINNUM,      // 内部コード 数値
B_VAR,      // 内部コード 変数
B_ARRAY,    // 内部コード　配列
B_STR,      // 内部コード 文字列 "
B_COMMA,    // ,
B_COLON,    // :
B_SEMICOLON,// ;
B_EOL,      // eol
};


