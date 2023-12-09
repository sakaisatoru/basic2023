#include <stdint.h>
#include "basic.h"



int16_t (_basic_func)[] = {};






int16_t _var[26];

int16_t factor (uint8_t **pos)
{
    int16_t n = 0, n1 = 0;
    uint8_t c;

    switch (**pos) {
        case B_OPENPAR:
            ++*pos;
            n = expression (pos, B_CLOSEPAR);
            break;
        case B_VAR:
            ++*pos;
            c = **pos - 'A';
            if (c >= 25) {
                //error
                printf ("variable error.\n");
                exit (0);
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
        case B_BINAND:
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

    n = factor (pos);
    while (**pos == B_MUL || **pos == B_DIV || **pos == B_MOD) {
        c = **pos;
        ++*pos;
        n1 = factor (pos);
        switch (c) {
            case B_MUL:
                n *= n1;
                break;
            case B_DIV:
                n /= n1;
                break;
            case B_MOD:
                n %= n1;
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

    n = term6 (pos);
    while (**pos == B_PLUS || **pos == B_MINUS) {
        c = **pos;
        ++*pos;
        n1 = term6 (pos);
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

    n = term5 (pos);
    // 算術シフトは未実装
    while (**pos == B_LSHIFT || **pos == B_RSHIFT) {
        c = **pos;
        ++*pos;
        n1 = term5 (pos);
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

    n = term4 (pos);
    while (**pos == B_LE ||**pos == B_GRE ||**pos == B_LESS ||**pos == B_GR ) {
        c = **pos;
        ++*pos;
        n1 = term4 (pos);
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

    n = term3 (pos);
    while (**pos == B_EQ || **pos == B_NEQ) {
        c = **pos;
        ++*pos;
        n1 = term3 (pos);
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

    n = term2 (pos);
    while (**pos == B_BINAND || **pos == B_BINOR || **pos == B_XOR) {
        c = **pos;
        ++*pos;
        n1 = term2 (pos);
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

int16_t expression (uint8_t **pos, uint8_t endcode)
{
    int16_t n, n1;
    uint8_t c;

    n = term1 (pos);
    while (**pos == B_OR || **pos == B_AND) {
        c = **pos;
        ++*pos;
        n1 = term1 (pos);
        if (c == B_OR) {
            n = (n != 0) | (n1 != 0);
        }else if (c == B_AND) {
            n = (n != 0) & (n1 != 0);
        }
    }

    if (**pos != endcode) {
        switch (**pos) {
            case B_COLON:
            case B_SEMICOLON:
            case B_COMMA:
            case B_TOL:
            case B_EOT:
                return n;
        }
        // error
        printf ("expression error.\n");
        exit (0);
    }
    ++*pos;
    return n;
}

