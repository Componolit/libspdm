/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "internal/libspdm_responder_lib.h"

/**
 * Get the SPDM encapsulated KEY_UPDATE request.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  encap_request_size             size in bytes of the encapsulated request data.
 *                                     On input, it means the size in bytes of encapsulated request data buffer.
 *                                     On output, it means the size in bytes of copied encapsulated request data buffer if RETURN_SUCCESS is returned,
 *                                     and means the size in bytes of desired encapsulated request data buffer if RETURN_BUFFER_TOO_SMALL is returned.
 * @param  encap_request                 A pointer to the encapsulated request data.
 *
 * @retval RETURN_SUCCESS               The encapsulated request is returned.
 * @retval RETURN_BUFFER_TOO_SMALL      The buffer is too small to hold the data.
 **/
return_status
libspdm_get_encap_request_key_update(libspdm_context_t *spdm_context,
                                     uintn *encap_request_size,
                                     void *encap_request)
{
    spdm_key_update_request_t *spdm_request;
    uint32_t session_id;
    libspdm_session_info_t *session_info;
    libspdm_session_state_t session_state;
    return_status status;

    spdm_context->encap_context.last_encap_request_size = 0;

    if (!libspdm_is_capabilities_flag_supported(
            spdm_context, false,
            SPDM_GET_CAPABILITIES_REQUEST_FLAGS_KEY_UPD_CAP,
            SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_KEY_UPD_CAP)) {
        return RETURN_UNSUPPORTED;
    }

    if (!spdm_context->last_spdm_request_session_id_valid) {
        return RETURN_UNSUPPORTED;
    }
    session_id = spdm_context->last_spdm_request_session_id;
    session_info =
        libspdm_get_session_info_via_session_id(spdm_context, session_id);
    if (session_info == NULL) {
        return RETURN_UNSUPPORTED;
    }
    session_state = libspdm_secured_message_get_session_state(
        session_info->secured_message_context);
    if (session_state != LIBSPDM_SESSION_STATE_ESTABLISHED) {
        return RETURN_UNSUPPORTED;
    }

    LIBSPDM_ASSERT(*encap_request_size >= sizeof(spdm_key_update_request_t));
    *encap_request_size = sizeof(spdm_key_update_request_t);

    spdm_request = encap_request;

    spdm_request->header.spdm_version = libspdm_get_connection_version (spdm_context);
    spdm_request->header.request_response_code = SPDM_KEY_UPDATE;

    libspdm_reset_message_buffer_via_request_code(spdm_context, session_info,
                                                  spdm_request->header.request_response_code);

    if (spdm_context->encap_context.last_encap_request_header
        .request_response_code != SPDM_KEY_UPDATE) {
        spdm_request->header.param1 =
            SPDM_KEY_UPDATE_OPERATIONS_TABLE_UPDATE_KEY;
        spdm_request->header.param2 = 0;
        if(!libspdm_get_random_number(sizeof(spdm_request->header.param2),
                                      &spdm_request->header.param2)) {
            return RETURN_DEVICE_ERROR;
        }
    } else {
        spdm_request->header.param1 =
            SPDM_KEY_UPDATE_OPERATIONS_TABLE_VERIFY_NEW_KEY;
        spdm_request->header.param2 = 1;
        if(!libspdm_get_random_number(sizeof(spdm_request->header.param2),
                                      &spdm_request->header.param2)) {
            return RETURN_DEVICE_ERROR;
        }

        /* Create new key*/
        LIBSPDM_DEBUG((LIBSPDM_DEBUG_INFO,
                       "libspdm_create_update_session_data_key[%x] Responder\n",
                       session_id));
        status = libspdm_create_update_session_data_key(
            session_info->secured_message_context,
            LIBSPDM_KEY_UPDATE_ACTION_RESPONDER);
        if (RETURN_ERROR(status)) {
            return status;
        }
        LIBSPDM_DEBUG((LIBSPDM_DEBUG_INFO,
                       "libspdm_activate_update_session_data_key[%x] Responder new\n",
                       session_id));
        status = libspdm_activate_update_session_data_key(
            session_info->secured_message_context,
            LIBSPDM_KEY_UPDATE_ACTION_RESPONDER, true);
        if (RETURN_ERROR(status)) {
            return status;
        }
    }

    libspdm_copy_mem(&spdm_context->encap_context.last_encap_request_header,
                     sizeof(spdm_context->encap_context.last_encap_request_header),
                     &spdm_request->header, sizeof(spdm_message_header_t));
    spdm_context->encap_context.last_encap_request_size =
        *encap_request_size;

    return RETURN_SUCCESS;
}

