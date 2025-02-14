/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "internal/libspdm_common_lib.h"

/**
 * Returns if an SPDM data_type requires session info.
 *
 * @param data_type  SPDM data type.
 *
 * @retval true  session info is required.
 * @retval false session info is not required.
 **/
bool need_session_info_for_data(libspdm_data_type_t data_type)
{
    switch (data_type) {
    case LIBSPDM_DATA_SESSION_USE_PSK:
    case LIBSPDM_DATA_SESSION_MUT_AUTH_REQUESTED:
    case LIBSPDM_DATA_SESSION_END_SESSION_ATTRIBUTES:
    case LIBSPDM_DATA_SESSION_POLICY:
        return true;
    default:
        return false;
    }
}

/**
 * Set an SPDM context data.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  data_type                     Type of the SPDM context data.
 * @param  parameter                    Type specific parameter of the SPDM context data.
 * @param  data                         A pointer to the SPDM context data.
 * @param  data_size                     size in bytes of the SPDM context data.
 *
 * @retval RETURN_SUCCESS               The SPDM context data is set successfully.
 * @retval RETURN_INVALID_PARAMETER     The data is NULL or the data_type is zero.
 * @retval RETURN_UNSUPPORTED           The data_type is unsupported.
 * @retval RETURN_ACCESS_DENIED         The data_type cannot be set.
 * @retval RETURN_NOT_READY             data is not ready to set.
 **/
