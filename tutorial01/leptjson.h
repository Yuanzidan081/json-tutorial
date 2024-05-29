#ifndef LEPTJSON_H__
#define LEPTJSON_H__

typedef enum
{
    LEPT_NULL,
    LEPT_FALSE,
    LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT
} lept_type;

typedef struct
{
    lept_type type;
} lept_value;

enum
{
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};
/* parse相关的json字符串，返回enum的值，期望的是LEPT_PARSE_OK */
int lept_parse(lept_value *v, const char *json);
/* 从v获得相关的类型，可以是lept_type的任一类型 */
lept_type lept_get_type(const lept_value *v);

#endif /* LEPTJSON_H__ */
