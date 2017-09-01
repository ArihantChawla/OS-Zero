#ifndef __V0_IO_H__
#define __V0_IO_H__

void v0readkbd(struct v0 *vm, uint16_t port, uint8_t reg);
void v0writetty(struct v0 *vm, uint16_t port, uint8_t reg);
void v0readrtc(struct v0 *vm, uint16_t port, uint8_t reg);
void v0readtmr(struct v0 *vm, uint16_t port, uint8_t reg);

#endif /* __V0_IO_H__ */

