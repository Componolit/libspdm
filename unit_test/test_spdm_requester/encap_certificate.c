/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link:
 * https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "spdm_unit_test.h"
#include "internal/libspdm_requester_lib.h"

#if LIBSPDM_ENABLE_CAPABILITY_CERT_CAP

/* #define TEST_DEBUG*/
#ifdef TEST_DEBUG
#define TEST_DEBUG_PRINT(format, ...) printf(format, ## __VA_ARGS__)
#else
#define TEST_DEBUG_PRINT(...)
#endif

spdm_get_certificate_request_t m_spdm_get_certificate_request1 = {
    {SPDM_MESSAGE_VERSION_10, SPDM_GET_CERTIFICATE, 0, 0},
    0,
    LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN
};
uintn m_spdm_get_certificate_request1_size =
    sizeof(m_spdm_get_certificate_request1);

spdm_get_certificate_request_t m_spdm_get_certificate_request2 = {
    {SPDM_MESSAGE_VERSION_10, SPDM_GET_CERTIFICATE, 0, 0},
    0,
    LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN
};
uintn m_spdm_get_certificate_request2_size = LIBSPDM_MAX_MESSAGE_BUFFER_SIZE;

spdm_get_certificate_request_t m_spdm_get_certificate_request3 = {
    {SPDM_MESSAGE_VERSION_10, SPDM_GET_CERTIFICATE, 0, 0}, 0, 0
};
uintn m_spdm_get_certificate_request3_size =
    sizeof(m_spdm_get_certificate_request3);

/**
 * Test 1: request the first LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN bytes of the
 * certificate chain Expected Behavior: generate a correctly formed Certficate
 * message, including its portion_length and remainder_length fields
 **/
