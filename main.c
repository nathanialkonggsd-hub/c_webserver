#include <stdio.h>
#include <string.h>
#include "http_parser.h"

int main() {
    // 模拟一段完整的标准 HTTP 请求报文
    const char *http_lines[] = {
        "GET /api/user/info HTTP/1.1\r\n",
        "Host: 127.0.0.1:8080\r\n",
        "User-Agent: Mozilla/5.0 ConsoleClient\r\n",
        "Accept: application/json\r\n",
        "Connection: keep-alive\r\n",
        "\r\n" // 空行，标志 Header 结束
    };
    int total_lines = sizeof(http_lines) / sizeof(http_lines[0]);

    HttpRequest req;
    memset(&req, 0, sizeof(HttpRequest)); // 全局初始化一次

    printf("=== 1. 解析请求行 ===\n");
    if (parse_request_line(http_lines[0], &req) == 0) {
        printf("[PASS] Method : %s\n", req.method);
        printf("       URL    : %s\n", req.url);
        printf("       Version: %s\n", req.version);
    } else {
        printf("[FAIL] 请求行解析失败\n");
        return -1;
    }

    printf("\n=== 2. 循环解析 Headers ===\n");
    for (int i = 1; i < total_lines; i++) {
        int ret = parse_header_line(http_lines[i], &req);
        if (ret == 0) {
            int cur = req.header_count - 1;
            printf("[HEADER %d] Key: [%s]  -->  Value: [%s]\n", 
                   cur + 1, req.headers[cur].key, req.headers[cur].value);
        } else if (ret == 1) {
            printf("[END] 检测到报文头部结束空行 (CRLF)，准备解析 Body\n");
            break;
        } else {
            printf("[FAIL] 第 %d 行 Header 解析出错！\n", i);
            return -1;
        }
    }

    printf("\n=== 统计结果 ===\n");
    printf("成功提取有效 Header 数量: %d\n", req.header_count);

    const char *host = get_header_value(&req, "host");
    const char *agent = get_header_value(&req, "User-AGENT");
    printf("Found Host: %s\n", host ? host : "Not Found");
    printf("Found Agent: %s\n", agent ? agent : "Not Found");
    return 0;
}