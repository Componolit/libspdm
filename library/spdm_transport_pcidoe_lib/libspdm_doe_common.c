/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "library/spdm_transport_pcidoe_lib.h"
#include "library/spdm_secured_message_lib.h"

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
return_status libspdm_pci_doe_encode_message(const uint32_t *session_id,
                                             uintn message_size, const void *message,
                                             uintn *transport_message_size,
                                             void *transport_message);

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
return_status libspdm_pci_doe_decode_message(uint32_t **session_id,
                                             uintn transport_message_size,
                                             const void *transport_message,
                                             uintn *message_size,
                                             void *message);

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
typedef return_status (*libspdm_pci_doe_encode_message_func)(
    const uint32_t *session_id, uintn message_size, const void *message,
    uintn *transport_message_size, void *transport_message);

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
typedef return_status (*libspdm_pci_doe_decode_message_func)(
    uint32_t **session_id, uintn transport_message_size,
    const void *transport_message, uintn *message_size,
    void *message);

/**
 * Encode an SPDM or APP message to a transport layer message.
 *
 * For normal SPDM message, it adds the transport layer wrapper.
 * For secured SPDM message, it encrypts a secured message then adds the transport layer wrapper.
 * For secured APP message, it encrypts a secured message then adds the transport layer wrapper.
 *
 * The APP message is encoded to a secured message directly in SPDM session.
 * The APP message format is defined by the transport layer.
 * Take MCTP as example: APP message == MCTP header (MCTP_MESSAGE_TYPE_SPDM) + SPDM message
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_id                    Indicates if it is a secured message protected via SPDM session.
 *                                     If session_id is NULL, it is a normal message.
 *                                     If session_id is NOT NULL, it is a secured message.
 * @param  is_app_message                 Indicates if it is an APP message or SPDM message.
 * @param  is_requester                  Indicates if it is a requester message.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a source buffer to store the message.
 * @param  transport_message_size         size in bytes of the transport message data buffer.
 * @param  transport_message             A pointer to a destination buffer to store the transport message.
 *
 * @retval RETURN_SUCCESS               The message is encoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 **/