return_status libspdm_set_data(void *context, libspdm_data_type_t data_type,
                               const libspdm_data_parameter_t *parameter, void *data,
                               uintn data_size)
{
    libspdm_context_t *spdm_context;
    uint32_t session_id;
    libspdm_session_info_t *session_info;
    uint8_t slot_id;
    uint8_t mut_auth_requested;
    uint8_t root_cert_index;
#if !LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    bool status;
#endif

    if (!context || !data || data_type >= LIBSPDM_DATA_MAX) {
        return RETURN_INVALID_PARAMETER;
    }

    spdm_context = context;

    if (need_session_info_for_data(data_type)) {
        if (parameter->location != LIBSPDM_DATA_LOCATION_SESSION) {
            return RETURN_INVALID_PARAMETER;
        }
        session_id = *(uint32_t *)parameter->additional_data;
        session_info = libspdm_get_session_info_via_session_id(
            spdm_context, session_id);
        if (session_info == NULL) {
            return RETURN_INVALID_PARAMETER;
        }
    } else {
        session_info = NULL;
    }

    switch (data_type) {
    case LIBSPDM_DATA_SPDM_VERSION:
        LIBSPDM_ASSERT (data_size <= sizeof(spdm_version_number_t) * SPDM_MAX_VERSION_COUNT);
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            /* Only have one connected version */
            LIBSPDM_ASSERT (data_size == sizeof(spdm_version_number_t));
            libspdm_copy_mem(&(spdm_context->connection_info.version),
                             sizeof(spdm_context->connection_info.version),
                             data,
                             sizeof(spdm_version_number_t));
        } else {
            spdm_context->local_context.version.spdm_version_count =
                (uint8_t)(data_size /
                          sizeof(spdm_version_number_t));
            libspdm_copy_mem(spdm_context->local_context.version.spdm_version,
                             sizeof(spdm_context->local_context.version.spdm_version),
                             data,
                             spdm_context->local_context.version
                             .spdm_version_count *
                             sizeof(spdm_version_number_t));
        }
        break;
    case LIBSPDM_DATA_SECURED_MESSAGE_VERSION:
        LIBSPDM_ASSERT (data_size <= sizeof(spdm_version_number_t) * SPDM_MAX_VERSION_COUNT);
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            /* Only have one connected version */
            LIBSPDM_ASSERT (data_size == sizeof(spdm_version_number_t));
            libspdm_copy_mem(&(spdm_context->connection_info.secured_message_version),
                             sizeof(spdm_context->connection_info.secured_message_version),
                             data,
                             sizeof(spdm_version_number_t));
        } else {
            spdm_context->local_context.secured_message_version
            .spdm_version_count = (uint8_t)(
                data_size / sizeof(spdm_version_number_t));
            libspdm_copy_mem(spdm_context->local_context
                             .secured_message_version.spdm_version,
                             sizeof(spdm_context->local_context
                                    .secured_message_version.spdm_version),
                             data,
                             spdm_context->local_context
                             .secured_message_version
                             .spdm_version_count *
                             sizeof(spdm_version_number_t));
        }
        break;
    case LIBSPDM_DATA_CAPABILITY_FLAGS:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }

    #if !LIBSPDM_ENABLE_CAPABILITY_CERT_CAP
        LIBSPDM_ASSERT(((*(uint32_t *)data) & SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP) == 0);
    #endif /* !LIBSPDM_ENABLE_CAPABILITY_CERT_CAP*/

    #if !LIBSPDM_ENABLE_CAPABILITY_CHAL_CAP
        LIBSPDM_ASSERT(((*(uint32_t *)data) & SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CHAL_CAP) == 0);
    #endif /* !LIBSPDM_ENABLE_CAPABILITY_CHAL_CAP*/

    #if !LIBSPDM_ENABLE_CAPABILITY_MEAS_CAP
        LIBSPDM_ASSERT(((*(uint32_t *)data) & SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_MEAS_CAP) == 0);
    #endif /* !LIBSPDM_ENABLE_CAPABILITY_MEAS_CAP*/

    #if !LIBSPDM_ENABLE_CAPABILITY_KEY_EX_CAP
        LIBSPDM_ASSERT(((*(uint32_t *)data) & SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_KEY_EX_CAP) ==
                       0);
    #endif /* !LIBSPDM_ENABLE_CAPABILITY_KEY_EX_CAP*/

    #if !LIBSPDM_ENABLE_CAPABILITY_PSK_EX_CAP
        LIBSPDM_ASSERT(((*(uint32_t *)data) & SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_PSK_CAP) == 0);
    #endif /* !LIBSPDM_ENABLE_CAPABILITY_PSK_EX_CAP*/

        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.capability.flags =
                *(uint32_t *)data;
        } else {
            spdm_context->local_context.capability.flags =
                *(uint32_t *)data;
        }
        break;
    case LIBSPDM_DATA_CAPABILITY_CT_EXPONENT:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context.capability.ct_exponent =
            *(uint8_t *)data;
        break;
    case LIBSPDM_DATA_CAPABILITY_RTT_US:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context.capability.rtt = *(uint8_t *)data;
        break;
    case LIBSPDM_DATA_CAPABILITY_DATA_TRANSFER_SIZE:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        /* Only allow set smaller value*/
        LIBSPDM_ASSERT (*(uint32_t *)data <= LIBSPDM_MAX_MESSAGE_BUFFER_SIZE);
        spdm_context->local_context.capability.data_transfer_size =
            *(uint32_t *)data;
        break;
    case LIBSPDM_DATA_CAPABILITY_MAX_SPDM_MSG_SIZE:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        /* Only allow set smaller value. Need different value for CHUNK - TBD*/
        LIBSPDM_ASSERT (*(uint32_t *)data <= LIBSPDM_MAX_MESSAGE_BUFFER_SIZE);
        spdm_context->local_context.capability.max_spdm_msg_size =
            *(uint32_t *)data;
        break;
    case LIBSPDM_DATA_MEASUREMENT_SPEC:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm
            .measurement_spec = *(uint8_t *)data;
        } else {
            spdm_context->local_context.algorithm.measurement_spec =
                *(uint8_t *)data;
        }
        break;
    case LIBSPDM_DATA_MEASUREMENT_HASH_ALGO:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm
            .measurement_hash_algo = *(uint32_t *)data;
        } else {
            spdm_context->local_context.algorithm
            .measurement_hash_algo = *(uint32_t *)data;
        }
        break;
    case LIBSPDM_DATA_BASE_ASYM_ALGO:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm.base_asym_algo =
                *(uint32_t *)data;
        } else {
            spdm_context->local_context.algorithm.base_asym_algo =
                *(uint32_t *)data;
        }
        break;
    case LIBSPDM_DATA_BASE_HASH_ALGO:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm.base_hash_algo =
                *(uint32_t *)data;
        } else {
            spdm_context->local_context.algorithm.base_hash_algo =
                *(uint32_t *)data;
        }
        break;
    case LIBSPDM_DATA_DHE_NAME_GROUP:
        if (data_size != sizeof(uint16_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm.dhe_named_group =
                *(uint16_t *)data;
        } else {
            spdm_context->local_context.algorithm.dhe_named_group =
                *(uint16_t *)data;
        }
        break;
    case LIBSPDM_DATA_AEAD_CIPHER_SUITE:
        if (data_size != sizeof(uint16_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm
            .aead_cipher_suite = *(uint16_t *)data;
        } else {
            spdm_context->local_context.algorithm.aead_cipher_suite =
                *(uint16_t *)data;
        }
        break;
    case LIBSPDM_DATA_REQ_BASE_ASYM_ALG:
        if (data_size != sizeof(uint16_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm
            .req_base_asym_alg = *(uint16_t *)data;
        } else {
            spdm_context->local_context.algorithm.req_base_asym_alg =
                *(uint16_t *)data;
        }
        break;
    case LIBSPDM_DATA_KEY_SCHEDULE:
        if (data_size != sizeof(uint16_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm.key_schedule =
                *(uint16_t *)data;
        } else {
            spdm_context->local_context.algorithm.key_schedule =
                *(uint16_t *)data;
        }
        break;
    case LIBSPDM_DATA_OTHER_PARAMS_SUPPORT:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            spdm_context->connection_info.algorithm
            .other_params_support = *(uint8_t *)data;
        } else {
            spdm_context->local_context.algorithm.other_params_support =
                *(uint8_t *)data;
        }
        break;
    case LIBSPDM_DATA_CONNECTION_STATE:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->connection_info.connection_state =
            *(uint32_t *)data;
        break;
    case LIBSPDM_DATA_RESPONSE_STATE:
        if (data_size != sizeof(uint32_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->response_state = *(uint32_t *)data;
        break;
    case LIBSPDM_DATA_PEER_PUBLIC_ROOT_CERT:
        root_cert_index = 0;
        while (spdm_context->local_context.peer_root_cert_provision[root_cert_index] != NULL) {
            root_cert_index++;
            if (root_cert_index >= LIBSPDM_MAX_ROOT_CERT_SUPPORT) {
                return RETURN_OUT_OF_RESOURCES;
            }
        }
        spdm_context->local_context.peer_root_cert_provision_size[root_cert_index] =
            data_size;
        spdm_context->local_context.peer_root_cert_provision[root_cert_index] =
            data;
        break;
    case LIBSPDM_DATA_PEER_PUBLIC_CERT_CHAIN:
        spdm_context->local_context.peer_cert_chain_provision_size =
            data_size;
        spdm_context->local_context.peer_cert_chain_provision = data;
        break;
    case LIBSPDM_DATA_LOCAL_SLOT_COUNT:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        slot_id = *(uint8_t *)data;
        if (slot_id > SPDM_MAX_SLOT_COUNT) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context.slot_count = slot_id;
        break;
    case LIBSPDM_DATA_LOCAL_PUBLIC_CERT_CHAIN:
        slot_id = parameter->additional_data[0];
        if (slot_id >= spdm_context->local_context.slot_count) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context
        .local_cert_chain_provision_size[slot_id] = data_size;
        spdm_context->local_context.local_cert_chain_provision[slot_id] =
            data;
        break;
    case LIBSPDM_DATA_LOCAL_USED_CERT_CHAIN_BUFFER:
        if (data_size > LIBSPDM_MAX_CERT_CHAIN_SIZE) {
            return RETURN_OUT_OF_RESOURCES;
        }
        spdm_context->connection_info.local_used_cert_chain_buffer_size =
            data_size;
        spdm_context->connection_info.local_used_cert_chain_buffer =
            data;
        break;
    case LIBSPDM_DATA_PEER_USED_CERT_CHAIN_BUFFER:
        if (data_size > LIBSPDM_MAX_CERT_CHAIN_SIZE) {
            return RETURN_OUT_OF_RESOURCES;
        }
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
        spdm_context->connection_info.peer_used_cert_chain_buffer_size =
            data_size;
        libspdm_copy_mem(spdm_context->connection_info.peer_used_cert_chain_buffer,
                         sizeof(spdm_context->connection_info.peer_used_cert_chain_buffer),
                         data, data_size);
#else
        status = libspdm_hash_all(
            spdm_context->connection_info.algorithm.base_hash_algo,
            data, data_size,
            spdm_context->connection_info.peer_used_cert_chain_buffer_hash);
        if (!status) {
            return RETURN_UNSUPPORTED;
        }

        spdm_context->connection_info.peer_used_cert_chain_buffer_hash_size =
            libspdm_get_hash_size(spdm_context->connection_info.algorithm.base_hash_algo);

        status = false;
#if (LIBSPDM_RSA_SSA_SUPPORT == 1) || (LIBSPDM_RSA_PSS_SUPPORT == 1)
        if (!status) {
            status = libspdm_rsa_get_public_key_from_x509(data, data_size,
                                                          &spdm_context->connection_info.peer_used_leaf_cert_public_key);
        }
#endif
#if LIBSPDM_ECDSA_SUPPORT == 1
        if (!status) {
            status = libspdm_ec_get_public_key_from_x509(data, data_size,
                                                         &spdm_context->connection_info.peer_used_leaf_cert_public_key);
        }
#endif
#if (LIBSPDM_EDDSA_ED25519_SUPPORT == 1) || (LIBSPDM_EDDSA_ED448_SUPPORT == 1)
        if (!status) {
            status = libspdm_ecd_get_public_key_from_x509(data, data_size,
                                                          &spdm_context->connection_info.peer_used_leaf_cert_public_key);
        }
#endif
#if LIBSPDM_SM2_DSA_SUPPORT == 1
        if (!status) {
            status = libspdm_sm2_get_public_key_from_x509(data, data_size,
                                                          &spdm_context->connection_info.peer_used_leaf_cert_public_key);
        }
#endif
        if (!status) {
            return RETURN_UNSUPPORTED;
        }
#endif
        break;
    case LIBSPDM_DATA_BASIC_MUT_AUTH_REQUESTED:
        if (data_size != sizeof(bool)) {
            return RETURN_INVALID_PARAMETER;
        }
        mut_auth_requested = *(uint8_t *)data;
        if (((mut_auth_requested != 0) && (mut_auth_requested != 1))) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context.basic_mut_auth_requested =
            mut_auth_requested;
        spdm_context->encap_context.error_state = 0;
        spdm_context->encap_context.request_id = 0;
        spdm_context->encap_context.req_slot_id =
            parameter->additional_data[0];
        break;
    case LIBSPDM_DATA_MUT_AUTH_REQUESTED:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        mut_auth_requested = *(uint8_t *)data;
        if (((mut_auth_requested != 0) &&
             (mut_auth_requested !=
              SPDM_KEY_EXCHANGE_RESPONSE_MUT_AUTH_REQUESTED) &&
             (mut_auth_requested !=
              SPDM_KEY_EXCHANGE_RESPONSE_MUT_AUTH_REQUESTED_WITH_ENCAP_REQUEST) &&
             (mut_auth_requested !=
              SPDM_KEY_EXCHANGE_RESPONSE_MUT_AUTH_REQUESTED_WITH_GET_DIGESTS))) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context.mut_auth_requested =
            mut_auth_requested;
        spdm_context->encap_context.error_state = 0;
        spdm_context->encap_context.request_id = 0;
        spdm_context->encap_context.req_slot_id =
            parameter->additional_data[0];
        break;
    case LIBSPDM_DATA_HEARTBEAT_PERIOD:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context.heartbeat_period = *(uint8_t *)data;
        break;
    case LIBSPDM_DATA_PSK_HINT:
        if (data_size > LIBSPDM_PSK_MAX_HINT_LENGTH) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->local_context.psk_hint_size = data_size;
        spdm_context->local_context.psk_hint = data;
        break;
    case LIBSPDM_DATA_SESSION_USE_PSK:
        if (data_size != sizeof(bool)) {
            return RETURN_INVALID_PARAMETER;
        }
        session_info->use_psk = *(bool *)data;
        break;
    case LIBSPDM_DATA_SESSION_MUT_AUTH_REQUESTED:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        session_info->mut_auth_requested = *(uint8_t *)data;
        break;
    case LIBSPDM_DATA_SESSION_END_SESSION_ATTRIBUTES:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        session_info->end_session_attributes = *(uint8_t *)data;
        break;
    case LIBSPDM_DATA_SESSION_POLICY:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        session_info->session_policy = *(uint8_t *)data;
        break;
    case LIBSPDM_DATA_APP_CONTEXT_DATA:
        if (data_size != sizeof(void *) || *(void **)data == NULL) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->app_context_data_ptr = *(void **)data;
        break;
    case LIBSPDM_DATA_HANDLE_ERROR_RETURN_POLICY:
        if (data_size != sizeof(uint8_t)) {
            return RETURN_INVALID_PARAMETER;
        }
        spdm_context->handle_error_return_policy = *(uint8_t *)data;
        break;
    default:
        return RETURN_UNSUPPORTED;
        break;
    }

    return RETURN_SUCCESS;
}

/**
 * Get an SPDM context data.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  data_type                     Type of the SPDM context data.
 * @param  parameter                    Type specific parameter of the SPDM context data.
 * @param  data                         A pointer to the SPDM context data.
 * @param  data_size                     size in bytes of the SPDM context data.
 *                                     On input, it means the size in bytes of data buffer.
 *                                     On output, it means the size in bytes of copied data buffer if RETURN_SUCCESS,
 *                                     and means the size in bytes of desired data buffer if RETURN_BUFFER_TOO_SMALL.
 *
 * @retval RETURN_SUCCESS               The SPDM context data is set successfully.
 * @retval RETURN_INVALID_PARAMETER     The data_size is NULL or the data is NULL and *data_size is not zero.
 * @retval RETURN_UNSUPPORTED           The data_type is unsupported.
 * @retval RETURN_NOT_FOUND             The data_type cannot be found.
 * @retval RETURN_NOT_READY             The data is not ready to return.
 * @retval RETURN_BUFFER_TOO_SMALL      The buffer is too small to hold the data.
 **/
return_status libspdm_get_data(void *context, libspdm_data_type_t data_type,
                               const libspdm_data_parameter_t *parameter,
                               void *data, uintn *data_size)
{
    libspdm_context_t *spdm_context;
    uintn target_data_size;
    void *target_data;
    uint32_t session_id;
    libspdm_session_info_t *session_info;

    if (!context || !data || !data_size || data_type >= LIBSPDM_DATA_MAX) {
        return RETURN_INVALID_PARAMETER;
    }

    spdm_context = context;

    if (need_session_info_for_data(data_type)) {
        if (parameter->location != LIBSPDM_DATA_LOCATION_SESSION) {
            return RETURN_INVALID_PARAMETER;
        }
        session_id = *(uint32_t *)parameter->additional_data;
        session_info = libspdm_get_session_info_via_session_id(
            spdm_context, session_id);
        if (session_info == NULL) {
            return RETURN_INVALID_PARAMETER;
        }
    } else {
        session_info = NULL;
    }

    switch (data_type) {
    case LIBSPDM_DATA_SPDM_VERSION:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(spdm_version_number_t);
        target_data =
            &(spdm_context->connection_info.version);
        break;
    case LIBSPDM_DATA_SECURED_MESSAGE_VERSION:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(spdm_version_number_t);
        target_data =
            &(spdm_context->connection_info.secured_message_version);
        break;
    case LIBSPDM_DATA_CAPABILITY_FLAGS:
        target_data_size = sizeof(uint32_t);
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            target_data =
                &spdm_context->connection_info.capability.flags;
        } else {
            target_data =
                &spdm_context->local_context.capability.flags;
        }
        break;
    case LIBSPDM_DATA_CAPABILITY_CT_EXPONENT:
        target_data_size = sizeof(uint8_t);
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            target_data = &spdm_context->connection_info.capability
                          .ct_exponent;
        } else {
            target_data = &spdm_context->local_context.capability
                          .ct_exponent;
        }
        break;
    case LIBSPDM_DATA_CAPABILITY_DATA_TRANSFER_SIZE:
        target_data_size = sizeof(uint32_t);
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            target_data =
                &spdm_context->connection_info.capability.data_transfer_size;
        } else {
            target_data =
                &spdm_context->local_context.capability.data_transfer_size;
        }
        break;
    case LIBSPDM_DATA_CAPABILITY_MAX_SPDM_MSG_SIZE:
        target_data_size = sizeof(uint32_t);
        if (parameter->location == LIBSPDM_DATA_LOCATION_CONNECTION) {
            target_data =
                &spdm_context->connection_info.capability.max_spdm_msg_size;
        } else {
            target_data =
                &spdm_context->local_context.capability.max_spdm_msg_size;
        }
        break;
    case LIBSPDM_DATA_MEASUREMENT_SPEC:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint8_t);
        target_data = &spdm_context->connection_info.algorithm
                      .measurement_spec;
        break;
    case LIBSPDM_DATA_MEASUREMENT_HASH_ALGO:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint32_t);
        target_data = &spdm_context->connection_info.algorithm
                      .measurement_hash_algo;
        break;
    case LIBSPDM_DATA_BASE_ASYM_ALGO:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint32_t);
        target_data =
            &spdm_context->connection_info.algorithm.base_asym_algo;
        break;
    case LIBSPDM_DATA_BASE_HASH_ALGO:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint32_t);
        target_data =
            &spdm_context->connection_info.algorithm.base_hash_algo;
        break;
    case LIBSPDM_DATA_DHE_NAME_GROUP:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint16_t);
        target_data =
            &spdm_context->connection_info.algorithm.dhe_named_group;
        break;
    case LIBSPDM_DATA_AEAD_CIPHER_SUITE:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint16_t);
        target_data = &spdm_context->connection_info.algorithm
                      .aead_cipher_suite;
        break;
    case LIBSPDM_DATA_REQ_BASE_ASYM_ALG:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint16_t);
        target_data = &spdm_context->connection_info.algorithm
                      .req_base_asym_alg;
        break;
    case LIBSPDM_DATA_KEY_SCHEDULE:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint16_t);
        target_data =
            &spdm_context->connection_info.algorithm.key_schedule;
        break;
    case LIBSPDM_DATA_OTHER_PARAMS_SUPPORT:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint8_t);
        target_data = &spdm_context->connection_info.algorithm
                      .other_params_support;
        break;
    case LIBSPDM_DATA_CONNECTION_STATE:
        if (parameter->location != LIBSPDM_DATA_LOCATION_CONNECTION) {
            return RETURN_INVALID_PARAMETER;
        }
        target_data_size = sizeof(uint32_t);
        target_data = &spdm_context->connection_info.connection_state;
        break;
    case LIBSPDM_DATA_RESPONSE_STATE:
        target_data_size = sizeof(uint32_t);
        target_data = &spdm_context->response_state;
        break;
    case LIBSPDM_DATA_SESSION_USE_PSK:
        target_data_size = sizeof(bool);
        target_data = &session_info->use_psk;
        break;
    case LIBSPDM_DATA_SESSION_MUT_AUTH_REQUESTED:
        target_data_size = sizeof(uint8_t);
        target_data = &session_info->mut_auth_requested;
        break;
    case LIBSPDM_DATA_SESSION_END_SESSION_ATTRIBUTES:
        target_data_size = sizeof(uint8_t);
        target_data = &session_info->end_session_attributes;
        break;
    case LIBSPDM_DATA_SESSION_POLICY:
        target_data_size = sizeof(uint8_t);
        target_data = &session_info->session_policy;
        break;
    case LIBSPDM_DATA_APP_CONTEXT_DATA:
        target_data_size = sizeof(void *);
        target_data = &spdm_context->app_context_data_ptr;
        break;
    case LIBSPDM_DATA_HANDLE_ERROR_RETURN_POLICY:
        target_data_size = sizeof(uint8_t);
        target_data = &spdm_context->handle_error_return_policy;
        break;
    default:
        return RETURN_UNSUPPORTED;
        break;
    }

    if (*data_size < target_data_size) {
        *data_size = target_data_size;
        return RETURN_BUFFER_TOO_SMALL;
    }
    libspdm_copy_mem(data, *data_size, target_data, target_data_size);
    *data_size = target_data_size;

    return RETURN_SUCCESS;
}

