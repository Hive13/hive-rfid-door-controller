#ifndef __API_H
#define __API_H

#define SHA512_SZ 64

#ifdef PLATFORM_ARDUINO
#define RAND_SIZE (2 * sizeof(unsigned long))
#endif
#ifdef PLATFORM_ESP8266
#define RAND_SIZE 16
#endif

#ifdef __cplusplus
extern "C" {
#endif

unsigned char val(char *i);
unsigned char parse_response(char *in, struct cJSON **out, char *rv);
void get_hash(struct cJSON *data, char *sha_buf);
void print_hex(char *str, char *src, unsigned char sz);

char *get_signed_packet(struct cJSON *data);
unsigned char http_request(struct cJSON *data, struct cJSON **result, char *rand);
void add_random_response(struct cJSON *data, char *rand);

#ifdef __cplusplus
}
#endif

#endif /* __API_H */
