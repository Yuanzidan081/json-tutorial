#include "leptjson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* NULL, strtod() */
#include <errno.h>  /* errno, ERANGE */
#include <math.h>   /* HUGE_VAL */

#define EXPECT(c, ch)             \
    do                            \
    {                             \
        assert(*c->json == (ch)); \
        c->json++;                \
    } while (0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef enum
{
    LEPT_INIT = 0,
    LEPT_SIGN,
    LEPT_DIGIT_BEFORE_DOT_FIRST_ZERO,
    LEPT_DIGIT_BEFORE_DOT,
    LEPT_DOT,
    LEPT_DIGIT_AFTER_DOT,
    LEPT_EXPONENTIAL,
    LEPT_SIGN_AFTER_EXPONENTIAL,
    LEPT_EXPONENTIAL_DIGIT_FIRST_ZERO,
    LEPT_EXPONENTIAL_DIGIT,
    LEPT_INVALID,
    LEPT_NOT_SINGULAR
} lept_number_state;

typedef struct
{
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c)
{
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, const char *expect, lept_type expect_type)
{
    while (c->json[0] != '\0' && *expect != '\0')
    {
        if (c->json[0] != *expect)
            return LEPT_PARSE_INVALID_VALUE;
        c->json++;
        expect++;
    }
    if (*expect != '\0')
        return LEPT_PARSE_INVALID_VALUE;
    v->type = expect_type;
    return LEPT_PARSE_OK;
}
static int lept_parse_number(lept_context *c, lept_value *v)
{
    int state = LEPT_INIT;
    const char *p = c->json;
    /* 使用状态机来写 */
    while (state != LEPT_INVALID && state != LEPT_NOT_SINGULAR && *p != '\0')
    {
        switch (state)
        {
        case LEPT_INIT:
        {
            if (*p == '-')
                state = LEPT_SIGN;
            else if (*p == '0')
                state = LEPT_DIGIT_BEFORE_DOT_FIRST_ZERO;
            else if (ISDIGIT1TO9(*p))
                state = LEPT_DIGIT_BEFORE_DOT;
            else
                state = LEPT_INVALID;
            break;
        }
        case LEPT_SIGN:
        {
            if (*p == '0')
                state = LEPT_DIGIT_BEFORE_DOT_FIRST_ZERO;
            else if (ISDIGIT1TO9(*p))
                state = LEPT_DIGIT_BEFORE_DOT;
            else
                state = LEPT_INVALID;
            break;
        }
        case LEPT_DIGIT_BEFORE_DOT_FIRST_ZERO:
        {
            if (*p == '.')
                state = LEPT_DOT;
            else if (*p == 'e' || *p == 'E')
                state = LEPT_EXPONENTIAL;
            else
                state = LEPT_NOT_SINGULAR;
            break;
        }
        case LEPT_DIGIT_BEFORE_DOT:
        {
            if (ISDIGIT(*p))
                state = LEPT_DIGIT_BEFORE_DOT;
            else if (*p == '.')
                state = LEPT_DOT;
            else if (*p == 'e' || *p == 'E')
                state = LEPT_EXPONENTIAL;
            else
                state = LEPT_INVALID;
            break;
        }
        case LEPT_DOT:
        {
            if (ISDIGIT(*p))
                state = LEPT_DIGIT_AFTER_DOT;
            else
                state = LEPT_INVALID;
            break;
        }
        case LEPT_DIGIT_AFTER_DOT:
        {
            if (ISDIGIT(*p))
                state = LEPT_DIGIT_AFTER_DOT;
            else if (*p == 'e' || *p == 'E')
                state = LEPT_EXPONENTIAL;
            else
                state = LEPT_INVALID;
            break;
        }
        case LEPT_EXPONENTIAL:
        {
            if (*p == '+' || *p == '-')
                state = LEPT_SIGN_AFTER_EXPONENTIAL;
            else if (*p == '0')
                state = LEPT_EXPONENTIAL_DIGIT_FIRST_ZERO;
            else if (ISDIGIT1TO9(*p))
                state = LEPT_EXPONENTIAL_DIGIT;
            else
                state = LEPT_INVALID;
            break;
        }
        case LEPT_SIGN_AFTER_EXPONENTIAL:
        {
            if (*p == '0')
                state = LEPT_EXPONENTIAL_DIGIT_FIRST_ZERO;
            else if (ISDIGIT1TO9(*p))
                state = LEPT_EXPONENTIAL_DIGIT;
            else
                state = LEPT_INVALID;
        }
        case LEPT_EXPONENTIAL_DIGIT_FIRST_ZERO:
        {
            state = LEPT_NOT_SINGULAR;
        }
        case LEPT_EXPONENTIAL_DIGIT:
        {
            if (ISDIGIT(*p))
                state = LEPT_EXPONENTIAL_DIGIT;
            else
                state = LEPT_INVALID;
        }
        default:
            break;
        }
        ++p;
    }
    if (state == LEPT_NOT_SINGULAR)
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    if (state == LEPT_INVALID || state == LEPT_SIGN ||
        state == LEPT_DOT || state == LEPT_EXPONENTIAL ||
        state == LEPT_SIGN_AFTER_EXPONENTIAL)
        return LEPT_PARSE_INVALID_VALUE;
    errno = 0; /* 一个全局变量 */
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context *c, lept_value *v)
{
    switch (*c->json)
    {
    case 't':
        return lept_parse_literal(c, v, "true", LEPT_TRUE);
    case 'f':
        return lept_parse_literal(c, v, "false", LEPT_FALSE);
    case 'n':
        return lept_parse_literal(c, v, "null", LEPT_NULL);

    default:
        return lept_parse_number(c, v);
    case '\0':
        return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value *v, const char *json)
{
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK)
    {
        lept_parse_whitespace(&c);
        if (*c.json != '\0')
        {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