/**
 * Reset message A cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 **/
void libspdm_reset_message_a(void *context)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    libspdm_reset_managed_buffer(&spdm_context->transcript.message_a);
}

/**
 * Reset message B cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 **/
void libspdm_reset_message_b(void *context)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    libspdm_reset_managed_buffer(&spdm_context->transcript.message_b);
#else
    if (spdm_context->transcript.digest_context_m1m2 != NULL) {
        libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                           spdm_context->transcript.digest_context_m1m2);
        spdm_context->transcript.digest_context_m1m2 = NULL;
    }
#endif
}

/**
 * Reset message C cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 **/
void libspdm_reset_message_c(void *context)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    libspdm_reset_managed_buffer(&spdm_context->transcript.message_c);
#else
    if (spdm_context->transcript.digest_context_m1m2 != NULL) {
        libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                           spdm_context->transcript.digest_context_m1m2);
        spdm_context->transcript.digest_context_m1m2 = NULL;
    }
#endif
}

/**
 * Reset message MutB cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 **/
void libspdm_reset_message_mut_b(void *context)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    libspdm_reset_managed_buffer(&spdm_context->transcript.message_mut_b);
#else
    if (spdm_context->transcript.digest_context_mut_m1m2 != NULL) {
        libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                           spdm_context->transcript.digest_context_mut_m1m2);
        spdm_context->transcript.digest_context_mut_m1m2 = NULL;
    }
#endif
}

/**
 * Reset message MutC cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 **/
void libspdm_reset_message_mut_c(void *context)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    libspdm_reset_managed_buffer(&spdm_context->transcript.message_mut_c);
#else
    if (spdm_context->transcript.digest_context_mut_m1m2 != NULL) {
        libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                           spdm_context->transcript.digest_context_mut_m1m2);
        spdm_context->transcript.digest_context_mut_m1m2 = NULL;
    }
#endif
}

/**
 * Reset message M cache in SPDM context.
 * If session_info is NULL, this function will use M cache of SPDM context,
 * else will use M cache of SPDM session context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_info                  A pointer to the SPDM session context.
 **/
void libspdm_reset_message_m(void *context, void *session_info)
{
    libspdm_context_t *spdm_context;
    libspdm_session_info_t *spdm_session_info;

    spdm_context = context;
    spdm_session_info = session_info;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    if (spdm_session_info == NULL) {
        libspdm_reset_managed_buffer(&spdm_context->transcript.message_m);
    } else {
        libspdm_reset_managed_buffer(&spdm_session_info->session_transcript.message_m);
    }
#else
    if (spdm_session_info == NULL) {
        if (spdm_context->transcript.digest_context_l1l2 != NULL) {
            libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                               spdm_context->transcript.digest_context_l1l2);
            spdm_context->transcript.digest_context_l1l2 = NULL;
        }
    } else {
        if (spdm_session_info->session_transcript.digest_context_l1l2 != NULL) {
            libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                               spdm_session_info->session_transcript.digest_context_l1l2);
            spdm_session_info->session_transcript.digest_context_l1l2 = NULL;
        }
    }