return_status libspdm_transport_pci_doe_encode_message(
    void *spdm_context, const uint32_t *session_id, bool is_app_message,
    bool is_requester, uintn message_size, const void *message,
    uintn *transport_message_size, void *transport_message)
{
    return_status status;
    libspdm_pci_doe_encode_message_func transport_encode_message;
    uint8_t secured_message[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    uintn secured_message_size;
    libspdm_secured_message_callbacks_t spdm_secured_message_callbacks;
    void *secured_message_context;

    spdm_secured_message_callbacks.version =
        SPDM_SECURED_MESSAGE_CALLBACKS_VERSION;
    spdm_secured_message_callbacks.get_sequence_number =
        libspdm_pci_doe_get_sequence_number;
    spdm_secured_message_callbacks.get_max_random_number_count =
        libspdm_pci_doe_get_max_random_number_count;

    if (is_app_message) {
        return RETURN_UNSUPPORTED;
    }

    transport_encode_message = libspdm_pci_doe_encode_message;
    if (session_id != NULL) {
        secured_message_context =
            libspdm_get_secured_message_context_via_session_id(
                spdm_context, *session_id);
        if (secured_message_context == NULL) {
            return RETURN_UNSUPPORTED;
        }

        /* message to secured message*/
        secured_message_size = sizeof(secured_message);
        status = libspdm_encode_secured_message(
            secured_message_context, *session_id, is_requester,
            message_size, message, &secured_message_size,
            secured_message, &spdm_secured_message_callbacks);
        if (RETURN_ERROR(status)) {
            LIBSPDM_DEBUG((LIBSPDM_DEBUG_ERROR,
                           "libspdm_encode_secured_message - %p\n", status));
            return status;
        }

        /* secured message to secured PCI DOE message*/
        status = transport_encode_message(
            session_id, secured_message_size, secured_message,
            transport_message_size, transport_message);
        if (RETURN_ERROR(status)) {
            LIBSPDM_DEBUG((LIBSPDM_DEBUG_ERROR, "transport_encode_message - %p\n",
                           status));
            return RETURN_UNSUPPORTED;
        }
    } else {
        /* SPDM message to normal PCI DOE message*/
        status = transport_encode_message(NULL, message_size, message,
                                          transport_message_size,
                                          transport_message);
        if (RETURN_ERROR(status)) {
            LIBSPDM_DEBUG((LIBSPDM_DEBUG_ERROR, "transport_encode_message - %p\n",
                           status));
            return RETURN_UNSUPPORTED;
        }
    }

    return RETURN_SUCCESS;
}

/**
 * Decode an SPDM or APP message from a transport layer message.
 *
 * For normal SPDM message, it removes the transport layer wrapper,
 * For secured SPDM message, it removes the transport layer wrapper, then decrypts and verifies a secured message.
 * For secured APP message, it removes the transport layer wrapper, then decrypts and verifies a secured message.
 *
 * The APP message is decoded from a secured message directly in SPDM session.
 * The APP message format is defined by the transport layer.
 * Take MCTP as example: APP message == MCTP header (MCTP_MESSAGE_TYPE_SPDM) + SPDM message
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_id                    Indicates if it is a secured message protected via SPDM session.
 *                                     If *session_id is NULL, it is a normal message.
 *                                     If *session_id is NOT NULL, it is a secured message.
 * @param  is_app_message                 Indicates if it is an APP message or SPDM message.
 * @param  is_requester                  Indicates if it is a requester message.
 * @param  transport_message_size         size in bytes of the transport message data buffer.
 * @param  transport_message             A pointer to a source buffer to store the transport message.
 * @param  message_size                  size in bytes of the message data buffer.
 * @param  message                      A pointer to a destination buffer to store the message.
 *
 * @retval RETURN_SUCCESS               The message is decoded successfully.
 * @retval RETURN_INVALID_PARAMETER     The message is NULL or the message_size is zero.
 * @retval RETURN_UNSUPPORTED           The transport_message is unsupported.
 **/
return_status libspdm_transport_pci_doe_decode_message(
    void *spdm_context, uint32_t **session_id,
    bool *is_app_message, bool is_requester,
    uintn transport_message_size, const void *transport_message,
    uintn *message_size, void *message)
{
    return_status status;
    libspdm_pci_doe_decode_message_func transport_decode_message;
    uint32_t *secured_message_session_id;
    uint8_t secured_message[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    uintn secured_message_size;
    libspdm_secured_message_callbacks_t spdm_secured_message_callbacks;
    void *secured_message_context;
    libspdm_error_struct_t spdm_error;

    spdm_error.error_code = 0;
    spdm_error.session_id = 0;
    libspdm_set_last_spdm_error_struct(spdm_context, &spdm_error);

    spdm_secured_message_callbacks.version =
        SPDM_SECURED_MESSAGE_CALLBACKS_VERSION;
    spdm_secured_message_callbacks.get_sequence_number =
        libspdm_pci_doe_get_sequence_number;
    spdm_secured_message_callbacks.get_max_random_number_count =
        libspdm_pci_doe_get_max_random_number_count;

    if ((session_id == NULL) || (is_app_message == NULL)) {
        return RETURN_UNSUPPORTED;
    }
    *is_app_message = false;

    transport_decode_message = libspdm_pci_doe_decode_message;

    secured_message_session_id = NULL;
    /* Detect received message*/
    secured_message_size = sizeof(secured_message);
    status = transport_decode_message(
        &secured_message_session_id, transport_message_size,
        transport_message, &secured_message_size, secured_message);
    if (RETURN_ERROR(status)) {
        LIBSPDM_DEBUG((LIBSPDM_DEBUG_ERROR, "transport_decode_message - %p\n", status));
        return RETURN_UNSUPPORTED;
    }

    if (secured_message_session_id != NULL) {
        *session_id = secured_message_session_id;

        secured_message_context =
            libspdm_get_secured_message_context_via_session_id(
                spdm_context, *secured_message_session_id);
        if (secured_message_context == NULL) {
            spdm_error.error_code = SPDM_ERROR_CODE_INVALID_SESSION;
            spdm_error.session_id = *secured_message_session_id;
            libspdm_set_last_spdm_error_struct(spdm_context,
                                               &spdm_error);
            return RETURN_UNSUPPORTED;
        }

        /* Secured message to message*/
        status = libspdm_decode_secured_message(
            secured_message_context, *secured_message_session_id,
            is_requester, secured_message_size, secured_message,
            message_size, message,
            &spdm_secured_message_callbacks);
        if (RETURN_ERROR(status)) {
            LIBSPDM_DEBUG((LIBSPDM_DEBUG_ERROR,
                           "libspdm_decode_secured_message - %p\n", status));
            libspdm_secured_message_get_last_spdm_error_struct(
                secured_message_context, &spdm_error);
            libspdm_set_last_spdm_error_struct(spdm_context,
                                               &spdm_error);
            return RETURN_UNSUPPORTED;
        }
        return RETURN_SUCCESS;
    } else {
        /* get non-secured message*/
        status = transport_decode_message(&secured_message_session_id,
                                          transport_message_size,
                                          transport_message,
                                          message_size, message);
        if (RETURN_ERROR(status)) {
            LIBSPDM_DEBUG((LIBSPDM_DEBUG_ERROR, "transport_decode_message - %p\n",
                           status));
            return RETURN_UNSUPPORTED;
        }
        LIBSPDM_ASSERT(secured_message_session_id == NULL);
        *session_id = NULL;
        return RETURN_SUCCESS;
    }
}
