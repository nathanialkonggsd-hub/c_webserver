#include <string.h>
#include <strings.h>
#include "http_parser.h"
int parse_request_line(const char *line, HttpRequest *req)
{
    if(!line || !req)
        return -1;
    memset(req, 0, sizeof(HttpRequest));
    ParseState state = PARSE_METHOD;
    int idx = 0;
    const char *p = line;
    while(*p != '\0')
    {
        switch (state)
        {
            case PARSE_METHOD:
                if(*p != ' ')
                {
                    if(*p >= 'A' && *p <= 'Z')
                    {
                        if(idx < (int)sizeof(req->method) - 1)
                            req->method[idx++] = *p;
                        else
                            state = PARSE_ERROR;
                    }
                    else
                        state = PARSE_ERROR;
                }
                else if (idx == 0) break;
                else
                {
                    req->method[idx] = '\0';
                    state = PARSE_URL;
                    idx = 0;
                }
                break;
            case PARSE_URL:
                if(*p != ' ')
                {
                    if(idx < (int)sizeof(req->url) - 1)
                        req->url[idx++] = *p;
                    else
                        state = PARSE_ERROR;
                }
                else if (idx == 0) break;
                else
                {
                    req->url[idx] = '\0';
                    state = PARSE_VERSION;
                    idx = 0;
                }
                break;
            case PARSE_VERSION:
                if(*p != '\r' && *p != '\n')
                {
                    if(idx < (int)sizeof(req->version) - 1)
                    {
                        if(*p != ' ')
                            req->version[idx++] = *p;
                    }
                    else
                        state = PARSE_ERROR;
                }
                else
                {
                    req->version[idx] = '\0';
                    state = PARSE_LINE_COMPLETE;
                    idx = 0;
                }
                break;
            default:
                break;
        }
        if (state == PARSE_ERROR || state == PARSE_LINE_COMPLETE) 
            break;
        p++;
    }
    if(state ==PARSE_LINE_COMPLETE)
        return 0;
    else
        return -1;
}
//Headers解析
int parse_header_line(const char *line, HttpRequest *req)
{
    if(!line || !req)
        return -1;
    ParseState state = PARSE_HEADER_KEY;
    int idx = 0;
    const char *p = line;
    if(*p == '\r' || *p == '\n')
    {
        state = PARSE_ALL_COMPLETE;
        return 1;
    }
    if(req->header_count >= MAX_HEADERS)
    {
        state = PARSE_ERROR;
        return -1;
    }
    while(*p != '\0')
    {
        switch (state)
        {
            case PARSE_HEADER_KEY:
                if(*p == ' ' || *p == '\r' || *p == '\n')
                {
                    state = PARSE_ERROR;
                    break;
                }
                else if(*p == ':')
                {
                    if(idx == 0)
                    {
                        state = PARSE_ERROR;
                        break;
                    }
                    else
                    {
                        req->headers[req->header_count].key[idx] = '\0';
                        state = PARSE_HEADER_VALUE;
                        idx = 0;
                        break;
                    }
                }
                else
                    {
                        if(idx < (int)sizeof(req->headers[0].key) - 1)
                        {
                            req->headers[req->header_count].key[idx++] = *p;
                            break;
                        }
                        else
                        {
                            state = PARSE_ERROR;
                            break;
                        }
                    }
            case PARSE_HEADER_VALUE:
                if(*p == ' ' && idx == 0)
                    break;
                else if(*p == '\r')
                {
                    if(*(p+1) != '\n')
                    {
                        state = PARSE_ERROR;
                        break;
                    }
                    else
                        {
                            while(idx > 0 && (req->headers[req->header_count].value[idx - 1] == ' ' || 
                                            req->headers[req->header_count].value[idx - 1] == '\t'))
                            {
                                idx--;
                            }
                            req->headers[req->header_count].value[idx] = '\0';
                            state = PARSE_LINE_COMPLETE;
                            idx = 0;
                            break;
                        }
                }
                else if(*p == '\n')
                {
                    if(*(p+1) == '\r')
                    {
                        state = PARSE_ERROR;
                        break;
                    }
                    else
                    {
                        while(idx > 0 && (req->headers[req->header_count].value[idx - 1] == ' ' || 
                                        req->headers[req->header_count].value[idx - 1] == '\t'))
                        {
                            idx--;
                        }
                        req->headers[req->header_count].value[idx] = '\0';
                        state = PARSE_LINE_COMPLETE;
                        idx = 0;
                        break;
                    }
                }
                else
                {
                    if(idx < (int)sizeof(req->headers[0].value) - 1)
                    {
                        req->headers[req->header_count].value[idx++] = *p;
                        break;
                    }
                    else
                    {
                        state = PARSE_ERROR;
                        break;
                    }
                }
        }
        if(state == PARSE_LINE_COMPLETE || state == PARSE_ERROR)
            break;
        p++;
    }
    if(state == PARSE_LINE_COMPLETE)
    {
        req->header_count++;
        return 0;
    }
    else
        return -1;
}

// 根据 key 获取对应的 value，如果找不到则返回 NULL
// 注意：Header 字段名比较应忽略大小写（可使用 POSIX 标准库函数 strcasecmp）
const char *get_header_value(const HttpRequest *req, const char *key)
{
    if(!req || !key)
        return NULL;
    int i = 0;
    for(i = 0;i < req->header_count;i++)
    {
        if(strcasecmp(req->headers[i].key,key) == 0)
        {
            return req->headers[i].value;
        }
    }
    return NULL;
}