/**
 * Process the SPDM encapsulated KEY_UPDATE response.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  encap_response_size            size in bytes of the encapsulated response data.
 * @param  encap_response                A pointer to the encapsulated response data.
 * @param  need_continue                     Indicate if encapsulated communication need continue.
 *
 * @retval RETURN_SUCCESS               The encapsulated response is processed.
 * @retval RETURN_BUFFER_TOO_SMALL      The buffer is too small to hold the data.
 * @retval RETURN_SECURITY_VIOLATION    Any verification fails.
 **/
return_status libspdm_process_encap_response_key_update(
    libspdm_context_t *spdm_context, uintn encap_response_size,
    const void *encap_response, bool *need_continue)
{
    spdm_key_update_request_t *spdm_request;
    const spdm_key_update_response_t *spdm_response;
    uintn spdm_response_size;
    uint32_t session_id;
    libspdm_session_info_t *session_info;
    libspdm_session_state_t session_state;

    if (!spdm_context->last_spdm_request_session_id_valid) {
        return RETURN_UNSUPPORTED;
    }
    session_id = spdm_context->last_spdm_request_session_id;
    session_info =
        libspdm_get_session_info_via_session_id(spdm_context, session_id);
    if (session_info == NULL) {
        return RETURN_UNSUPPORTED;
    }
    session_state = libspdm_secured_message_get_session_state(
        session_info->secured_message_context);
    if (session_state != LIBSPDM_SESSION_STATE_ESTABLISHED) {
        return RETURN_UNSUPPORTED;
    }

    spdm_request =
        (void *)&spdm_context->encap_context.last_encap_request_header;

    spdm_response = encap_response;
    spdm_response_size = encap_response_size;

    if (spdm_response->header.spdm_version != libspdm_get_connection_version (spdm_context)) {
        return RETURN_DEVICE_ERROR;
    }

    if (spdm_response->header.request_response_code == SPDM_ERROR) {
        if (spdm_response->header.param1 == SPDM_ERROR_CODE_DECRYPT_ERROR) {
            libspdm_free_session_id(spdm_context, session_id);
            return RETURN_SECURITY_VIOLATION;
        }
    }

    if ((spdm_response_size != sizeof(spdm_key_update_response_t)) ||
        (spdm_response->header.request_response_code !=
         SPDM_KEY_UPDATE_ACK) ||
        (spdm_response->header.param1 != spdm_request->header.param1) ||
        (spdm_response->header.param2 != spdm_request->header.param2)) {
        if (spdm_request->header.param1 !=
            SPDM_KEY_UPDATE_OPERATIONS_TABLE_VERIFY_NEW_KEY) {
            LIBSPDM_DEBUG((LIBSPDM_DEBUG_INFO, "libspdm_key_update[%x] failed\n",
                           session_id));
        } else {
            LIBSPDM_DEBUG((LIBSPDM_DEBUG_INFO, "SpdmVerifyKey[%x] failed\n",
                           session_id));
        }
        return RETURN_DEVICE_ERROR;
    }

    if (spdm_request->header.param1 !=
        SPDM_KEY_UPDATE_OPERATIONS_TABLE_VERIFY_NEW_KEY) {
        LIBSPDM_DEBUG((LIBSPDM_DEBUG_INFO, "libspdm_key_update[%x] success\n",
                       session_id));
        *need_continue = true;
    } else {
        LIBSPDM_DEBUG((LIBSPDM_DEBUG_INFO, "SpdmVerifyKey[%x] Success\n", session_id));
        *need_continue = false;
    }

    return RETURN_SUCCESS;
}
