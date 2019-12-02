#include <stdio.h>
#include <stdint.h>

static uint32_t CRC32Code(const void *data, size_t size)
{
    const uint8_t *d  = data;
    uint32_t      crc = 0xFFFFFFFF;

    while (size--)
    {
        uint32_t index = (crc ^ *(d++)) & 0xFF;

        crc = (crc >> 8) ^ crc_table[index];
    }

    return crc;
}

int main(void)
{
    //
}

