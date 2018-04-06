#ifndef __HTTP_H
#define __HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

signed int http_get_json(char *host, char *path, char *req_body, char **resp);
signed int http_get(char *req, char *host, char *path);

#ifdef __cplusplus
};
#endif

#endif /* __HTTP_H */