void libspdm_test_requester_encap_certificate_case1(void **state)
{
    return_status status;
    libspdm_test_context_t *spdm_test_context;
    libspdm_context_t *spdm_context;
    uintn response_size;
    uint8_t response[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    spdm_certificate_response_t *spdm_response;
    void *data;
    uintn data_size;

    spdm_test_context = *state;
    spdm_context = spdm_test_context->spdm_context;
    spdm_test_context->case_id = 0x1;
    spdm_context->connection_info.version = SPDM_MESSAGE_VERSION_10
                                            << SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->connection_info.connection_state =
        LIBSPDM_CONNECTION_STATE_AFTER_DIGESTS;
    spdm_context->local_context.capability.flags |=
        SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
    spdm_context->connection_info.algorithm.base_hash_algo = m_libspdm_use_hash_algo;
    libspdm_read_responder_public_certificate_chain(m_libspdm_use_hash_algo,
                                                    m_libspdm_use_asym_algo,
                                                    &data, &data_size, NULL, NULL);
    spdm_context->local_context.local_cert_chain_provision[0] = data;
    spdm_context->local_context.local_cert_chain_provision_size[0] = data_size;
    spdm_context->local_context.slot_count = 1;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    spdm_context->transcript.message_m.buffer_size =
        spdm_context->transcript.message_m.max_buffer_size;
#endif

    response_size = sizeof(response);
    status = libspdm_get_encap_response_certificate(
        spdm_context, m_spdm_get_certificate_request1_size,
        &m_spdm_get_certificate_request1, &response_size, response);

    assert_int_equal(status, RETURN_SUCCESS);
    assert_int_equal(response_size, sizeof(spdm_certificate_response_t) +
                     LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN);
    spdm_response = (void *)response;
    assert_int_equal(spdm_response->header.request_response_code,
                     SPDM_CERTIFICATE);
    assert_int_equal(spdm_response->header.param1, 0);
    assert_int_equal(spdm_response->portion_length,
                     LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN);
    assert_int_equal(spdm_response->remainder_length,
                     data_size - LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN);
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    assert_int_equal(spdm_context->transcript.message_m.buffer_size, 0);
#endif
    free(data);
}

/**
 * Test 2: Wrong GET_CERTIFICATE message size (larger than expected)
 * Expected Behavior: generate an ERROR_RESPONSE with code
 * SPDM_ERROR_CODE_INVALID_REQUEST
 **/
void libspdm_test_requester_encap_certificate_case2(void **state)
{
    return_status status;
    libspdm_test_context_t *spdm_test_context;
    libspdm_context_t *spdm_context;
    uintn response_size;
    uint8_t response[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    spdm_certificate_response_t *spdm_response;
    void *data;
    uintn data_size;

    spdm_test_context = *state;
    spdm_context = spdm_test_context->spdm_context;
    spdm_test_context->case_id = 0x2;
    spdm_context->connection_info.version = SPDM_MESSAGE_VERSION_10
                                            << SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.capability.flags |=
        SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
    spdm_context->connection_info.algorithm.base_hash_algo = m_libspdm_use_hash_algo;
    libspdm_read_responder_public_certificate_chain(m_libspdm_use_hash_algo,
                                                    m_libspdm_use_asym_algo,
                                                    &data, &data_size, NULL, NULL);
    spdm_context->local_context.local_cert_chain_provision[0] = data;
    spdm_context->local_context.local_cert_chain_provision_size[0] = data_size;
    spdm_context->local_context.slot_count = 1;

    response_size = sizeof(response);
    status = libspdm_get_encap_response_certificate(
        spdm_context, m_spdm_get_certificate_request2_size,
        &m_spdm_get_certificate_request2, &response_size, response);
    assert_int_equal(status, RETURN_SUCCESS);
    assert_int_equal(response_size, sizeof(spdm_error_response_t));
    spdm_response = (void *)response;
    assert_int_equal(spdm_response->header.request_response_code, SPDM_ERROR);
    assert_int_equal(spdm_response->header.param1,
                     SPDM_ERROR_CODE_INVALID_REQUEST);
    assert_int_equal(spdm_response->header.param2, 0);
    free(data);
}

/**
 * Test 3: request length at the boundary of maximum integer values, while
 * keeping offset 0 Expected Behavior: generate correctly formed Certficate
 * messages, including its portion_length and remainder_length fields
 **/
void libspdm_test_requester_encap_certificate_case3(void **state)
{
    return_status status;
    libspdm_test_context_t *spdm_test_context;
    libspdm_context_t *spdm_context;
    uintn response_size;
    uint8_t response[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    spdm_certificate_response_t *spdm_response;
    void *data;
    uintn data_size;

    /* Testing Lengths at the boundary of maximum integer values*/
    uint16_t test_lenghts[] = {
        0,
        MAX_INT8,
        (uint16_t)(MAX_INT8 + 1),
        MAX_UINT8,
        MAX_INT16,
        (uint16_t)(MAX_INT16 + 1),
        MAX_UINT16,
    };
    uint16_t expected_chunk_size;

    /* Setting up the spdm_context and loading a sample certificate chain*/
    spdm_test_context = *state;
    spdm_context = spdm_test_context->spdm_context;
    spdm_test_context->case_id = 0x3;
    spdm_context->connection_info.version = SPDM_MESSAGE_VERSION_10
                                            << SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.capability.flags |=
        SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
    spdm_context->connection_info.algorithm.base_hash_algo = m_libspdm_use_hash_algo;
    libspdm_read_responder_public_certificate_chain(m_libspdm_use_hash_algo,
                                                    m_libspdm_use_asym_algo,
                                                    &data, &data_size, NULL, NULL);
    spdm_context->local_context.local_cert_chain_provision[0] = data;
    spdm_context->local_context.local_cert_chain_provision_size[0] = data_size;
    spdm_context->local_context.slot_count = 1;

    /* This tests considers only offset = 0, other tests vary offset value*/
    m_spdm_get_certificate_request3.offset = 0;

    for (int i = 0; i < sizeof(test_lenghts) / sizeof(test_lenghts[0]); i++)
    {
        TEST_DEBUG_PRINT("i:%d test_lenghts[i]:%u\n", i, test_lenghts[i]);
        m_spdm_get_certificate_request3.length = test_lenghts[i];
        /* Expected received length is limited by LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN
         * (implementation specific?)*/
        expected_chunk_size = MIN(m_spdm_get_certificate_request3.length,
                                  LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN);

        /* reseting an internal buffer to avoid overflow and prevent tests to
         * succeed*/
        libspdm_reset_message_b(spdm_context);
        response_size = sizeof(response);
        m_spdm_get_certificate_request3_size =
            sizeof(m_spdm_get_certificate_request3);
        status = libspdm_get_encap_response_certificate(
            spdm_context, m_spdm_get_certificate_request3_size,
            &m_spdm_get_certificate_request3, &response_size, response);
        assert_int_equal(status, RETURN_SUCCESS);
        assert_int_equal(response_size,
                         sizeof(spdm_certificate_response_t) + expected_chunk_size);
        spdm_response = (void *)response;
        assert_int_equal(spdm_response->header.request_response_code,
                         SPDM_CERTIFICATE);
        assert_int_equal(spdm_response->header.param1, 0);
        assert_int_equal(spdm_response->portion_length, expected_chunk_size);
        assert_int_equal(spdm_response->remainder_length,
                         data_size - expected_chunk_size);
    }
    free(data);
}

/**
 * Test 4: request offset at the boundary of maximum integer values, while
 * keeping length 0 Expected Behavior: generate correctly formed Certficate
 * messages, including its portion_length and remainder_length fields
 **/
void libspdm_test_requester_encap_certificate_case4(void **state)
{
    return_status status;
    libspdm_test_context_t *spdm_test_context;
    libspdm_context_t *spdm_context;
    uintn response_size;
    uint8_t response[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    spdm_certificate_response_t *spdm_response;
    spdm_error_response_t *spdm_responseError;
    void *data;
    uintn data_size;

    /* Testing offsets at the boundary of maximum integer values and at the
     * boundary of certificate length (first three positions)*/
    uint16_t test_offsets[] = {(uint16_t)(-1),
                               0,
                               +1,
                               0,
                               MAX_INT8,
                               (uint16_t)(MAX_INT8 + 1),
                               MAX_UINT8,
                               MAX_INT16,
                               (uint16_t)(MAX_INT16 + 1),
                               MAX_UINT16,
                               (uint16_t)(-1)};

    /* Setting up the spdm_context and loading a sample certificate chain*/
    spdm_test_context = *state;
    spdm_context = spdm_test_context->spdm_context;
    spdm_test_context->case_id = 0x4;
    spdm_context->connection_info.version = SPDM_MESSAGE_VERSION_10
                                            << SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.capability.flags |=
        SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
    spdm_context->connection_info.algorithm.base_hash_algo = m_libspdm_use_hash_algo;
    libspdm_read_responder_public_certificate_chain(m_libspdm_use_hash_algo,
                                                    m_libspdm_use_asym_algo,
                                                    &data, &data_size, NULL, NULL);
    spdm_context->local_context.local_cert_chain_provision[0] = data;
    spdm_context->local_context.local_cert_chain_provision_size[0] = data_size;
    spdm_context->local_context.slot_count = 1;

    /* This tests considers only length = 0, other tests vary length value*/
    m_spdm_get_certificate_request3.length = 0;
    /* Setting up offset values at the boundary of certificate length*/
    test_offsets[0] = (uint16_t)(test_offsets[0] + data_size);
    test_offsets[1] = (uint16_t)(test_offsets[1] + data_size);
    test_offsets[2] = (uint16_t)(test_offsets[2] + data_size);

    for (int i = 0; i < sizeof(test_offsets) / sizeof(test_offsets[0]); i++)
    {
        TEST_DEBUG_PRINT("i:%d test_offsets[i]:%u\n", i, test_offsets[i]);
        m_spdm_get_certificate_request3.offset = test_offsets[i];

        /* reseting an internal buffer to avoid overflow and prevent tests to
         * succeed*/
        libspdm_reset_message_b(spdm_context);
        response_size = sizeof(response);
        status = libspdm_get_encap_response_certificate(
            spdm_context, m_spdm_get_certificate_request3_size,
            &m_spdm_get_certificate_request3, &response_size, response);
        assert_int_equal(status, RETURN_SUCCESS);

        if (m_spdm_get_certificate_request3.offset >= data_size)
        {
            /* A too long of an offset should return an error*/
            spdm_responseError = (void *)response;
            assert_int_equal(spdm_responseError->header.request_response_code,
                             SPDM_ERROR);
            assert_int_equal(spdm_responseError->header.param1,
                             SPDM_ERROR_CODE_INVALID_REQUEST);
        }
        else
        {
            /* Otherwise it should work properly, considering length = 0*/
            assert_int_equal(response_size, sizeof(spdm_certificate_response_t));
            spdm_response = (void *)response;
            assert_int_equal(spdm_response->header.request_response_code,
                             SPDM_CERTIFICATE);
            assert_int_equal(spdm_response->header.param1, 0);
            assert_int_equal(spdm_response->portion_length, 0);
            assert_int_equal(
                spdm_response->remainder_length,
                (uint16_t)(data_size - m_spdm_get_certificate_request3.offset));
        }
    }
    free(data);
}

/**
 * Test 5: request LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN bytes of long certificate
 * chains, with the largest valid offset Expected Behavior: generate correctly
 * formed Certficate messages, including its portion_length and remainder_length
 * fields
 **/
void libspdm_test_requester_encap_certificate_case5(void **state)
{
    return_status status;
    libspdm_test_context_t *spdm_test_context;
    libspdm_context_t *spdm_context;
    uintn response_size;
    uint8_t response[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    spdm_certificate_response_t *spdm_response;
    spdm_error_response_t *spdm_responseError;
    void *data;
    uintn data_size;

    uint16_t test_cases[] = {LIBSPDM_TEST_CERT_MAXINT16, LIBSPDM_TEST_CERT_MAXUINT16};

    uintn expected_chunk_size;
    uintn expected_remainder;

    /* Setting up the spdm_context and loading a sample certificate chain*/
    spdm_test_context = *state;
    spdm_context = spdm_test_context->spdm_context;
    spdm_test_context->case_id = 0x5;
    spdm_context->connection_info.version = SPDM_MESSAGE_VERSION_10
                                            << SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.capability.flags |=
        SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
    spdm_context->connection_info.algorithm.base_hash_algo = m_libspdm_use_hash_algo;

    m_spdm_get_certificate_request3.length = LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN;

    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        libspdm_read_responder_public_certificate_chain_by_size(
            /*MAXUINT16_CERT signature_algo is SHA256RSA */
            m_libspdm_use_hash_algo, SPDM_ALGORITHMS_BASE_ASYM_ALGO_TPM_ALG_RSASSA_2048,
            test_cases[i], &data, &data_size, NULL, NULL);

        spdm_context->local_context.local_cert_chain_provision[0] = data;
        spdm_context->local_context.local_cert_chain_provision_size[0] = data_size;
        spdm_context->local_context.slot_count = 1;

        m_spdm_get_certificate_request3.offset =
            (uint16_t)(MIN(data_size - 1, MAX_UINT16));
        TEST_DEBUG_PRINT("data_size: %u\n", data_size);
        TEST_DEBUG_PRINT("m_spdm_get_certificate_request3.offset: %u\n",
                         m_spdm_get_certificate_request3.offset);
        TEST_DEBUG_PRINT("m_spdm_get_certificate_request3.length: %u\n",
                         m_spdm_get_certificate_request3.length);
        TEST_DEBUG_PRINT("offset + length: %u\n",
                         m_spdm_get_certificate_request3.offset +
                         m_spdm_get_certificate_request3.length);

        /* reseting an internal buffer to avoid overflow and prevent tests to
         * succeed*/
        libspdm_reset_message_b(spdm_context);
        response_size = sizeof(response);
        status = libspdm_get_encap_response_certificate(
            spdm_context, m_spdm_get_certificate_request3_size,
            &m_spdm_get_certificate_request3, &response_size, response);
        assert_int_equal(status, RETURN_SUCCESS);

        /* Expected received length is limited by LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN
         * and by the remaining length*/
        expected_chunk_size =
            (uint16_t)(MIN(m_spdm_get_certificate_request3.length,
                           data_size - m_spdm_get_certificate_request3.offset));
        expected_chunk_size =
            MIN(expected_chunk_size, LIBSPDM_MAX_CERT_CHAIN_BLOCK_LEN);
        /* Expected certificate length left*/
        expected_remainder =
            (uint16_t)(data_size - m_spdm_get_certificate_request3.offset -
                       expected_chunk_size);

        TEST_DEBUG_PRINT("expected_chunk_size %u\n", expected_chunk_size);
        TEST_DEBUG_PRINT("expected_remainder %u\n", expected_remainder);

        if (expected_remainder > MAX_UINT16 || expected_chunk_size > MAX_UINT16)
        {
            spdm_responseError = (void *)response;
            assert_int_equal(spdm_responseError->header.request_response_code,
                             SPDM_ERROR);
            assert_int_equal(spdm_responseError->header.param1,
                             SPDM_ERROR_CODE_INVALID_REQUEST);
        }
        else
        {
            assert_int_equal(response_size, sizeof(spdm_certificate_response_t) +
                             expected_chunk_size);
            spdm_response = (void *)response;
            assert_int_equal(spdm_response->header.request_response_code,
                             SPDM_CERTIFICATE);
            assert_int_equal(spdm_response->header.param1, 0);
            assert_int_equal(spdm_response->portion_length, expected_chunk_size);
            assert_int_equal(spdm_response->remainder_length, expected_remainder);
        }

        TEST_DEBUG_PRINT("\n");

        spdm_context->local_context.local_cert_chain_provision[0] = NULL;
        spdm_context->local_context.local_cert_chain_provision_size[0] = 0;
        free(data);
    }
}

/**
 * Test 6: request a whole certificate chain byte by byte
 * Expected Behavior: generate correctly formed Certficate messages, including
 * its portion_length and remainder_length fields
 **/
void libspdm_test_requester_encap_certificate_case6(void **state)
{
    return_status status;
    libspdm_test_context_t *spdm_test_context;
    libspdm_context_t *spdm_context;
    uintn response_size;
    uint8_t response[LIBSPDM_MAX_MESSAGE_BUFFER_SIZE];
    spdm_certificate_response_t *spdm_response;
    void *data;
    uintn data_size;
    uint16_t expected_chunk_size;
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
    uintn count;
#endif
    /* Setting up the spdm_context and loading a sample certificate chain*/
    spdm_test_context = *state;
    spdm_context = spdm_test_context->spdm_context;
    spdm_test_context->case_id = 0x6;
    spdm_context->connection_info.version = SPDM_MESSAGE_VERSION_10
                                            << SPDM_VERSION_NUMBER_SHIFT_BIT;
    spdm_context->local_context.capability.flags |=
        SPDM_GET_CAPABILITIES_RESPONSE_FLAGS_CERT_CAP;
    spdm_context->connection_info.algorithm.base_hash_algo = m_libspdm_use_hash_algo;
    libspdm_read_responder_public_certificate_chain(m_libspdm_use_hash_algo,
                                                    m_libspdm_use_asym_algo,
                                                    &data, &data_size, NULL, NULL);
    spdm_context->local_context.local_cert_chain_provision[0] = data;
    spdm_context->local_context.local_cert_chain_provision_size[0] = data_size;
    spdm_context->local_context.slot_count = 1;

    /* This tests considers only length = 1*/
    m_spdm_get_certificate_request3.length = 1;
    expected_chunk_size = 1;

    /* reseting an internal buffer to avoid overflow and prevent tests to
     * succeed*/
    libspdm_reset_message_b(spdm_context);

    spdm_response = NULL;
    for (uintn offset = 0; offset < data_size; offset++)
    {
        TEST_DEBUG_PRINT("offset:%u \n", offset);
        m_spdm_get_certificate_request3.offset = (uint16_t)offset;

        response_size = sizeof(response);
        status = libspdm_get_encap_response_certificate(
            spdm_context, m_spdm_get_certificate_request3_size,
            &m_spdm_get_certificate_request3, &response_size, response);
        assert_int_equal(status, RETURN_SUCCESS);
        spdm_response = (void *)response;
        /* It may fail because the spdm does not support too many messages.
         * assert_int_equal (spdm_response->header.request_response_code,
         * SPDM_CERTIFICATE);*/
        if (spdm_response->header.request_response_code == SPDM_CERTIFICATE)
        {
            assert_int_equal(spdm_response->header.request_response_code,
                             SPDM_CERTIFICATE);
            assert_int_equal(response_size, sizeof(spdm_certificate_response_t) +
                             expected_chunk_size);
            assert_int_equal(spdm_response->header.param1, 0);
            assert_int_equal(spdm_response->portion_length, expected_chunk_size);
            assert_int_equal(spdm_response->remainder_length,
                             data_size - offset - expected_chunk_size);
            assert_int_equal(((uint8_t *)data)[offset],
                             (response + sizeof(spdm_certificate_response_t))[0]);
        }
        else
        {
            assert_int_equal(spdm_response->header.request_response_code, SPDM_ERROR);
            break;
        }
    }
    if (spdm_response != NULL)
    {
        if (spdm_response->header.request_response_code == SPDM_CERTIFICATE)
        {
#if LIBSPDM_RECORD_TRANSCRIPT_DATA_SUPPORT
            count = (data_size + m_spdm_get_certificate_request3.length - 1) /
                    m_spdm_get_certificate_request3.length;
            assert_int_equal(spdm_context->transcript.message_b.buffer_size,
                             sizeof(spdm_get_certificate_request_t) * count +
                             sizeof(spdm_certificate_response_t) * count +
                             data_size);
#endif
        }
    }
    free(data);
}

libspdm_test_context_t m_libspdm_requester_encap_certificate_test_context = {
    LIBSPDM_TEST_CONTEXT_SIGNATURE,
    false,
};

int libspdm_requester_encap_certificate_test_main(void)
{
    const struct CMUnitTest spdm_requester_encap_certificate_tests[] = {
        /* Success Case*/
        cmocka_unit_test(libspdm_test_requester_encap_certificate_case1),
        /* Bad request size*/
        cmocka_unit_test(libspdm_test_requester_encap_certificate_case2),
        /* Tests varying offset*/
        cmocka_unit_test(libspdm_test_requester_encap_certificate_case4),
        /* Tests large certificate chains*/
        cmocka_unit_test(libspdm_test_requester_encap_certificate_case5),
        /* Requests byte by byte*/
        cmocka_unit_test(libspdm_test_requester_encap_certificate_case6),
    };

    libspdm_setup_test_context(&m_libspdm_requester_encap_certificate_test_context);

    return cmocka_run_group_tests(spdm_requester_encap_certificate_tests,
                                  libspdm_unit_test_group_setup,
                                  libspdm_unit_test_group_teardown);
}

#endif /* LIBSPDM_ENABLE_CAPABILITY_CERT_CAP*/
