
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define IN_UINT8_MV(_p) (*((_p)++))

/*****************************************************************************/
/**
 * decompress an RLE color plane
 * RDP6_BITMAP_STREAM
 */
static int
process_rle_plane(uint8_t* in, int width, int height, uint8_t* out, int size)
{
	int indexw;
	int indexh;
	int code;
	int collen;
	int replen;
	int color;
	int x;
	int revcode;
	uint8_t* last_line;
	uint8_t* this_line;
	uint8_t* org_in;
	uint8_t* org_out;

	org_in = in;
	org_out = out;
	last_line = 0;
	indexh = 0;
	while (indexh < height)
	{
		out = org_out + indexh * width;
		color = 0;
		this_line = out;
		indexw = 0;
		if (last_line == 0)
		{
			while (indexw < width)
			{
				code = IN_UINT8_MV(in);
				replen = code & 0xf;
				collen = (code >> 4) & 0xf;
				revcode = (replen << 4) | collen;
				if ((revcode <= 47) && (revcode >= 16))
				{
					replen = revcode;
					collen = 0;
				}
				while (collen > 0)
				{
					color = IN_UINT8_MV(in);
					*out = color;
					out += 1;
					indexw++;
					collen--;
				}
				while (replen > 0)
				{
					*out = color;
					out += 1;
					indexw++;
					replen--;
				}
			}
		}
		else
		{
			while (indexw < width)
			{
				code = IN_UINT8_MV(in);
				replen = code & 0xf;
				collen = (code >> 4) & 0xf;
				revcode = (replen << 4) | collen;
				if ((revcode <= 47) && (revcode >= 16))
				{
					replen = revcode;
					collen = 0;
				}
				while (collen > 0)
				{
					x = IN_UINT8_MV(in);
					if (x & 1)
					{
						x = x >> 1;
						x = x + 1;
						color = -x;
					}
					else
					{
						x = x >> 1;
						color = x;
					}
					x = last_line[indexw] + color;
					*out = x;
					out += 1;
					indexw++;
					collen--;
				}
				while (replen > 0)
				{
					x = last_line[indexw] + color;
					*out = x;
					out += 1;
					indexw++;
					replen--;
				}
			}
		}
		indexh++;
		last_line = this_line;
	}
	return (int) (in - org_in);
}

/*****************************************************************************/
static int
unsplit4(uint8_t* planes[], uint8_t* dstData, int width, int height)
{
	int index;
	int jndex;
	int pixel;
	int offset;
	int* dst32;

	offset = 0;
	for (jndex = 0; jndex < height; jndex++)
	{
		dst32 = (int*) dstData;
    	dst32 += width * height;
		dst32 -= (jndex + 1) * width;
		for (index = 0; index < width; index++)
		{
			pixel  = planes[0][offset] << 24;
			pixel |= planes[1][offset] << 16;
			pixel |= planes[2][offset] <<  8;
			pixel |= planes[3][offset] <<  0;
			*dst32 = pixel;
			dst32++;
			offset++;
		}
	}
	return 0;
}

/*****************************************************************************/
/**
 * 4 byte bitmap decompress
 * RDP6_BITMAP_STREAM
 */
/* return boolean */
static int
bitmap_decompress4(uint8_t* srcData, uint8_t* dstData, int width, int height,
        int size, uint8_t* temp)
{
	int RLE;
	int code;
	int NoAlpha;
	int bytes_processed;
	int total_processed;
	uint8_t* planes[4];

	planes[0] = temp;
	planes[1] = planes[0] + width * height;
	planes[2] = planes[1] + width * height;
	planes[3] = planes[2] + width * height;
	code = IN_UINT8_MV(srcData);
	RLE = code & 0x10;

	total_processed = 1;
	NoAlpha = code & 0x20;

	if (NoAlpha == 0)
	{
		if (RLE != 0)
		{
			bytes_processed = process_rle_plane(srcData, width, height,
                    planes[0], size - total_processed);
			total_processed += bytes_processed;
			srcData += bytes_processed;
		}
		else
		{
			planes[0] = srcData;
			bytes_processed = width * height;
			total_processed += bytes_processed;
			srcData += bytes_processed;
		}
	}
	else
	{
		memset(planes[0], 0xff, width * height);
	}

	if (RLE != 0)
	{
		bytes_processed = process_rle_plane(srcData, width, height,
                planes[1], size - total_processed);
		total_processed += bytes_processed;
		srcData += bytes_processed;

		bytes_processed = process_rle_plane(srcData, width, height,
                planes[2], size - total_processed);
		total_processed += bytes_processed;
		srcData += bytes_processed;

		bytes_processed = process_rle_plane(srcData, width, height,
                planes[3], size - total_processed);
		total_processed += bytes_processed;
	}
	else
	{
		planes[1] = srcData;
		bytes_processed = width * height;
		total_processed += bytes_processed;
		srcData += bytes_processed;

		planes[2] = srcData;
		bytes_processed = width * height;
		total_processed += bytes_processed;
		srcData += bytes_processed;

		planes[3] = srcData;
		bytes_processed = width * height;
		total_processed += bytes_processed + 1;
	}

	unsplit4(planes, dstData, width, height);

	return (size == total_processed) ? 1 : 0;
}

/*****************************************************************************/
/* return boolean */
int
bitmap_decompress(void* srcData, void* dstData, int width, int height,
        int size, int bpp, void* temp)
{
	if (bpp == 32)
	{
		if (!bitmap_decompress4((uint8_t*)srcData, (uint8_t*)dstData,
                width, height, size, (uint8_t*)temp))
        {
			return 0;
        }
	}
	else
	{
		return 0;
	}
	return 1;
}
