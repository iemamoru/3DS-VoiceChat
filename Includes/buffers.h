#ifndef BUFFERS_H
#define BUFFERS_H

#include <3ds.h>

extern u8 *soundBuffer;
extern u8 *micBuffer;

constexpr u32 SOC_BUFFER_ADDR = 0x7500000;
constexpr u32 SOC_BUFFER_SIZE = 0x100000;

#endif // BUFFERS_H
