
#ifndef __RLECODEC_DECODE_H
#define __RLECODEC_DECODE_H

int
bitmap_decompress(void* srcData, void* dstData, int width, int height,
                  int size, int bpp, void* temp);

#endif
