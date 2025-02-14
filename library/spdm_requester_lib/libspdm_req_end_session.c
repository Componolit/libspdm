/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "internal/libspdm_requester_lib.h"

#pragma pack(1)

typedef struct {
    spdm_message_header_t header;
    uint8_t dummy_data[sizeof(spdm_error_data_response_not_ready_t)];
} libspdm_end_session_response_mine_t;

#pragma pack()

/**
 * This function sends END_SESSION and receives END_SESSION_ACK for SPDM session end.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_id                    session_id to the END_SESSION request.
 * @param  end_session_attributes         end_session_attributes to the END_SESSION_ACK request.
 *
 * @retval RETURN_SUCCESS               The END_SESSION is sent and the END_SESSION_ACK is received.
 * @retval RETURN_DEVICE_ERROR          A device error occurs when communicates with the device.
 **/
return_status libspdm_try_send_receive_end_session(libspdm_context_t *spdm_context,
                                                   uint32_t session_id,
                                                   uint8_t end_session_attributes)
{
    return_status status;
    spdm_end_session_request_t spdm_request;
    uintn spdm_request_size;
    libspdm_end_session_response_mine_t spdm_response;
    uintn spdm_response_size;
    libspdm_session_info_t *session_info;
    libspdm_session_state_t session_state;

    if (spdm_context->connection_info.connection_state <
        LIBSPDM_CONNECTION_STATE_NEGOTIATED) {
        return RETURN_UNSUPPORTED;
    }
    session_info =
        libspdm_get_session_info_via_session_id(spdm_context, session_id);
    if (session_info == NULL) {
        LIBSPDM_ASSERT(false);
        return RETURN_UNSUPPORTED;
    }
    session_state = libspdm_secured_message_get_session_state(
        session_info->secured_message_context);
    if (session_state != LIBSPDM_SESSION_STATE_ESTABLISHED) {
        return RETURN_UNSUPPORTED;
    }

    spdm_context->error_state = LIBSPDM_STATUS_ERROR_DEVICE_NO_CAPABILITIES;

    if (!libspdm_is_capabilities_flag_supported(
            spdm_context, true, 0,
            SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CACHE_CAP)) {
        end_session_attributes = 0;
    }

    spdm_request.header.spdm_version = libspdm_get_connection_version (spdm_context);
    spdm_request.header.request_response_code = SPDM_END_SESSION;
    spdm_request.header.param1 = end_session_attributes;
    spdm_request.header.param2 = 0;

    spdm_request_size = sizeof(spdm_end_session_request_t);
    status = libspdm_send_spdm_request(spdm_context, &session_id,
                                       spdm_request_size, &spdm_request);
    if (RETURN_ERROR(status)) {
        return status;
    }

    libspdm_reset_message_buffer_via_request_code(spdm_context, session_info,
                                                  SPDM_END_SESSION);

    spdm_response_size = sizeof(spdm_response);
    libspdm_zero_mem(&spdm_response, sizeof(spdm_response));
    status = libspdm_receive_spdm_response(
        spdm_context, &session_id, &spdm_response_size, &spdm_response);
    if (RETURN_ERROR(status)) {
        return status;
    }
    if (spdm_response_size < sizeof(spdm_message_header_t)) {
        return RETURN_DEVICE_ERROR;
    }
    if (spdm_response.header.spdm_version != spdm_request.header.spdm_version) {
        return RETURN_DEVICE_ERROR;
    }
    if (spdm_response.header.request_response_code == SPDM_ERROR) {
        status = libspdm_handle_error_response_main(
            spdm_context, &session_id, &spdm_response_size,
            &spdm_response, SPDM_END_SESSION, SPDM_END_SESSION_ACK,
            sizeof(libspdm_end_session_response_mine_t));
        if (RETURN_ERROR(status)) {
            return status;
        }
    } else if (spdm_response.header.request_response_code !=
               SPDM_END_SESSION_ACK) {
        return RETURN_DEVICE_ERROR;
    }
    if (spdm_response_size != sizeof(spdm_end_session_response_t)) {
        return RETURN_DEVICE_ERROR;
    }

    session_info->end_session_attributes = end_session_attributes;

    libspdm_secured_message_set_session_state(
        session_info->secured_message_context,
        LIBSPDM_SESSION_STATE_NOT_STARTED);
    libspdm_free_session_id(spdm_context, session_id);

    spdm_context->error_state = LIBSPDM_STATUS_SUCCESS;

    return RETURN_SUCCESS;
}

return_status libspdm_send_receive_end_session(libspdm_context_t *spdm_context,
                                               uint32_t session_id,
                                               uint8_t end_session_attributes)
{
    uintn retry;
    return_status status;

    spdm_context->crypto_request = true;
    retry = spdm_context->retry_times;
    do {
        status = libspdm_try_send_receive_end_session(
            spdm_context, session_id, end_session_attributes);
        if (RETURN_NO_RESPONSE != status) {
            return status;
        }
    } while (retry-- != 0);

    return status;
}
