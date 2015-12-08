#ifndef Sha256_h
#define Sha256_h

#define HASH_LENGTH 32
#define BLOCK_LENGTH 64
#define BUFFER_SIZE 64
#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c

#ifdef __cplusplus
extern "C" {
#endif

union _buffer {
  uint8_t b[BLOCK_LENGTH];
  uint32_t w[BLOCK_LENGTH/4];
};
union _state {
  uint8_t b[HASH_LENGTH];
  uint32_t w[HASH_LENGTH/4];
};

void hmac_sha256_init(unsigned char *secret, size_t len);
size_t sha256_hmac_write(unsigned char *data, size_t len);
void sha1_init(void);
size_t sha256_write(unsigned char);
void sha256_print(char *str);
unsigned char *sha256_result(void);
unsigned char *hmac_sha256_result(void);

#ifdef __cplusplus
}
#endif

#endif