#endif
}

/**
 * Reset message K cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  spdm_session_info              A pointer to the SPDM session context.
 **/
void libspdm_reset_message_k(void *context, void *session_info)
{
    libspdm_session_info_t *spdm_session_info;

    spdm_session_info = session_info;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    libspdm_reset_managed_buffer(&spdm_session_info->session_transcript.message_k);
#else
    {
        libspdm_context_t *spdm_context;
        void *secured_message_context;

        spdm_context = context;
        secured_message_context = spdm_session_info->secured_message_context;

        libspdm_reset_managed_buffer(&spdm_session_info->session_transcript.temp_message_k);

        if (spdm_session_info->session_transcript.digest_context_th != NULL) {
            libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                               spdm_session_info->session_transcript.digest_context_th);
            spdm_session_info->session_transcript.digest_context_th = NULL;
        }
        if (spdm_session_info->session_transcript.hmac_rsp_context_th != NULL) {
            libspdm_hmac_free_with_response_finished_key (secured_message_context,
                                                          spdm_session_info->session_transcript.hmac_rsp_context_th);
            spdm_session_info->session_transcript.hmac_rsp_context_th = NULL;
        }
        if (spdm_session_info->session_transcript.hmac_req_context_th != NULL) {
            libspdm_hmac_free_with_request_finished_key (secured_message_context,
                                                         spdm_session_info->session_transcript.hmac_req_context_th);
            spdm_session_info->session_transcript.hmac_req_context_th = NULL;
        }
        if (spdm_session_info->session_transcript.digest_context_th_backup != NULL) {
            libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                               spdm_session_info->session_transcript.digest_context_th_backup);
            spdm_session_info->session_transcript.digest_context_th_backup = NULL;
        }
        if (spdm_session_info->session_transcript.hmac_rsp_context_th_backup != NULL) {
            libspdm_hmac_free_with_response_finished_key (secured_message_context,
                                                          spdm_session_info->session_transcript.hmac_rsp_context_th_backup);
            spdm_session_info->session_transcript.hmac_rsp_context_th_backup = NULL;
        }
        if (spdm_session_info->session_transcript.hmac_req_context_th_backup != NULL) {
            libspdm_hmac_free_with_request_finished_key (secured_message_context,
                                                         spdm_session_info->session_transcript.hmac_req_context_th_backup);
            spdm_session_info->session_transcript.hmac_req_context_th_backup = NULL;
        }
        spdm_session_info->session_transcript.finished_key_ready = false;
    }
#endif
}

/**
 * Reset message F cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  spdm_session_info              A pointer to the SPDM session context.
 **/
void libspdm_reset_message_f(void *context, void *session_info)
{
    libspdm_session_info_t *spdm_session_info;

    spdm_session_info = session_info;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    libspdm_reset_managed_buffer(&spdm_session_info->session_transcript.message_f);
#else
    {
        libspdm_context_t *spdm_context;
        void *secured_message_context;

        spdm_context = context;
        secured_message_context = spdm_session_info->secured_message_context;

        if (spdm_session_info->session_transcript.digest_context_th != NULL) {
            libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                               spdm_session_info->session_transcript.digest_context_th);
            spdm_session_info->session_transcript.digest_context_th =
                spdm_session_info->session_transcript.digest_context_th_backup;
            spdm_session_info->session_transcript.digest_context_th_backup = NULL;
        }
        if (spdm_session_info->session_transcript.hmac_rsp_context_th != NULL) {
            libspdm_hmac_free_with_response_finished_key (secured_message_context,
                                                          spdm_session_info->session_transcript.hmac_rsp_context_th);
            spdm_session_info->session_transcript.hmac_rsp_context_th =
                spdm_session_info->session_transcript.hmac_rsp_context_th_backup;
            spdm_session_info->session_transcript.hmac_rsp_context_th_backup = NULL;
        }
        if (spdm_session_info->session_transcript.hmac_req_context_th != NULL) {
            libspdm_hmac_free_with_response_finished_key (secured_message_context,
                                                          spdm_session_info->session_transcript.hmac_req_context_th);
            spdm_session_info->session_transcript.hmac_req_context_th =
                spdm_session_info->session_transcript.hmac_req_context_th_backup;
            spdm_session_info->session_transcript.hmac_req_context_th_backup = NULL;
        }
        spdm_session_info->session_transcript.message_f_initialized = false;
    }
#endif
}

/**
 * Reset message buffer in SPDM context according to request code.
 *
 * @param  spdm_context                   A pointer to the SPDM context.
 * @param  spdm_session_info             A pointer to the SPDM session context.
 * @param  spdm_request                   The SPDM request code.
 */
