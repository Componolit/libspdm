/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "internal/libspdm_secured_message_lib.h"

/**
 * Encode an application message to a secured message.
 *
 * @param  spdm_secured_message_context    A pointer to the SPDM secured message context.
 * @param  session_id                    The session ID of the SPDM session.
 * @param  is_requester                  Indicates if it is a requester message.
 * @param  app_message_size               size in bytes of the application message data buffer.
 * @param  app_message                   A pointer to a source buffer to store the application message.
 * @param  secured_message_size           size in bytes of the secured message data buffer.
 * @param  secured_message               A pointer to a destination buffer to store the secured message.
 * @param  spdm_secured_message_callbacks  A pointer to a secured message callback functions structure.
 *
 * @retval RETURN_SUCCESS               The application message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
return_status libspdm_encode_secured_message(
    void *spdm_secured_message_context, uint32_t session_id,
    bool is_requester, uintn app_message_size,
    const void *app_message, uintn *secured_message_size,
    void *secured_message,
    const libspdm_secured_message_callbacks_t *spdm_secured_message_callbacks)
{
    libspdm_secured_message_context_t *secured_message_context;
    uintn total_secured_message_size;
    uintn plain_text_size;
    uintn cipher_text_size;
    uintn aead_pad_size;
    uintn aead_tag_size;
    uintn aead_key_size;
    uintn aead_iv_size;
    uint8_t *a_data;
    uint8_t *enc_msg;
    uint8_t *dec_msg;
    uint8_t *tag;
    spdm_secured_message_a_data_header1_t *record_header1;
    spdm_secured_message_a_data_header2_t *record_header2;
    uintn record_header_size;
    spdm_secured_message_cipher_header_t *enc_msg_header;
    bool result;
    uint8_t key[LIBSPDM_MAX_AEAD_KEY_SIZE];
    uint8_t salt[LIBSPDM_MAX_AEAD_IV_SIZE];
    uint64_t sequence_number;
    uint64_t sequence_num_in_header;
    uint8_t sequence_num_in_header_size;
    libspdm_session_type_t session_type;
    uint32_t rand_count;
    uint32_t max_rand_count;
    libspdm_session_state_t session_state;

    secured_message_context = spdm_secured_message_context;

    session_type = secured_message_context->session_type;
    LIBSPDM_ASSERT((session_type == LIBSPDM_SESSION_TYPE_MAC_ONLY) ||
                   (session_type == LIBSPDM_SESSION_TYPE_ENC_MAC));
    session_state = secured_message_context->session_state;
    LIBSPDM_ASSERT((session_state == LIBSPDM_SESSION_STATE_HANDSHAKING) ||
                   (session_state == LIBSPDM_SESSION_STATE_ESTABLISHED));

    aead_tag_size = secured_message_context->aead_tag_size;
    aead_key_size = secured_message_context->aead_key_size;
    aead_iv_size = secured_message_context->aead_iv_size;

    switch (session_state) {
    case LIBSPDM_SESSION_STATE_HANDSHAKING:
        if (is_requester) {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->handshake_secret
                             .request_handshake_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->handshake_secret
                             .request_handshake_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->handshake_secret
                .request_handshake_sequence_number;
        } else {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->handshake_secret
                             .response_handshake_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->handshake_secret
                             .response_handshake_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->handshake_secret
                .response_handshake_sequence_number;
        }
        break;
    case LIBSPDM_SESSION_STATE_ESTABLISHED:
        if (is_requester) {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->application_secret
                             .request_data_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->application_secret
                             .request_data_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->application_secret
                .request_data_sequence_number;
        } else {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->application_secret
                             .response_data_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->application_secret
                             .response_data_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->application_secret
                .response_data_sequence_number;
        }
        break;
    default:
        LIBSPDM_ASSERT(false);
        return RETURN_UNSUPPORTED;
        break;
    }

    if (sequence_number == (uint64_t)-1) {
        return RETURN_OUT_OF_RESOURCES;
    }

    *(uint64_t *)salt = *(uint64_t *)salt ^ sequence_number;

    sequence_num_in_header = 0;
    sequence_num_in_header_size =
        spdm_secured_message_callbacks->get_sequence_number(
            sequence_number, (uint8_t *)&sequence_num_in_header);
    LIBSPDM_ASSERT(sequence_num_in_header_size <= sizeof(sequence_num_in_header));

    sequence_number++;
    switch (session_state) {
    case LIBSPDM_SESSION_STATE_HANDSHAKING:
        if (is_requester) {
            secured_message_context->handshake_secret
            .request_handshake_sequence_number =
                sequence_number;
        } else {
            secured_message_context->handshake_secret
            .response_handshake_sequence_number =
                sequence_number;
        }
        break;
    case LIBSPDM_SESSION_STATE_ESTABLISHED:
        if (is_requester) {
            secured_message_context->application_secret
            .request_data_sequence_number = sequence_number;
        } else {
            secured_message_context->application_secret
            .response_data_sequence_number =
                sequence_number;
        }
        break;
    default:
        LIBSPDM_ASSERT(false);
        return RETURN_UNSUPPORTED;
    }

    record_header_size = sizeof(spdm_secured_message_a_data_header1_t) +
                         sequence_num_in_header_size +
                         sizeof(spdm_secured_message_a_data_header2_t);

    switch (session_type) {
    case LIBSPDM_SESSION_TYPE_ENC_MAC:
        max_rand_count = spdm_secured_message_callbacks
                         ->get_max_random_number_count();
        if (max_rand_count != 0) {
            rand_count = 0;
            result = libspdm_get_random_number(sizeof(rand_count),
                                               (uint8_t *)&rand_count);
            if (!result) {
                return RETURN_DEVICE_ERROR;
            }
            rand_count = (uint8_t)((rand_count % max_rand_count) + 1);
        } else {
            rand_count = 0;
        }

        plain_text_size = sizeof(spdm_secured_message_cipher_header_t) +
                          app_message_size + rand_count;
        cipher_text_size = plain_text_size;
        aead_pad_size = cipher_text_size - plain_text_size;
        total_secured_message_size =
            record_header_size + cipher_text_size + aead_tag_size;

        LIBSPDM_ASSERT(*secured_message_size >= total_secured_message_size);
        if (*secured_message_size < total_secured_message_size) {
            *secured_message_size = total_secured_message_size;
            return RETURN_BUFFER_TOO_SMALL;
        }
        *secured_message_size = total_secured_message_size;
        record_header1 = (void *)secured_message;
        record_header2 =
            (void *)((uint8_t *)record_header1 +
                     sizeof(spdm_secured_message_a_data_header1_t) +
                     sequence_num_in_header_size);
        record_header1->session_id = session_id;
        libspdm_copy_mem(record_header1 + 1,
                         *secured_message_size
                         - ((uint8_t*)(record_header1 + 1) - (uint8_t*)secured_message),
                         &sequence_num_in_header,
                         sequence_num_in_header_size);
        record_header2->length =
            (uint16_t)(cipher_text_size + aead_tag_size);
        enc_msg_header = (void *)(record_header2 + 1);
        enc_msg_header->application_data_length =
            (uint16_t)app_message_size;
        libspdm_copy_mem(enc_msg_header + 1,
                         *secured_message_size
                         - ((uint8_t*)(enc_msg_header + 1) - (uint8_t*)secured_message),
                         app_message, app_message_size);
        result = libspdm_get_random_number(rand_count,
                                           (uint8_t *)enc_msg_header +
                                           sizeof(spdm_secured_message_cipher_header_t) +
                                           app_message_size);
        if (!result) {
            return RETURN_DEVICE_ERROR;
        }
        libspdm_zero_mem((uint8_t *)enc_msg_header + plain_text_size,
                         aead_pad_size);

        a_data = (uint8_t *)record_header1;
        enc_msg = (uint8_t *)enc_msg_header;
        dec_msg = (uint8_t *)enc_msg_header;
        tag = (uint8_t *)record_header1 + record_header_size +
              cipher_text_size;

        result = libspdm_aead_encryption(
            secured_message_context->secured_message_version,
            secured_message_context->aead_cipher_suite, key,
            aead_key_size, salt, aead_iv_size, (uint8_t *)a_data,
            record_header_size, dec_msg, cipher_text_size, tag,
            aead_tag_size, enc_msg, &cipher_text_size);
        break;

    case LIBSPDM_SESSION_TYPE_MAC_ONLY:
        total_secured_message_size =
            record_header_size + app_message_size + aead_tag_size;

        LIBSPDM_ASSERT(*secured_message_size >= total_secured_message_size);
        if (*secured_message_size < total_secured_message_size) {
            *secured_message_size = total_secured_message_size;
            return RETURN_BUFFER_TOO_SMALL;
        }
        *secured_message_size = total_secured_message_size;
        record_header1 = (void *)secured_message;
        record_header2 =
            (void *)((uint8_t *)record_header1 +
                     sizeof(spdm_secured_message_a_data_header1_t) +
                     sequence_num_in_header_size);
        record_header1->session_id = session_id;
        libspdm_copy_mem(record_header1 + 1,
                         *secured_message_size
                         - ((uint8_t*)(record_header1 + 1) - (uint8_t*)secured_message),
                         &sequence_num_in_header,
                         sequence_num_in_header_size);
        record_header2->length =
            (uint16_t)(app_message_size + aead_tag_size);
        libspdm_copy_mem(record_header2 + 1,
                         *secured_message_size
                         - ((uint8_t*)(record_header2 + 1) - (uint8_t*)secured_message),
                         app_message, app_message_size);
        a_data = (uint8_t *)record_header1;
        tag = (uint8_t *)record_header1 + record_header_size +
              app_message_size;

        result = libspdm_aead_encryption(
            secured_message_context->secured_message_version,
            secured_message_context->aead_cipher_suite, key,
            aead_key_size, salt, aead_iv_size, (uint8_t *)a_data,
            record_header_size + app_message_size, NULL, 0, tag,
            aead_tag_size, NULL, NULL);
        break;

    default:
        LIBSPDM_ASSERT(false);
        return RETURN_UNSUPPORTED;
    }
    if (!result) {
        return RETURN_OUT_OF_RESOURCES;
    }
    return RETURN_SUCCESS;
}

/**
 * Decode an application message from a secured message.
 *
 * @param  spdm_secured_message_context    A pointer to the SPDM secured message context.
 * @param  session_id                    The session ID of the SPDM session.
 * @param  is_requester                  Indicates if it is a requester message.
 * @param  secured_message_size           size in bytes of the secured message data buffer.
 * @param  secured_message               A pointer to a source buffer to store the secured message.
 * @param  app_message_size               size in bytes of the application message data buffer.
 * @param  app_message                   A pointer to a destination buffer to store the application message.
 * @param  spdm_secured_message_callbacks  A pointer to a secured message callback functions structure.
 *
 * @retval RETURN_SUCCESS               The application message is decoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 * @retval RETURN_UNSUPPORTED           The secured_message is unsupported.
 **/
