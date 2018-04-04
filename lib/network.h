#ifndef __NETWORK_H
#define __NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

void network_init(void);

#ifdef PLATFORM_ESP8266
void wifi_error(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NETWORK_H */