#include "../common.h"
#include "yaz.h"

int yaz_is_compressed(bin_t* in)
{
    yaz_header_t* yaz_header = (yaz_header_t*)in->buffer;
    return !strncmp(yaz_header->id, "Yaz0", 4) || !(strncmp(yaz_header->id, "Yaz1", 4));
}

int yaz_decompress(bin_t* in, bin_t* out)
{
    yaz_header_t* yaz_header = (yaz_header_t*)in->buffer;
    if (in->size < sizeof(yaz_header_t))
        return YAZ_INVALID_SIZE;
    if (!yaz_is_compressed(in))
        return YAZ_INVALID_HEADER;

    size_t decompressed_size = bswap_int32(yaz_header->size_decompressed);
    out->size = decompressed_size;
    out->buffer = malloc(decompressed_size);

    uint8_t* src = in->buffer + sizeof(yaz_header_t);
    uint8_t* dst = out->buffer;
    size_t src_pos = 0;
    size_t dst_pos = 0;

    while (dst_pos < decompressed_size) 
    {
        uint8_t code_byte = src[src_pos++];
        for (int i = 0; i < 8 && dst_pos < decompressed_size; i++) 
        {
            if (code_byte & (0x80 >> i)) 
            {
                dst[dst_pos++] = src[src_pos++];
            }
            else 
            {
                uint16_t encode = bswap_uint16(*(uint16_t*)(src + src_pos));
                src_pos += 2;

                int seekback = (encode & 0x0FFF) + 1;
                int n = (encode & 0xF000) >> 12;
                if (n == 0)
                    n = (uint8_t)src[src_pos++] + 0x12;
                else
                    n += 2;

                for (int j = 0; j < n && dst_pos < decompressed_size; j++) 
                {
                    dst[dst_pos] = dst[dst_pos - seekback];
                    dst_pos++;
                }
            }
        }
    }

    return YAZ_OK;
}