return_status libspdm_decode_secured_message(
    void *spdm_secured_message_context, uint32_t session_id,
    bool is_requester, uintn secured_message_size,
    const void *secured_message, uintn *app_message_size,
    void *app_message,
    const libspdm_secured_message_callbacks_t *spdm_secured_message_callbacks)
{
    libspdm_secured_message_context_t *secured_message_context;
    uintn plain_text_size;
    uintn cipher_text_size;
    uintn aead_tag_size;
    uintn aead_key_size;
    uintn aead_iv_size;
    uint8_t *a_data;
    uint8_t *enc_msg;
    uint8_t *dec_msg;
    uint8_t *tag;
    spdm_secured_message_a_data_header1_t *record_header1;
    spdm_secured_message_a_data_header2_t *record_header2;
    uintn record_header_size;
    spdm_secured_message_cipher_header_t *enc_msg_header;
    bool result;
    uint8_t key[LIBSPDM_MAX_AEAD_KEY_SIZE];
    uint8_t salt[LIBSPDM_MAX_AEAD_IV_SIZE];
    uint64_t sequence_number;
    uint64_t sequence_num_in_header;
    uint8_t sequence_num_in_header_size;
    libspdm_session_type_t session_type;
    libspdm_session_state_t session_state;
    libspdm_error_struct_t spdm_error;
    uint8_t dec_message[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    return_status status;

    spdm_error.error_code = 0;
    spdm_error.session_id = 0;
    libspdm_secured_message_set_last_spdm_error_struct(
        spdm_secured_message_context, &spdm_error);

    spdm_error.error_code = SPDM_ERROR_CODE_DECRYPT_ERROR;
    spdm_error.session_id = session_id;

    secured_message_context = spdm_secured_message_context;

    session_type = secured_message_context->session_type;
    LIBSPDM_ASSERT((session_type == LIBSPDM_SESSION_TYPE_MAC_ONLY) ||
                   (session_type == LIBSPDM_SESSION_TYPE_ENC_MAC));
    session_state = secured_message_context->session_state;
    LIBSPDM_ASSERT((session_state == LIBSPDM_SESSION_STATE_HANDSHAKING) ||
                   (session_state == LIBSPDM_SESSION_STATE_ESTABLISHED));

    aead_tag_size = secured_message_context->aead_tag_size;
    aead_key_size = secured_message_context->aead_key_size;
    aead_iv_size = secured_message_context->aead_iv_size;

    switch (session_state) {
    case LIBSPDM_SESSION_STATE_HANDSHAKING:
        if (is_requester) {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->handshake_secret
                             .request_handshake_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->handshake_secret
                             .request_handshake_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->handshake_secret
                .request_handshake_sequence_number;
        } else {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->handshake_secret
                             .response_handshake_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->handshake_secret
                             .response_handshake_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->handshake_secret
                .response_handshake_sequence_number;
        }
        break;
    case LIBSPDM_SESSION_STATE_ESTABLISHED:
        if (is_requester) {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->application_secret
                             .request_data_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->application_secret
                             .request_data_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->application_secret
                .request_data_sequence_number;
        } else {
            libspdm_copy_mem(key, sizeof(key),
                             secured_message_context->application_secret
                             .response_data_encryption_key,
                             secured_message_context->aead_key_size);
            libspdm_copy_mem(salt, sizeof(salt),
                             secured_message_context->application_secret
                             .response_data_salt,
                             secured_message_context->aead_iv_size);
            sequence_number =
                secured_message_context->application_secret
                .response_data_sequence_number;
        }
        break;
    default:
        LIBSPDM_ASSERT(false);
        return RETURN_UNSUPPORTED;
    }

    if (sequence_number == (uint64_t)-1) {
        libspdm_secured_message_set_last_spdm_error_struct(
            spdm_secured_message_context, &spdm_error);
        return RETURN_SECURITY_VIOLATION;
    }

    *(uint64_t *)salt = *(uint64_t *)salt ^ sequence_number;

    sequence_num_in_header = 0;
    sequence_num_in_header_size =
        spdm_secured_message_callbacks->get_sequence_number(
            sequence_number, (uint8_t *)&sequence_num_in_header);
    LIBSPDM_ASSERT(sequence_num_in_header_size <= sizeof(sequence_num_in_header));

    sequence_number++;
    switch (session_state) {
    case LIBSPDM_SESSION_STATE_HANDSHAKING:
        if (is_requester) {
            secured_message_context->handshake_secret
            .request_handshake_sequence_number =
                sequence_number;
        } else {
            secured_message_context->handshake_secret
            .response_handshake_sequence_number =
                sequence_number;
        }
        break;
    case LIBSPDM_SESSION_STATE_ESTABLISHED:
        if (is_requester) {
            secured_message_context->application_secret
            .request_data_sequence_number = sequence_number;
        } else {
            secured_message_context->application_secret
            .response_data_sequence_number =
                sequence_number;
        }
        break;
    default:
        LIBSPDM_ASSERT(false);
        return RETURN_UNSUPPORTED;
    }

    record_header_size = sizeof(spdm_secured_message_a_data_header1_t) +
                         sequence_num_in_header_size +
                         sizeof(spdm_secured_message_a_data_header2_t);

    switch (session_type) {
    case LIBSPDM_SESSION_TYPE_ENC_MAC:
        if (secured_message_size < record_header_size + aead_tag_size) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        record_header1 = (void *)secured_message;
        record_header2 =
            (void *)((uint8_t *)record_header1 +
                     sizeof(spdm_secured_message_a_data_header1_t) +
                     sequence_num_in_header_size);
        if (record_header1->session_id != session_id) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        if (libspdm_const_compare_mem(record_header1 + 1, &sequence_num_in_header,
                                      sequence_num_in_header_size) != 0) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        if (record_header2->length >
            secured_message_size - record_header_size) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        if (record_header2->length < aead_tag_size) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        cipher_text_size = (record_header2->length - aead_tag_size);
        if (cipher_text_size > sizeof(dec_message)) {
            return RETURN_OUT_OF_RESOURCES;
        }
        libspdm_zero_mem(dec_message, sizeof(dec_message));
        enc_msg_header = (void *)(record_header2 + 1);
        a_data = (uint8_t *)record_header1;
        enc_msg = (uint8_t *)enc_msg_header;
        dec_msg = (uint8_t *)dec_message;
        enc_msg_header = (void *)dec_msg;
        tag = (uint8_t *)record_header1 + record_header_size +
              cipher_text_size;
        result = libspdm_aead_decryption(
            secured_message_context->secured_message_version,
            secured_message_context->aead_cipher_suite, key,
            aead_key_size, salt, aead_iv_size, (uint8_t *)a_data,
            record_header_size, enc_msg, cipher_text_size, tag,
            aead_tag_size, dec_msg, &cipher_text_size);
        if (!result) {

            /* Try to use backup key to decrypt, because peer may use old key to encrypt error message.
             * Recursive call only once, because the xxx_backup_valid will be cleard in libspdm_activate_update_session_data_key().*/

            if ((is_requester && secured_message_context->requester_backup_valid) ||
                ((!is_requester) && secured_message_context->responder_backup_valid)) {
                status = libspdm_activate_update_session_data_key(
                    secured_message_context,
                    is_requester ? LIBSPDM_KEY_UPDATE_ACTION_REQUESTER : LIBSPDM_KEY_UPDATE_ACTION_RESPONDER,
                    false);
                if (RETURN_ERROR(status)) {
                    return status;
                }
                status = libspdm_decode_secured_message(
                    spdm_secured_message_context, session_id,
                    is_requester, secured_message_size,
                    secured_message, app_message_size,
                    app_message, spdm_secured_message_callbacks);
                if (RETURN_ERROR(status)) {
                    return status;
                }

                /* Handle special case:
                 * If the responder returns SPDM_RESPOND_IF_READY error, the requester need activate backup key to parse the error.
                 * Then later the responder will return SUCCESS, the requester need activate new key.
                 * So we need restore the environment by libspdm_create_update_session_data_key() again.*/

                return libspdm_create_update_session_data_key (secured_message_context,
                                                               is_requester ? LIBSPDM_KEY_UPDATE_ACTION_REQUESTER : LIBSPDM_KEY_UPDATE_ACTION_RESPONDER);
            }

            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        plain_text_size = enc_msg_header->application_data_length;
        if (plain_text_size > cipher_text_size) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }

        LIBSPDM_ASSERT(*app_message_size >= plain_text_size);
        if (*app_message_size < plain_text_size) {
            *app_message_size = plain_text_size;
            return RETURN_BUFFER_TOO_SMALL;
        }
        libspdm_copy_mem(app_message, *app_message_size, enc_msg_header + 1, plain_text_size);
        *app_message_size = plain_text_size;
        break;

    case LIBSPDM_SESSION_TYPE_MAC_ONLY:
        if (secured_message_size < record_header_size + aead_tag_size) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        record_header1 = (void *)secured_message;
        record_header2 =
            (void *)((uint8_t *)record_header1 +
                     sizeof(spdm_secured_message_a_data_header1_t) +
                     sequence_num_in_header_size);
        if (record_header1->session_id != session_id) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        if (libspdm_const_compare_mem(record_header1 + 1, &sequence_num_in_header,
                                      sequence_num_in_header_size) != 0) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        if (record_header2->length >
            secured_message_size - record_header_size) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        if (record_header2->length < aead_tag_size) {
            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }
        a_data = (uint8_t *)record_header1;
        tag = (uint8_t *)record_header1 + record_header_size +
              record_header2->length - aead_tag_size;
        result = libspdm_aead_decryption(
            secured_message_context->secured_message_version,
            secured_message_context->aead_cipher_suite, key,
            aead_key_size, salt, aead_iv_size, (uint8_t *)a_data,
            record_header_size + record_header2->length -
            aead_tag_size,
            NULL, 0, tag, aead_tag_size, NULL, NULL);
        if (!result) {

            /* try to use backup key to decrypt, because peer may use old key to encrypt error message.
             * recursive call only once, because the xxx_backup_valid will be cleard in libspdm_activate_update_session_data_key().*/

            if ((is_requester && secured_message_context->requester_backup_valid) ||
                ((!is_requester) && secured_message_context->responder_backup_valid)) {
                status = libspdm_activate_update_session_data_key(
                    secured_message_context,
                    is_requester ? LIBSPDM_KEY_UPDATE_ACTION_REQUESTER : LIBSPDM_KEY_UPDATE_ACTION_RESPONDER,
                    false);
                if (RETURN_ERROR(status)) {
                    return status;
                }
                status = libspdm_decode_secured_message(
                    spdm_secured_message_context, session_id,
                    is_requester, secured_message_size,
                    secured_message, app_message_size,
                    app_message, spdm_secured_message_callbacks);
                if (RETURN_ERROR(status)) {
                    return status;
                }

                /* Handle special case:
                 * If the responder returns SPDM_RESPOND_IF_READY error, the requester need activate backup key to parse the error.
                 * Then later the responder will return SUCCESS, the requester need activate new key.
                 * So we need restore the environment by libspdm_create_update_session_data_key() again.*/

                return libspdm_create_update_session_data_key (secured_message_context,
                                                               is_requester ? LIBSPDM_KEY_UPDATE_ACTION_REQUESTER : LIBSPDM_KEY_UPDATE_ACTION_RESPONDER);
            }

            libspdm_secured_message_set_last_spdm_error_struct(
                spdm_secured_message_context, &spdm_error);
            return RETURN_SECURITY_VIOLATION;
        }

        plain_text_size = record_header2->length - aead_tag_size;
        LIBSPDM_ASSERT(*app_message_size >= plain_text_size);
        if (*app_message_size < plain_text_size) {
            *app_message_size = plain_text_size;
            return RETURN_BUFFER_TOO_SMALL;
        }
        libspdm_copy_mem(app_message, *app_message_size, record_header2 + 1, plain_text_size);
        *app_message_size = plain_text_size;
        break;

    default:
        LIBSPDM_ASSERT(false);
        return RETURN_UNSUPPORTED;
    }

    return RETURN_SUCCESS;
}
