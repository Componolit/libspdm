/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

/** @file
 * libspdm_zero_mem() implementation.
 **/

#include "base.h"

/**
 * Fills a target buffer with zeros, and returns the target buffer.
 *
 * This function fills length bytes of buffer with zeros, and returns buffer.
 *
 * If length > 0 and buffer is NULL, then LIBSPDM_ASSERT().
 * If length is greater than (MAX_ADDRESS - buffer + 1), then LIBSPDM_ASSERT().
 *
 * @param  buffer      The pointer to the target buffer to fill with zeros.
 * @param  length      The number of bytes in buffer to fill with zeros.
 *
 * @return buffer.
 *
 **/
void *libspdm_zero_mem(void *buffer, uintn length)
{
    volatile uint8_t *pointer;

    pointer = (uint8_t *)buffer;
    while (length-- != 0) {
        *(pointer++) = 0;
    }

    return buffer;
}
