#ifndef __V0_VM_H__
#define __V0_VM_H__

/* I/O function prototypes */
void v0readkbd(struct v0 *vm, uint16_t port, uint8_t reg);
void v0writetty(struct v0 *vm, uint16_t port, uint8_t reg);
void v0readrtc(struct v0 *vm, uint16_t port, uint8_t reg);
void v0readtmr(struct v0 *vm, uint16_t port, uint8_t reg);
void v0writevtd(struct v0 *vm, uint16_t port, uint8_t reg);

#endif /* __V0_VM_H__ */