void libspdm_reset_message_buffer_via_request_code(void *context, void *session_info,
                                                   uint8_t request_code)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    /**
     * Any request other than SPDM_GET_MEASUREMENTS resets L1/L2
     */
    if (request_code != SPDM_GET_MEASUREMENTS) {
        libspdm_reset_message_m(spdm_context, session_info);
    }
    /**
     * If the Requester issued GET_MEASUREMENTS or KEY_EXCHANGE or FINISH or PSK_EXCHANGE
     * or PSK_FINISH or KEY_UPDATE or HEARTBEAT or GET_ENCAPSULATED_REQUEST or DELIVER_ENCAPSULATED_RESPONSE
     * or END_SESSSION request(s) and skipped CHALLENGE completion, M1 and M2 are reset to null.
     */
    switch (request_code)
    {
    case SPDM_KEY_EXCHANGE:
    case SPDM_GET_MEASUREMENTS:
    case SPDM_FINISH:
    case SPDM_PSK_EXCHANGE:
    case SPDM_PSK_FINISH:
    case SPDM_KEY_UPDATE:
    case SPDM_HEARTBEAT:
    case SPDM_GET_ENCAPSULATED_REQUEST:
    case SPDM_END_SESSION:
        if (spdm_context->connection_info.connection_state <
            LIBSPDM_CONNECTION_STATE_AUTHENTICATED) {
            libspdm_reset_message_b(spdm_context);
            libspdm_reset_message_c(spdm_context);
            libspdm_reset_message_mut_b(spdm_context);
            libspdm_reset_message_mut_c(spdm_context);
        }
        break;
    case SPDM_DELIVER_ENCAPSULATED_RESPONSE:
        if (spdm_context->connection_info.connection_state <
            LIBSPDM_CONNECTION_STATE_AUTHENTICATED) {
            libspdm_reset_message_b(spdm_context);
            libspdm_reset_message_c(spdm_context);
        }
        break;
    default:
        break;
    }
}
/**
 * Append message A cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_a(void *context, const void *message,
                                       uintn message_size)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    return libspdm_append_managed_buffer(&spdm_context->transcript.message_a,
                                         message, message_size);
}

/**
 * Append message B cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_b(void *context, const void *message,
                                       uintn message_size)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    return libspdm_append_managed_buffer(&spdm_context->transcript.message_b,
                                         message, message_size);
#else
    {
        bool result;

        if (spdm_context->transcript.digest_context_m1m2 == NULL) {
            spdm_context->transcript.digest_context_m1m2 = libspdm_hash_new (
                spdm_context->connection_info.algorithm.base_hash_algo);
            if (spdm_context->transcript.digest_context_m1m2 == NULL) {
                return RETURN_DEVICE_ERROR;
            }
            result = libspdm_hash_init (spdm_context->connection_info.algorithm.base_hash_algo,
                                        spdm_context->transcript.digest_context_m1m2);
            if (!result) {
                libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                   spdm_context->transcript.digest_context_m1m2);
                spdm_context->transcript.digest_context_m1m2 = NULL;
                return RETURN_DEVICE_ERROR;
            }
            result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                          spdm_context->transcript.digest_context_m1m2,
                                          libspdm_get_managed_buffer(&spdm_context->transcript.
                                                                     message_a),
                                          libspdm_get_managed_buffer_size(&spdm_context->transcript.
                                                                          message_a));
            if (!result) {
                libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                   spdm_context->transcript.digest_context_m1m2);
                spdm_context->transcript.digest_context_m1m2 = NULL;
                return RETURN_DEVICE_ERROR;
            }
        }

        result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                      spdm_context->transcript.digest_context_m1m2, message,
                                      message_size);
        if (!result) {
            return RETURN_DEVICE_ERROR;
        }

        return RETURN_SUCCESS;
    }
#endif
}

/**
 * Append message C cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_c(void *context, const void *message,
                                       uintn message_size)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    return libspdm_append_managed_buffer(&spdm_context->transcript.message_c,
                                         message, message_size);
#else
    {
        bool result;

        if (spdm_context->transcript.digest_context_m1m2 == NULL) {
            spdm_context->transcript.digest_context_m1m2 = libspdm_hash_new (
                spdm_context->connection_info.algorithm.base_hash_algo);
            if (spdm_context->transcript.digest_context_m1m2 == NULL) {
                return RETURN_DEVICE_ERROR;
            }
            result = libspdm_hash_init (spdm_context->connection_info.algorithm.base_hash_algo,
                                        spdm_context->transcript.digest_context_m1m2);
            if (!result) {
                libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                   spdm_context->transcript.digest_context_m1m2);
                spdm_context->transcript.digest_context_m1m2 = NULL;
                return RETURN_DEVICE_ERROR;
            }
            result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                          spdm_context->transcript.digest_context_m1m2,
                                          libspdm_get_managed_buffer(&spdm_context->transcript.
                                                                     message_a),
                                          libspdm_get_managed_buffer_size(&spdm_context->transcript.
                                                                          message_a));
            if (!result) {
                libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                   spdm_context->transcript.digest_context_m1m2);
                spdm_context->transcript.digest_context_m1m2 = NULL;
                return RETURN_DEVICE_ERROR;
            }
        }

        result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                      spdm_context->transcript.digest_context_m1m2, message,
                                      message_size);
        if (!result) {
            return RETURN_DEVICE_ERROR;
        }

        return RETURN_SUCCESS;
    }
#endif
}

/**
 * Append message MutB cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_mut_b(void *context, const void *message,
                                           uintn message_size)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    return libspdm_append_managed_buffer(&spdm_context->transcript.message_mut_b,
                                         message, message_size);
#else
    {
        bool result;

        if (spdm_context->transcript.digest_context_mut_m1m2 == NULL) {
            spdm_context->transcript.digest_context_mut_m1m2 = libspdm_hash_new (
                spdm_context->connection_info.algorithm.base_hash_algo);
            if (spdm_context->transcript.digest_context_mut_m1m2 == NULL) {
                return RETURN_DEVICE_ERROR;
            }
            result = libspdm_hash_init (spdm_context->connection_info.algorithm.base_hash_algo,
                                        spdm_context->transcript.digest_context_mut_m1m2);
            if (!result) {
                libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                   spdm_context->transcript.digest_context_mut_m1m2);
                spdm_context->transcript.digest_context_mut_m1m2 = NULL;
                return RETURN_DEVICE_ERROR;
            }
        }

        result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                      spdm_context->transcript.digest_context_mut_m1m2, message,
                                      message_size);
        if (!result) {
            return RETURN_DEVICE_ERROR;
        }

        return RETURN_SUCCESS;
    }
#endif
}

/**
 * Append message MutC cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_mut_c(void *context, const void *message,
                                           uintn message_size)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    return libspdm_append_managed_buffer(&spdm_context->transcript.message_mut_c,
                                         message, message_size);
#else
    {
        bool result;

        if (spdm_context->transcript.digest_context_mut_m1m2 == NULL) {
            spdm_context->transcript.digest_context_mut_m1m2 = libspdm_hash_new (
                spdm_context->connection_info.algorithm.base_hash_algo);
            if (spdm_context->transcript.digest_context_mut_m1m2 == NULL) {
                return RETURN_DEVICE_ERROR;
            }
            result = libspdm_hash_init (spdm_context->connection_info.algorithm.base_hash_algo,
                                        spdm_context->transcript.digest_context_mut_m1m2);
            if (!result) {
                libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                   spdm_context->transcript.digest_context_mut_m1m2);
                spdm_context->transcript.digest_context_mut_m1m2 = NULL;
                return RETURN_DEVICE_ERROR;
            }
        }

        result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                      spdm_context->transcript.digest_context_mut_m1m2, message,
                                      message_size);
        if (!result) {
            return RETURN_DEVICE_ERROR;
        }

        return RETURN_SUCCESS;
    }
#endif
}

/**
 * Append message M cache in SPDM context.
 * If session_info is NULL, this function will use M cache of SPDM context,
 * else will use M cache of SPDM session context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  session_info                  A pointer to the SPDM session context.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_m(void *context, void *session_info,
                                       const void *message, uintn message_size)
{
    libspdm_context_t *spdm_context;
    libspdm_session_info_t *spdm_session_info;

    spdm_context = context;
    spdm_session_info = session_info;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    if (spdm_session_info == NULL) {
        return libspdm_append_managed_buffer(&spdm_context->transcript.message_m,
                                             message, message_size);
    } else {
        return libspdm_append_managed_buffer(&spdm_session_info->session_transcript.message_m,
                                             message, message_size);
    }
#else
    {
        bool result;

        if (spdm_session_info == NULL) {
            if (spdm_context->transcript.digest_context_l1l2 == NULL) {
                spdm_context->transcript.digest_context_l1l2 = libspdm_hash_new (
                    spdm_context->connection_info.algorithm.base_hash_algo);
                if (spdm_context->transcript.digest_context_l1l2 == NULL) {
                    return RETURN_DEVICE_ERROR;
                }
                result = libspdm_hash_init (spdm_context->connection_info.algorithm.base_hash_algo,
                                            spdm_context->transcript.digest_context_l1l2);
                if (!result) {
                    libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                       spdm_context->transcript.digest_context_l1l2);
                    spdm_context->transcript.digest_context_l1l2 = NULL;
                    return RETURN_DEVICE_ERROR;
                }
            }
            if ((spdm_context->connection_info.version >> SPDM_VERSION_NUMBER_SHIFT_BIT) >
                SPDM_MESSAGE_VERSION_11) {

                /* Need append VCA since 1.2 script*/

                result = libspdm_hash_update (
                    spdm_context->connection_info.algorithm.base_hash_algo,
                    spdm_context->transcript.digest_context_l1l2,
                    libspdm_get_managed_buffer(
                        &spdm_context->transcript.message_a),
                    libspdm_get_managed_buffer_size(&spdm_context->transcript.
                                                    message_a));
                if (!result) {
                    libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                       spdm_context->transcript.digest_context_l1l2);
                    spdm_context->transcript.digest_context_l1l2 = NULL;
                    return RETURN_DEVICE_ERROR;
                }
            }
            result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                          spdm_context->transcript.digest_context_l1l2, message,
                                          message_size);
        } else {
            if (spdm_session_info->session_transcript.digest_context_l1l2 == NULL) {
                spdm_session_info->session_transcript.digest_context_l1l2 = libspdm_hash_new (
                    spdm_context->connection_info.algorithm.base_hash_algo);
                if (spdm_session_info->session_transcript.digest_context_l1l2 == NULL) {
                    return RETURN_DEVICE_ERROR;
                }
                result = libspdm_hash_init (spdm_context->connection_info.algorithm.base_hash_algo,
                                            spdm_session_info->session_transcript.digest_context_l1l2);
                if (!result) {
                    libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                       spdm_session_info->session_transcript.digest_context_l1l2);
                    spdm_session_info->session_transcript.digest_context_l1l2 = NULL;
                    return RETURN_DEVICE_ERROR;
                }
            }
            if ((spdm_context->connection_info.version >> SPDM_VERSION_NUMBER_SHIFT_BIT) >
                SPDM_MESSAGE_VERSION_11) {

                /* Need append VCA since 1.2 script*/

                result = libspdm_hash_update (
                    spdm_context->connection_info.algorithm.base_hash_algo,
                    spdm_session_info->session_transcript.digest_context_l1l2,
                    libspdm_get_managed_buffer(
                        &spdm_context->transcript.message_a),
                    libspdm_get_managed_buffer_size(&spdm_context->transcript.
                                                    message_a));
                if (!result) {
                    libspdm_hash_free (spdm_context->connection_info.algorithm.base_hash_algo,
                                       spdm_session_info->session_transcript.digest_context_l1l2);
                    spdm_session_info->session_transcript.digest_context_l1l2 = NULL;
                    return RETURN_DEVICE_ERROR;
                }
            }
            result = libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                          spdm_session_info->session_transcript.digest_context_l1l2,
                                          message, message_size);
        }

        if (!result) {
            return RETURN_DEVICE_ERROR;
        }

        return RETURN_SUCCESS;
    }
#endif
}

