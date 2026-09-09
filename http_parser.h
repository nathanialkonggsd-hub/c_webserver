#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define MAX_HEADERS 16

// 单个 Header 键值对
typedef struct {
    char key[64];
    char value[128];
} HttpHeader;

// 解析状态定义（扩充 Header 解析状态）
typedef enum {
    PARSE_METHOD = 0,
    PARSE_URL,
    PARSE_VERSION,
    PARSE_LINE_COMPLETE,
    PARSE_HEADER_KEY,
    PARSE_HEADER_VALUE,
    PARSE_ALL_COMPLETE,
    PARSE_ERROR
} ParseState;

// 存储完整请求结果
typedef struct {
    char method[16];
    char url[256];
    char version[16];
    HttpHeader headers[MAX_HEADERS];
    int header_count; // 记录实际解析出了多少个 Header
} HttpRequest;

// 函数声明
int parse_request_line(const char *line, HttpRequest *req);

// Day 2 新增：解析一行 Header，如果是空行（\r\n）则返回 1 表示头部全部结束，成功解析一行返回 0，格式错误返回 -1
int parse_header_line(const char *line, HttpRequest *req);

// 根据 key 获取对应的 value，如果找不到则返回 NULL
// 注意：Header 字段名比较应忽略大小写（可使用 POSIX 标准库函数 strcasecmp）
const char *get_header_value(const HttpRequest *req, const char *key);

#endif