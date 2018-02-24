#ifndef __API_H
#define __API_H

#define SHA512_SZ 64

#define RESPONSE_ACCESS_DENIED 4
#define RESPONSE_BAD_JSON      3
#define RESPONSE_BAD_HTTP      2
#define RESPONSE_BAD_CKSUM     1
#define RESPONSE_GOOD          0

#ifdef __cplusplus
extern "C" {
#endif

unsigned char val(char *i);
unsigned char parse_response(char *in, struct cJSON **out, char *key, unsigned char key_len, char *rv, unsigned char rv_len);
void get_hash(struct cJSON *data, char *sha_buf, char *key, unsigned char key_len);
char *get_request(unsigned long badge_num, char *operation, char *location, char *device, char *key, unsigned char key_len, char *rv, unsigned char rv_len, char * nonce);
char *log_data(struct cJSON *l_data, char *device, char *key, unsigned char key_len, char *rv, unsigned char rv_len, char *nonce);
void print_hex(char *str, char *src, unsigned char sz);
char *get_nonce(char *device, char *key, unsigned char key_len, char *rv, unsigned char rv_len);

#ifdef __cplusplus
}
#endif

#endif /* __API_H */