/**
 * Append message K cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  spdm_session_info              A pointer to the SPDM session context.
 * @param  is_requester                  Indicate of the key generation for a requester or a responder.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_k(void *context, void *session_info,
                                       bool is_requester, const void *message,
                                       uintn message_size)
{
    libspdm_session_info_t *spdm_session_info;

    spdm_session_info = session_info;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    return libspdm_append_managed_buffer(
        &spdm_session_info->session_transcript.message_k, message,
        message_size);
#else
    {
        libspdm_context_t *spdm_context;
        void *secured_message_context;
        uint8_t *cert_chain_buffer;
        uintn cert_chain_buffer_size;
        bool result;
        uint8_t cert_chain_buffer_hash[LIBSPDM_MAX_HASH_SIZE];
        uint32_t hash_size;
        bool finished_key_ready;

        spdm_context = context;
        secured_message_context = spdm_session_info->secured_message_context;
        finished_key_ready = libspdm_secured_message_is_finished_key_ready(secured_message_context);

        if (spdm_session_info->session_transcript.digest_context_th == NULL) {
            if (!spdm_session_info->use_psk) {
                if (is_requester) {
                    if(spdm_context->connection_info.peer_used_cert_chain_buffer_hash_size != 0) {
                        hash_size =
                            spdm_context->connection_info.peer_used_cert_chain_buffer_hash_size;
                        libspdm_copy_mem(cert_chain_buffer_hash,
                                         sizeof(cert_chain_buffer_hash),
                                         spdm_context->connection_info.peer_used_cert_chain_buffer_hash,
                                         hash_size);
                        result = true;
                    } else {
                        result = libspdm_get_peer_cert_chain_buffer(
                            spdm_context, (void **)&cert_chain_buffer, &cert_chain_buffer_size);
                        if (!result) {
                            return false;
                        }
                        hash_size = libspdm_get_hash_size(
                            spdm_context->connection_info.algorithm.base_hash_algo);
                        result = libspdm_hash_all(
                            spdm_context->connection_info.algorithm.base_hash_algo,
                            cert_chain_buffer, cert_chain_buffer_size,
                            cert_chain_buffer_hash);
                        if (!result) {
                            return false;
                        }

                    }
                } else {
                    result = libspdm_get_local_cert_chain_buffer(
                        spdm_context, (void **)&cert_chain_buffer, &cert_chain_buffer_size);

                    if (!result) {
                        return false;
                    }
                    hash_size = libspdm_get_hash_size(
                        spdm_context->connection_info.algorithm.base_hash_algo);
                    result = libspdm_hash_all(
                        spdm_context->connection_info.algorithm.base_hash_algo,
                        cert_chain_buffer, cert_chain_buffer_size,
                        cert_chain_buffer_hash);
                    if (!result) {
                        return false;
                    }
                }
            }
        }


        /* prepare digest_context_th*/

        if (spdm_session_info->session_transcript.digest_context_th == NULL) {
            spdm_session_info->session_transcript.digest_context_th = libspdm_hash_new (
                spdm_context->connection_info.algorithm.base_hash_algo);
            libspdm_hash_init (spdm_context->connection_info.algorithm.base_hash_algo,
                               spdm_session_info->session_transcript.digest_context_th);
            libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                 spdm_session_info->session_transcript.digest_context_th,
                                 libspdm_get_managed_buffer(&spdm_context->transcript.message_a),
                                 libspdm_get_managed_buffer_size(
                                     &spdm_context->transcript.message_a));
            libspdm_append_managed_buffer(
                &spdm_session_info->session_transcript.temp_message_k,
                libspdm_get_managed_buffer(&spdm_context->transcript.message_a),
                libspdm_get_managed_buffer_size(&spdm_context->transcript.message_a));
            if (!spdm_session_info->use_psk) {
                libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                     spdm_session_info->session_transcript.digest_context_th,
                                     cert_chain_buffer_hash, hash_size);
                libspdm_append_managed_buffer(
                    &spdm_session_info->session_transcript.temp_message_k,
                    cert_chain_buffer_hash, hash_size);
            }
        }
        libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                             spdm_session_info->session_transcript.digest_context_th, message,
                             message_size);
        if (!finished_key_ready) {

            /* append message only if finished_key is NOT ready.*/

            libspdm_append_managed_buffer(
                &spdm_session_info->session_transcript.temp_message_k, message, message_size);
        }


        /* Above action is to calculate HASH for message_k.
         * However, we cannot use similar way to calculate HMAC. (chicken-egg problem)
         * HMAC need finished_key, and finished_key calculation need message_k.
         * If the finished_key is NOT ready, then we cannot calculate HMAC. We have to cache to temp_message_k and stop here.
         * If the finished_key is ready, then we can start calculating HMAC. No need to cache temp_message_k.*/

        if (!finished_key_ready) {
            return RETURN_SUCCESS;
        }


        /* prepare hmac_rsp_context_th*/

        if (spdm_session_info->session_transcript.hmac_rsp_context_th == NULL) {
            spdm_session_info->session_transcript.hmac_rsp_context_th =
                libspdm_hmac_new_with_response_finished_key (
                    secured_message_context);
            libspdm_hmac_init_with_response_finished_key (secured_message_context,
                                                          spdm_session_info->session_transcript.hmac_rsp_context_th);
            libspdm_hmac_update_with_response_finished_key (secured_message_context,
                                                            spdm_session_info->session_transcript.hmac_rsp_context_th,
                                                            libspdm_get_managed_buffer(&
                                                                                       spdm_session_info
                                                                                       ->
                                                                                       session_transcript
                                                                                       .
                                                                                       temp_message_k),
                                                            libspdm_get_managed_buffer_size(&
                                                                                            spdm_session_info
                                                                                            ->
                                                                                            session_transcript
                                                                                            .
                                                                                            temp_message_k));
        }
        libspdm_hmac_update_with_response_finished_key (secured_message_context,
                                                        spdm_session_info->session_transcript.hmac_rsp_context_th, message,
                                                        message_size);


        /* prepare hmac_req_context_th*/

        if (spdm_session_info->session_transcript.hmac_req_context_th == NULL) {
            spdm_session_info->session_transcript.hmac_req_context_th =
                libspdm_hmac_new_with_request_finished_key (
                    secured_message_context);
            libspdm_hmac_init_with_request_finished_key (secured_message_context,
                                                         spdm_session_info->session_transcript.hmac_req_context_th);
            libspdm_hmac_update_with_request_finished_key (secured_message_context,
                                                           spdm_session_info->session_transcript.hmac_req_context_th,
                                                           libspdm_get_managed_buffer(&
                                                                                      spdm_session_info
                                                                                      ->
                                                                                      session_transcript
                                                                                      .
                                                                                      temp_message_k),
                                                           libspdm_get_managed_buffer_size(&
                                                                                           spdm_session_info
                                                                                           ->
                                                                                           session_transcript
                                                                                           .
                                                                                           temp_message_k));
        }
        libspdm_hmac_update_with_request_finished_key (secured_message_context,
                                                       spdm_session_info->session_transcript.hmac_req_context_th, message,
                                                       message_size);
        return RETURN_SUCCESS;
    }
#endif
}

/**
 * Append message F cache in SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  spdm_session_info              A pointer to the SPDM session context.
 * @param  is_requester                  Indicate of the key generation for a requester or a responder.
 * @param  message                      message buffer.
 * @param  message_size                  size in bytes of message buffer.
 *
 * @return RETURN_SUCCESS          message is appended.
 * @return RETURN_OUT_OF_RESOURCES message is not appended because the internal cache is full.
 **/
