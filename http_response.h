#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stddef.h>
#include "http_parser.h"

// 静态根目录路径常量
#define STATIC_ROOT "./www"

// 响应结构体定义
typedef struct {
    int status_code;             // 状态码：200, 403, 404 等
    const char *status_text;     // "OK", "Forbidden", "Not Found"
    char content_type[64];       // "text/html", "text/css", "image/png" 等
    long content_length;         // 静态文件字节大小
    int file_fd;                 // 目标文件描述符（若为错误页面则置为 -1）
    char *body_data;             // 内存中的正文字符串（用于错误信息或内存数据，可选）
} HttpResponse;

// 1. 根据文件后缀推导 MIME 类型
const char *get_mime_type(const char *path);

// 2. 核心处理接口：安全映射 URL 到磁盘文件，执行路径防御与 stat/open 检查，并填充 HttpResponse 结构体
int handle_static_request(const HttpRequest *req, HttpResponse *res);

// 3. 构建 HTTP 响应头部到用户缓冲区，返回写入的总字节数
int build_response_headers(const HttpResponse *res, char *buffer, size_t buf_size);

// 4. 清理与释放资源（如关闭打开的 file_fd）
void free_response(HttpResponse *res);

#endif