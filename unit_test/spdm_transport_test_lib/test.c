/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "library/spdm_transport_test_lib.h"

#define LIBSPDM_TEST_ALIGNMENT 4
#define TEST_SEQUENCE_NUMBER_COUNT 2
#define TEST_MAX_RANDOM_NUMBER_COUNT 32

/**
 * Get sequence number in an SPDM secure message.
 *
 * This value is transport layer specific.
 *
 * @param sequence_number        The current sequence number used to encode or decode message.
 * @param sequence_number_buffer  A buffer to hold the sequence number output used in the secured message.
 *                             The size in byte of the output buffer shall be 8.
 *
 * @return size in byte of the sequence_number_buffer.
 *        It shall be no greater than 8.
 *        0 means no sequence number is required.
 **/
uint8_t libspdm_test_get_sequence_number(uint64_t sequence_number,
                                         uint8_t *sequence_number_buffer)
{
    libspdm_copy_mem(sequence_number_buffer, TEST_SEQUENCE_NUMBER_COUNT,
                     &sequence_number, TEST_SEQUENCE_NUMBER_COUNT);
    return TEST_SEQUENCE_NUMBER_COUNT;
}

/**
 * Return max random number count in an SPDM secure message.
 *
 * This value is transport layer specific.
 *
 * @return Max random number count in an SPDM secured message.
 *        0 means no randum number is required.
 **/
uint32_t libspdm_test_get_max_random_number_count(void)
{
    return TEST_MAX_RANDOM_NUMBER_COUNT;
}

/**
 * Encode a normal message or secured message to a transport message.
 *
 * @param  session_id                    Indicates if it is a secured message protected via SPDM session.
 *                                     If session_id is NULL, it is a normal message.
 *                                     If session_id is NOT NULL, it is a secured message.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a source buffer to store the message.
 * @param  transport_message_size         size in bytes of the transport message data buffer.
 * @param  transport_message             A pointer to a destination buffer to store the transport message.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
return_status libspdm_test_encode_message(const uint32_t *session_id, uintn message_size,
                                          const void *message,
                                          uintn *transport_message_size,
                                          void *transport_message)
{
    uintn aligned_message_size;
    uintn alignment;
    libspdm_test_message_header_t *test_message_header;
    uintn init_transport_message_size;

    init_transport_message_size = *transport_message_size;
    alignment = LIBSPDM_TEST_ALIGNMENT;
    aligned_message_size =
        (message_size + (alignment - 1)) & ~(alignment - 1);

    LIBSPDM_ASSERT(*transport_message_size >=
                   aligned_message_size + sizeof(libspdm_test_message_header_t));
    if (*transport_message_size <
        aligned_message_size + sizeof(libspdm_test_message_header_t)) {
        *transport_message_size =
            aligned_message_size + sizeof(libspdm_test_message_header_t);
        return RETURN_BUFFER_TOO_SMALL;
    }
    *transport_message_size =
        aligned_message_size + sizeof(libspdm_test_message_header_t);
    test_message_header = transport_message;
    if (session_id != NULL) {
        test_message_header->message_type =
            LIBSPDM_TEST_MESSAGE_TYPE_SECURED_TEST;
        LIBSPDM_ASSERT(*session_id == *(uint32_t *)(message));
        if (*session_id != *(uint32_t *)(message)) {
            return RETURN_UNSUPPORTED;
        }
    } else {
        test_message_header->message_type = LIBSPDM_TEST_MESSAGE_TYPE_SPDM;
    }
    libspdm_copy_mem((uint8_t *)transport_message + sizeof(libspdm_test_message_header_t),
                     init_transport_message_size - sizeof(libspdm_test_message_header_t),
                     message, message_size);
    libspdm_zero_mem((uint8_t *)transport_message + sizeof(libspdm_test_message_header_t) +
                     message_size,
                     *transport_message_size - sizeof(libspdm_test_message_header_t) -
                     message_size);
    return RETURN_SUCCESS;
}

/**
 * Decode a transport message to a normal message or secured message.
 *
 * @param  session_id                    Indicates if it is a secured message protected via SPDM session.
 *                                     If *session_id is NULL, it is a normal message.
 *                                     If *session_id is NOT NULL, it is a secured message.
 * @param  transport_message_size         size in bytes of the transport message data buffer.
 * @param  transport_message             A pointer to a source buffer to store the transport message.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a destination buffer to store the message.
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
return_status libspdm_test_decode_message(uint32_t **session_id,
                                          uintn transport_message_size,
                                          const void *transport_message,
                                          uintn *message_size, void *message)
{
    uintn alignment;
    const libspdm_test_message_header_t *test_message_header;
    uintn init_message_size;

    init_message_size = *message_size;
    alignment = LIBSPDM_TEST_ALIGNMENT;

    LIBSPDM_ASSERT(transport_message_size > sizeof(libspdm_test_message_header_t));
    if (transport_message_size <= sizeof(libspdm_test_message_header_t)) {
        return RETURN_UNSUPPORTED;
    }

    test_message_header = transport_message;

    switch (test_message_header->message_type) {
    case LIBSPDM_TEST_MESSAGE_TYPE_SECURED_TEST:
        LIBSPDM_ASSERT(session_id != NULL);
        if (session_id == NULL) {
            return RETURN_UNSUPPORTED;
        }
        if (transport_message_size <=
            sizeof(libspdm_test_message_header_t) + sizeof(uint32_t)) {
            return RETURN_UNSUPPORTED;
        }
        *session_id = (uint32_t *)((uint8_t *)transport_message +
                                   sizeof(libspdm_test_message_header_t));
        break;
    case LIBSPDM_TEST_MESSAGE_TYPE_SPDM:
        if (session_id != NULL) {
            *session_id = NULL;
        }
        break;
    default:
        return RETURN_UNSUPPORTED;
    }

    LIBSPDM_ASSERT(((transport_message_size - sizeof(libspdm_test_message_header_t)) &
                    (alignment - 1)) == 0);

    if (*message_size <
        transport_message_size - sizeof(libspdm_test_message_header_t)) {

        /* Handle special case for the side effect of alignment
         * Caller may allocate a good enough buffer without considering alignment.
         * Here we will not copy all the message and ignore the the last padding bytes.*/

        if (*message_size + alignment - 1 >=
            transport_message_size - sizeof(libspdm_test_message_header_t)) {
            libspdm_copy_mem(message, init_message_size,
                             (uint8_t *)transport_message + sizeof(libspdm_test_message_header_t),
                             *message_size);
            return RETURN_SUCCESS;
        }
        *message_size =
            transport_message_size - sizeof(libspdm_test_message_header_t);
        LIBSPDM_ASSERT(*message_size >=
                       transport_message_size - sizeof(libspdm_test_message_header_t));
        return RETURN_BUFFER_TOO_SMALL;
    }
    *message_size = transport_message_size - sizeof(libspdm_test_message_header_t);
    libspdm_copy_mem(message, init_message_size,
                     (uint8_t *)transport_message + sizeof(libspdm_test_message_header_t),
                     *message_size);
    return RETURN_SUCCESS;
}