return_status libspdm_append_message_f(void *context, void *session_info,
                                       bool is_requester, const void *message,
                                       uintn message_size)
{
    libspdm_session_info_t *spdm_session_info;

    spdm_session_info = session_info;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    return libspdm_append_managed_buffer(
        &spdm_session_info->session_transcript.message_f, message,
        message_size);
#else
    {
        libspdm_context_t *spdm_context;
        void *secured_message_context;
        uint8_t *mut_cert_chain_buffer;
        uintn mut_cert_chain_buffer_size;
        bool result;
        uint8_t mut_cert_chain_buffer_hash[LIBSPDM_MAX_HASH_SIZE];
        uint32_t hash_size;
        bool finished_key_ready;
        return_status status;

        spdm_context = context;
        secured_message_context = spdm_session_info->secured_message_context;
        finished_key_ready = libspdm_secured_message_is_finished_key_ready(secured_message_context);
        LIBSPDM_ASSERT (finished_key_ready);

        if (!spdm_session_info->session_transcript.message_f_initialized) {

            /* digest_context_th might be NULL in unit test, where message_k is hardcoded.
             * hmac_{rsp,req}_context_th might be NULL in real case, because
             *   after finished_key_ready is generated, no one trigger libspdm_append_message_k.
             * trigger message_k to initialize by using zero length message_k, no impact to hash or HMAC.
             *   only temp_message_k is appended.*/

            if (spdm_session_info->session_transcript.digest_context_th == NULL ||
                spdm_session_info->session_transcript.hmac_rsp_context_th == NULL ||
                spdm_session_info->session_transcript.hmac_req_context_th == NULL) {
                status = libspdm_append_message_k (context, session_info, is_requester, NULL, 0);
                if (RETURN_ERROR(status)) {
                    return status;
                }
            }

            if (!spdm_session_info->use_psk && spdm_session_info->mut_auth_requested) {
                if (is_requester) {
                    result = libspdm_get_local_cert_chain_buffer(
                        spdm_context,
                        (void **)&mut_cert_chain_buffer,
                        &mut_cert_chain_buffer_size);
                    if (!result) {
                        return RETURN_UNSUPPORTED;
                    }
                    hash_size = libspdm_get_hash_size(
                        spdm_context->connection_info.algorithm.base_hash_algo);
                    result = libspdm_hash_all(
                        spdm_context->connection_info.algorithm.base_hash_algo,
                        mut_cert_chain_buffer, mut_cert_chain_buffer_size,
                        mut_cert_chain_buffer_hash);
                    if (!result) {
                        return RETURN_DEVICE_ERROR;
                    }
                } else {
                    if (spdm_context->connection_info.peer_used_cert_chain_buffer_hash_size != 0) {
                        hash_size =
                            spdm_context->connection_info.peer_used_cert_chain_buffer_hash_size;
                        libspdm_copy_mem(mut_cert_chain_buffer_hash,
                                         sizeof(mut_cert_chain_buffer_hash),
                                         spdm_context->connection_info.peer_used_cert_chain_buffer_hash,
                                         hash_size);
                        result = true;
                    } else {
                        result = libspdm_get_peer_cert_chain_buffer(
                            spdm_context,
                            (void **)&mut_cert_chain_buffer,
                            &mut_cert_chain_buffer_size);
                        if (!result) {
                            return RETURN_UNSUPPORTED;
                        }
                        hash_size = libspdm_get_hash_size(
                            spdm_context->connection_info.algorithm.base_hash_algo);
                        result = libspdm_hash_all(
                            spdm_context->connection_info.algorithm.base_hash_algo,
                            mut_cert_chain_buffer, mut_cert_chain_buffer_size,
                            mut_cert_chain_buffer_hash);
                        if (!result) {
                            return RETURN_DEVICE_ERROR;
                        }
                    }
                }
            }

            /* It is first time call, backup current message_k context
             * this backup will be used in reset_message_f.*/

            LIBSPDM_ASSERT (spdm_session_info->session_transcript.digest_context_th != NULL);
            spdm_session_info->session_transcript.digest_context_th_backup = libspdm_hash_new (
                spdm_context->connection_info.algorithm.base_hash_algo);
            libspdm_hash_duplicate (spdm_context->connection_info.algorithm.base_hash_algo,
                                    spdm_session_info->session_transcript.digest_context_th,
                                    spdm_session_info->session_transcript.digest_context_th_backup);

            LIBSPDM_ASSERT (spdm_session_info->session_transcript.hmac_rsp_context_th != NULL);
            spdm_session_info->session_transcript.hmac_rsp_context_th_backup =
                libspdm_hmac_new_with_response_finished_key (
                    secured_message_context);
            libspdm_hmac_duplicate_with_response_finished_key (secured_message_context,
                                                               spdm_session_info->session_transcript.hmac_rsp_context_th,
                                                               spdm_session_info->session_transcript.hmac_rsp_context_th_backup);

            LIBSPDM_ASSERT (spdm_session_info->session_transcript.hmac_req_context_th != NULL);
            spdm_session_info->session_transcript.hmac_req_context_th_backup =
                libspdm_hmac_new_with_request_finished_key (
                    secured_message_context);
            libspdm_hmac_duplicate_with_request_finished_key (secured_message_context,
                                                              spdm_session_info->session_transcript.hmac_req_context_th,
                                                              spdm_session_info->session_transcript.hmac_req_context_th_backup);
        }


        /* prepare digest_context_th*/

        LIBSPDM_ASSERT (spdm_session_info->session_transcript.digest_context_th != NULL);
        if (!spdm_session_info->session_transcript.message_f_initialized) {
            if (!spdm_session_info->use_psk && spdm_session_info->mut_auth_requested) {
                libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                                     spdm_session_info->session_transcript.digest_context_th,
                                     mut_cert_chain_buffer_hash, hash_size);
            }
        }
        libspdm_hash_update (spdm_context->connection_info.algorithm.base_hash_algo,
                             spdm_session_info->session_transcript.digest_context_th, message,
                             message_size);


        /* prepare hmac_rsp_context_th*/

        LIBSPDM_ASSERT (spdm_session_info->session_transcript.hmac_rsp_context_th != NULL);
        if (!spdm_session_info->session_transcript.message_f_initialized) {
            if (!spdm_session_info->use_psk && spdm_session_info->mut_auth_requested) {
                libspdm_hmac_update_with_response_finished_key (secured_message_context,
                                                                spdm_session_info->session_transcript.hmac_rsp_context_th, mut_cert_chain_buffer_hash,
                                                                hash_size);
            }
        }
        libspdm_hmac_update_with_response_finished_key (secured_message_context,
                                                        spdm_session_info->session_transcript.hmac_rsp_context_th, message,
                                                        message_size);


        /* prepare hmac_req_context_th*/

        LIBSPDM_ASSERT (spdm_session_info->session_transcript.hmac_req_context_th != NULL);
        if (!spdm_session_info->session_transcript.message_f_initialized) {
            if (!spdm_session_info->use_psk && spdm_session_info->mut_auth_requested) {
                libspdm_hmac_update_with_request_finished_key (secured_message_context,
                                                               spdm_session_info->session_transcript.hmac_req_context_th, mut_cert_chain_buffer_hash,
                                                               hash_size);
            }
        }
        libspdm_hmac_update_with_request_finished_key (secured_message_context,
                                                       spdm_session_info->session_transcript.hmac_req_context_th, message,
                                                       message_size);

        spdm_session_info->session_transcript.message_f_initialized = true;
        return RETURN_SUCCESS;
    }
#endif
}

/**
 * This function returns if a given version is supported based upon the GET_VERSION/VERSION.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  version                      The SPDM version.
 *
 * @retval true  the version is supported.
 * @retval false the version is not supported.
 **/
bool libspdm_is_version_supported(const libspdm_context_t *spdm_context,
                                  uint8_t version)
{
    if (version ==
        (spdm_context->connection_info.version >> SPDM_VERSION_NUMBER_SHIFT_BIT)) {
        return true;
    }

    return false;
}

/**
 * This function returns connection version negotiated by GET_VERSION/VERSION.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 *
 * @return the connection version.
 **/
uint8_t libspdm_get_connection_version(const libspdm_context_t *spdm_context)
{
    return (uint8_t)(spdm_context->connection_info.version >> SPDM_VERSION_NUMBER_SHIFT_BIT);
}

/**
 * This function returns if a capablities flag is supported in current SPDM connection.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  is_requester                  Is the function called from a requester.
 * @param  requester_capabilities_flag    The requester capabilities flag to be checked
 * @param  responder_capabilities_flag    The responder capabilities flag to be checked
 *
 * @retval true  the capablities flag is supported.
 * @retval false the capablities flag is not supported.
 **/
bool
libspdm_is_capabilities_flag_supported(const libspdm_context_t *spdm_context,
                                       bool is_requester,
                                       uint32_t requester_capabilities_flag,
                                       uint32_t responder_capabilities_flag)
{
    uint32_t negotiated_requester_capabilities_flag;
    uint32_t negotiated_responder_capabilities_flag;

    if (is_requester) {
        negotiated_requester_capabilities_flag =
            spdm_context->local_context.capability.flags;
        negotiated_responder_capabilities_flag =
            spdm_context->connection_info.capability.flags;
    } else {
        negotiated_requester_capabilities_flag =
            spdm_context->connection_info.capability.flags;
        negotiated_responder_capabilities_flag =
            spdm_context->local_context.capability.flags;
    }

    if (((requester_capabilities_flag == 0) ||
         ((negotiated_requester_capabilities_flag &
           requester_capabilities_flag) != 0)) &&
        ((responder_capabilities_flag == 0) ||
         ((negotiated_responder_capabilities_flag &
           responder_capabilities_flag) != 0))) {
        return true;
    } else {
        return false;
    }
}

/**
 * Register SPDM device input/output functions.
 *
 * This function must be called after libspdm_init_context, and before any SPDM communication.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  send_message                  The fuction to send an SPDM transport layer message.
 * @param  receive_message               The fuction to receive an SPDM transport layer message.
 **/
void libspdm_register_device_io_func(
    void *context, libspdm_device_send_message_func send_message,
    libspdm_device_receive_message_func receive_message)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    spdm_context->send_message = send_message;
    spdm_context->receive_message = receive_message;
    return;
}

/**
 * Register SPDM transport layer encode/decode functions for SPDM or APP messages.
 *
 * This function must be called after libspdm_init_context, and before any SPDM communication.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  transport_encode_message       The fuction to encode an SPDM or APP message to a transport layer message.
 * @param  transport_decode_message       The fuction to decode an SPDM or APP message from a transport layer message.
 **/
void libspdm_register_transport_layer_func(
    void *context,
    libspdm_transport_encode_message_func transport_encode_message,
    libspdm_transport_decode_message_func transport_decode_message)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    spdm_context->transport_encode_message = transport_encode_message;
    spdm_context->transport_decode_message = transport_decode_message;
    return;
}

/**
 * Get the last error of an SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 *
 * @return Last error of an SPDM context.
 */
uint32_t libspdm_get_last_error(void *context)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    return spdm_context->error_state;
}

/**
 * Get the last SPDM error struct of an SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  last_spdm_error                Last SPDM error struct of an SPDM context.
 */
void libspdm_get_last_spdm_error_struct(void *context,
                                        libspdm_error_struct_t *last_spdm_error)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    libspdm_copy_mem(last_spdm_error, sizeof(libspdm_error_struct_t),
                     &spdm_context->last_spdm_error,sizeof(libspdm_error_struct_t));
}

/**
 * Set the last SPDM error struct of an SPDM context.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 * @param  last_spdm_error                Last SPDM error struct of an SPDM context.
 */
void libspdm_set_last_spdm_error_struct(void *context,
                                        libspdm_error_struct_t *last_spdm_error)
{
    libspdm_context_t *spdm_context;

    spdm_context = context;
    libspdm_copy_mem(&spdm_context->last_spdm_error, sizeof(spdm_context->last_spdm_error),
                     last_spdm_error, sizeof(libspdm_error_struct_t));
}

