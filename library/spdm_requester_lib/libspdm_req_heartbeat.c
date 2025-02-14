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
} libspdm_heartbeat_response_mine_t;

#pragma pack()

/**
 * This function sends HEARTBEAT
 * to an SPDM Session.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_id                    The session ID of the session.
 *
 * @retval RETURN_SUCCESS               The heartbeat is sent and received.
 * @retval RETURN_DEVICE_ERROR          A device error occurs when communicates with the device.
 * @retval RETURN_SECURITY_VIOLATION    Any verification fails.
 **/
return_status libspdm_try_heartbeat(void *context, uint32_t session_id)
{
    return_status status;
    spdm_heartbeat_request_t spdm_request;
    libspdm_heartbeat_response_mine_t spdm_response;
    uintn spdm_response_size;
    libspdm_context_t *spdm_context;
    libspdm_session_info_t *session_info;
    libspdm_session_state_t session_state;

    spdm_context = context;
    if (!libspdm_is_capabilities_flag_supported(
            spdm_context, true,
            SPDM_GET_CAPABILITIES_REQUEST_FLAGS_HBEAT_CAP,
            SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_HBEAT_CAP)) {
        return RETURN_UNSUPPORTED;
    }

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

    spdm_request.header.spdm_version = libspdm_get_connection_version (spdm_context);
    spdm_request.header.request_response_code = SPDM_HEARTBEAT;
    spdm_request.header.param1 = 0;
    spdm_request.header.param2 = 0;
    status = libspdm_send_spdm_request(spdm_context, &session_id,
                                       sizeof(spdm_request), &spdm_request);
    if (RETURN_ERROR(status)) {
        return status;
    }

    libspdm_reset_message_buffer_via_request_code(spdm_context, session_info,
                                                  SPDM_HEARTBEAT);

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
            &spdm_response, SPDM_HEARTBEAT, SPDM_HEARTBEAT_ACK,
            sizeof(libspdm_heartbeat_response_mine_t));
        if (RETURN_ERROR(status)) {
            return status;
        }
    } else if (spdm_response.header.request_response_code !=
               SPDM_HEARTBEAT_ACK) {
        return RETURN_DEVICE_ERROR;
    }
    if (spdm_response_size != sizeof(spdm_heartbeat_response_t)) {
        return RETURN_DEVICE_ERROR;
    }

    return RETURN_SUCCESS;
}

return_status libspdm_heartbeat(void *context, uint32_t session_id)
{
    uintn retry;
    return_status status;
    libspdm_context_t *spdm_context;

    spdm_context = context;
    spdm_context->crypto_request = true;
    retry = spdm_context->retry_times;
    do {
        status = libspdm_try_heartbeat(spdm_context, session_id);
        if (RETURN_NO_RESPONSE != status) {
            return status;
        }
    } while (retry-- != 0);

    return status;
}
