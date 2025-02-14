/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

/** @file
 * Pseudorandom Number generator Wrapper Implementation.
 **/

#include "internal_crypt_lib.h"

/**
 * Sets up the seed value for the pseudorandom number generator.
 *
 * This function sets up the seed value for the pseudorandom number generator.
 * If seed is not NULL, then the seed passed in is used.
 * If seed is NULL, then default seed is used.
 *
 * @param[in]  seed      Pointer to seed value.
 *                      If NULL, default seed is used.
 * @param[in]  seed_size  size of seed value.
 *                      If seed is NULL, this parameter is ignored.
 *
 * @retval true   Pseudorandom number generator has enough entropy for random generation.
 * @retval false  Pseudorandom number generator does not have enough entropy for random generation.
 *
 **/
bool libspdm_random_seed(const uint8_t *seed, uintn seed_size)
{
    /* TBD*/
    return true;
}

/**
 * Generates a pseudorandom byte stream of the specified size.
 *
 * If output is NULL, then return false.
 *
 * @param[out]  output  Pointer to buffer to receive random value.
 * @param[in]   size    size of random bytes to generate.
 *
 * @retval true   Pseudorandom byte stream generated successfully.
 * @retval false  Pseudorandom number generator fails to generate due to lack of entropy.
 *
 **/
bool libspdm_random_bytes(uint8_t *output, uintn size)
{
    return true;
}