/**
 * Initialize an SPDM context.
 *
 * The size in bytes of the spdm_context can be returned by libspdm_get_context_size.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 *
 * @retval RETURN_SUCCESS       context is initialized.
 * @retval RETURN_DEVICE_ERROR  context initialization failed.
 */
return_status libspdm_init_context(void *context)
{
    libspdm_context_t *spdm_context;
    void *secured_message_context;
    uintn SecuredMessageContextSize;
    uintn index;

    spdm_context = context;
    libspdm_zero_mem(spdm_context, sizeof(libspdm_context_t));
    spdm_context->version = libspdm_context_struct_version;
    spdm_context->transcript.message_a.max_buffer_size =
        sizeof(spdm_context->transcript.message_a.buffer);
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    spdm_context->transcript.message_b.max_buffer_size =
        sizeof(spdm_context->transcript.message_b.buffer);
    spdm_context->transcript.message_c.max_buffer_size =
        sizeof(spdm_context->transcript.message_c.buffer);
    spdm_context->transcript.message_mut_b.max_buffer_size =
        sizeof(spdm_context->transcript.message_mut_b.buffer);
    spdm_context->transcript.message_mut_c.max_buffer_size =
        sizeof(spdm_context->transcript.message_mut_c.buffer);
    spdm_context->transcript.message_m.max_buffer_size =
        sizeof(spdm_context->transcript.message_m.buffer);
#endif
    spdm_context->retry_times = LIBSPDM_MAX_REQUEST_RETRY_TIMES;
    spdm_context->response_state = LIBSPDM_RESPONSE_STATE_NORMAL;
    spdm_context->current_token = 0;
    spdm_context->local_context.version.spdm_version_count = 3;
    spdm_context->local_context.version.spdm_version[0] = SPDM_MESSAGE_VERSION_10 <<
                                                          SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.version.spdm_version[1] = SPDM_MESSAGE_VERSION_11 <<
                                                          SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.version.spdm_version[2] = SPDM_MESSAGE_VERSION_12 <<
                                                          SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.secured_message_version.spdm_version_count =
        1;
    spdm_context->local_context.secured_message_version.spdm_version[0] =
        SPDM_MESSAGE_VERSION_10 << SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.capability.st1 = SPDM_ST1_VALUE_US;

    spdm_context->encap_context.certificate_chain_buffer.max_buffer_size =
        sizeof(spdm_context->encap_context.certificate_chain_buffer.buffer);

    /* From the config.h, need different value for CHUNK - TBD*/
    spdm_context->local_context.capability.data_transfer_size = LIBSPDM_MAX_MESSAGE_BUFFER_SIZE;
    spdm_context->local_context.capability.max_spdm_msg_size = LIBSPDM_MAX_MESSAGE_BUFFER_SIZE;

#if !LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    spdm_context->connection_info.peer_used_cert_chain_buffer_hash_size = 0;
    spdm_context->connection_info.peer_used_leaf_cert_public_key = NULL;
#endif

    secured_message_context = (void *)((uintn)(spdm_context + 1));
    SecuredMessageContextSize = libspdm_secured_message_get_context_size();
    for (index = 0; index < LIBSPDM_MAX_SESSION_COUNT; index++) {
        spdm_context->session_info[index].secured_message_context =
            (void *)((uintn)secured_message_context +
                     SecuredMessageContextSize * index);
        libspdm_secured_message_init_context(
            spdm_context->session_info[index]
            .secured_message_context);
    }


    /* The libspdm_random_seed function may or may not be implemented.
     * If unimplemented, the stub should always return success.*/

    if (!libspdm_random_seed(NULL, 0)) {
        return RETURN_DEVICE_ERROR;
    }

    return RETURN_SUCCESS;
}

/**
 * Reset an SPDM context.
 *
 * The size in bytes of the spdm_context can be returned by libspdm_get_context_size.
 *
 * @param  spdm_context                  A pointer to the SPDM context.
 */
void libspdm_reset_context(void *context)
{
    libspdm_context_t *spdm_context;
    uintn index;

    spdm_context = context;
    /*Clear all info about last connection*/
    libspdm_zero_mem(&spdm_context->connection_info.version, sizeof(spdm_version_number_t));
    libspdm_zero_mem(&spdm_context->connection_info.capability,
                     sizeof(libspdm_device_capability_t));
    libspdm_zero_mem(&spdm_context->connection_info.algorithm, sizeof(libspdm_device_algorithm_t));
    libspdm_zero_mem(&spdm_context->last_spdm_error, sizeof(libspdm_error_struct_t));
    libspdm_zero_mem(&spdm_context->encap_context, sizeof(libspdm_encap_context_t));
    spdm_context->connection_info.local_used_cert_chain_buffer_size = 0;
    spdm_context->connection_info.local_used_cert_chain_buffer = NULL;
    spdm_context->cache_spdm_request_size = 0;
    spdm_context->retry_times = LIBSPDM_MAX_REQUEST_RETRY_TIMES;
    spdm_context->response_state = LIBSPDM_RESPONSE_STATE_NORMAL;
    spdm_context->current_token = 0;
    spdm_context->last_spdm_request_session_id = INVALID_SESSION_ID;
    spdm_context->last_spdm_request_session_id_valid = false;
    spdm_context->last_spdm_request_size = 0;
    spdm_context->encap_context.certificate_chain_buffer.max_buffer_size =
        sizeof(spdm_context->encap_context.certificate_chain_buffer.buffer);
    for (index = 0; index < LIBSPDM_MAX_SESSION_COUNT; index++)
    {
        libspdm_session_info_init(spdm_context,
                                  &spdm_context->session_info[index],
                                  INVALID_SESSION_ID,
                                  false);
    }
}
/**
 * Return the size in bytes of the SPDM context.
 *
 * @return the size in bytes of the SPDM context.
 **/
uintn libspdm_get_context_size(void)
{
    return sizeof(libspdm_context_t) +
           libspdm_secured_message_get_context_size() * LIBSPDM_MAX_SESSION_COUNT;
}

/**
 * Return the SPDMversion field of the version number struct.
 *
 * @param  ver                Spdm version number struct.
 *
 * @return the SPDMversion of the version number struct.
 **/
uint8_t libspdm_get_version_from_version_number(const spdm_version_number_t ver)
{
    return (uint8_t)(ver >> SPDM_VERSION_NUMBER_SHIFT_BIT);
}

/**
 * Sort SPDMversion in descending order.
 *
 * @param  spdm_context                A pointer to the SPDM context.
 * @param  ver_set                    A pointer to the version set.
 * @param  ver_num                    Version number.
 */
void libspdm_version_number_sort(spdm_version_number_t *ver_set, uintn ver_num)
{
    uintn index;
    uintn index_sort;
    uintn index_max;
    spdm_version_number_t version;

    /* Select sort */
    if (ver_num > 1) {
        for (index_sort = 0; index_sort < ver_num; index_sort++) {
            index_max = index_sort;
            for (index = index_sort + 1; index < ver_num; index++) {
                /* if ver_ser[index] higher than ver_set[index_max] */
                if (ver_set[index] > ver_set[index_max]) {
                    index_max = index;
                }
            }
            /* swap ver_ser[index_min] and ver_set[index_sort] */
            version = ver_set[index_sort];
            ver_set[index_sort] = ver_set[index_max];
            ver_set[index_max] = version;
        }
    }
}

/**
 * Negotiate SPDMversion for connection.
 * ver_set is the local version set of requester, res_ver_set is the version set of responder.
 *
 * @param  common_version             A pointer to store the common version.
 * @param  req_ver_set                A pointer to the requester version set.
 * @param  req_ver_num                Version number of requester.
 * @param  res_ver_set                A pointer to the responder version set.
 * @param  res_ver_num                Version number of responder.
 *
 * @retval true                       Negotiation successfully, connect version be saved to common_version.
 * @retval false                      Negotiation failed.
 */
bool libspdm_negotiate_connection_version(spdm_version_number_t *common_version,
                                          spdm_version_number_t *req_ver_set,
                                          uintn req_ver_num,
                                          spdm_version_number_t *res_ver_set,
                                          uintn res_ver_num)
{
    uintn req_index;
    uintn res_index;

    if (req_ver_set == NULL || req_ver_num == 0 || res_ver_set == NULL || res_ver_num == 0) {
        return false;
    }

    /* Sort SPDMversion in descending order. */
    libspdm_version_number_sort(req_ver_set, req_ver_num);
    libspdm_version_number_sort(res_ver_set, res_ver_num);

    /**
     * Find highest same version and make req_index point to it.
     * If not found, return false.
     **/
    for (res_index = 0; res_index < res_ver_num; res_index++) {
        for (req_index = 0; req_index < req_ver_num; req_index++) {
            if (libspdm_get_version_from_version_number(req_ver_set[req_index]) ==
                libspdm_get_version_from_version_number(res_ver_set[res_index])) {
                *common_version = req_ver_set[req_index];
                return true;
            }
        }
    }
    return false;
}
