/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "test_crypt.h"

#define LIBSPDM_RANDOM_NUMBER_SIZE 256

uint8_t m_libspdm_seed_string[] = "This is the random seed for PRNG verification.";

uint8_t m_libspdm_previous_random_buffer[LIBSPDM_RANDOM_NUMBER_SIZE] = { 0x0 };

uint8_t m_libspdm_random_buffer[LIBSPDM_RANDOM_NUMBER_SIZE] = { 0x0 };

/**
 * Validate Crypto pseudorandom number generator interfaces.
 *
 * @retval  RETURN_SUCCESS  Validation succeeded.
 * @retval  RETURN_ABORTED  Validation failed.
 *
 **/
return_status libspdm_validate_crypt_prng(void)
{
    uintn index;
    bool status;

    libspdm_my_print(" \nCrypto PRNG Engine Testing:\n");

    libspdm_my_print("- Random Generation...");

    status = libspdm_random_seed(m_libspdm_seed_string, sizeof(m_libspdm_seed_string));
    if (!status) {
        libspdm_my_print("[Fail]");
        return RETURN_ABORTED;
    }

    for (index = 0; index < 10; index++) {
        status = libspdm_random_bytes(m_libspdm_random_buffer, LIBSPDM_RANDOM_NUMBER_SIZE);
        if (!status) {
            libspdm_my_print("[Fail]");
            return RETURN_ABORTED;
        }

        if (libspdm_const_compare_mem(m_libspdm_previous_random_buffer, m_libspdm_random_buffer,
                                      LIBSPDM_RANDOM_NUMBER_SIZE) == 0) {
            libspdm_my_print("[Fail]");
            return RETURN_ABORTED;
        }

        libspdm_copy_mem(m_libspdm_previous_random_buffer, sizeof(m_libspdm_previous_random_buffer),
                         m_libspdm_random_buffer, LIBSPDM_RANDOM_NUMBER_SIZE);
    }

    libspdm_my_print("[Pass]\n");

    return RETURN_SUCCESS;
}
