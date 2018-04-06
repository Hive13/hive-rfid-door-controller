#ifndef __DOORBELL_H
#define __DOORBELL_H

#ifdef __cplusplus
extern "C" {
#endif

void doorbell_init(void);
void ring_doorbell(char send_packet);

#ifdef __cplusplus
}
#endif

#endif /* __DOORBELL_H */