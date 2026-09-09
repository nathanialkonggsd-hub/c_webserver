#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
// 1. 根据文件后缀推导 MIME 类型
const char *get_mime_type(const char *path)
{
    if(!path) return "application/octet-stream";
    char *dot = strrchr(path, '.');
    if(dot == NULL) return "application/octet-stream";
    const char *ext = dot;
    else if(!strcasecmp(ext,".html") || !strcasecmp(ext,".htm")) return "text/html; charset=utf-8";
    else if(!strcasecmp(ext,".css")) return "text/css";
    else if(!strcasecmp(ext,".js")) return "application/javascript";
    else if(!strcasecmp(ext,".png")) return "image/png";
    else if(!strcasecmp(ext,".jpg") || !strcasecmp(ext,".jpeg")) return "image/jpeg";
    else return "application/octet-stream";
}

// 2. 核心处理接口：安全映射 URL 到磁盘文件，执行路径防御与 stat/open 检查，并填充 HttpResponse 结构体
int handle_static_request(const HttpRequest *req, HttpResponse *res)
{
    if(!req || !res) return 0;
    memset(res, 0, sizeof(HttpResponse));
    res->file_fd = -1;
    if(strcmp(req->method, "GET"))
    {
        res->status_code = 405;
        res->status_text = "Method Not Allowed";
        return 0;
    }
    char full_path[512];
    int written = snprintf(full_path, sizeof(full_path), "%s%s", STATIC_ROOT, subpath);
    if (written >= (int)sizeof(full_path)) {
        res->status_code = 414; // URI Too Long
        res->status_text = "URI Too Long";
        return 0;
    }
    // realpath 会解析所有软链接和 .. ，生成规范化的绝对物理路径
    char resolved_path[PATH_MAX];
    if(realpath(full_path, resolved_path) == NULL)
    {
        res->status_code = 404;
        res->status_text = "Not Found";
        return 0;
    }
    // 获取网站静态根目录的绝对路径
    char root_path[PATH_MAX];
    realpath("./www", root_path);
    // 核心安全校验：目标文件的绝对路径必须以网站根目录作为前缀！
    if(strnamp(resolved_path, root_path, strlen(root_path)) != 0)
    {
        res->status_code = 403;
        res->status_text = "Forbidden";
        return 0;
    }
    struct stat st;
    if(stat(full_path, &st) < 0)
    {
        if(errno == ENOENT)
        {
            res->status_code = 404;
            res->status_text = "Not Found";
            return 0;
        }
        res->status_code = 500;
        res->status_text = "Internal Server Error";
        return 0;
    }
    // 2. 检查是否是普通文件（防御目录遍历：如果是目录则不能直接 read）
    if (!S_ISREG(st.st_mode))
    {
        res->status_code = 403;
        res->status_text = "Forbidden";
        return 0;
    }

    // 3. 检查 Other 用户是否有读权限
    if (!(st.st_mode & S_IROTH))
    {
        res->status_code = 403;
        res->status_text = "Forbidden";
        return 0;
    }
    int fd = open(full_path, O_RDONLY | O_CLOEXEC);
    if(fd < 0)
    {
        res->status_code = 500;
        res->status_text = "Internal Server Error";
        return 0;
    }
    char buf[1024];
    ssize_t bytes_read = read(fd, buf, sizeof(buf));
    if(bytes_read < 0)
    {
        close(fd);
        res->status_code = 500;
        res->status_text = "Internal Server Error";
        return 0;
    }
    res->status_code = 200;
    res->status_text = "OK";
    res->content_length = st.st_size;
    res->file_fd = fd;
    strncpy(res->content_type, get_mime_type(full_path), sizeof(res->content_type) - 1);
    return 0;
}

// 3. 构建 HTTP 响应头部到用户缓冲区，返回写入的总字节数
int build_response_headers(const HttpResponse *res, char *buffer, size_t buf_size);

// 4. 清理与释放资源（如关闭打开的 file_fd）
void free_response(HttpResponse *res);