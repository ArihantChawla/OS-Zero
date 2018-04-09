#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <v0/vm.h>

void v0readkbd(struct v0 *vm, uint16_t port, uint8_t reg);
void v0writetty(struct v0 *vm, uint16_t port, uint8_t reg);
void v0readrtc(struct v0 *vm, uint16_t port, uint8_t reg);
void v0readtmr(struct v0 *vm, uint16_t port, uint8_t reg);
void v0writevtd(struct v0 *vm, uint16_t port, uint8_t reg);

