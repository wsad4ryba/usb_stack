#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>

#include "mtp_responder.h"
#include "mtp_container.h"
#include "mtp_storage.h"
#include "mtp_util.h"

#include "mock_mtp_storage_api.h"

static mtp_responder_t *mtp = NULL;
static size_t given_length;
static uint16_t error;
static uint8_t given_data[512];

const uint8_t operation_request[] = {
        0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x10,
        0xe2, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF
    };

Describe(set_object_info);

BeforeEach(set_object_info)
{
    mtp = mtp_responder_alloc();
    mtp_responder_init(mtp);
    mtp_responder_set_data_buffer(mtp, given_data, sizeof(given_data));
    mtp_responder_set_storage(mtp, 0x00010001, &mock_api, NULL);
    given_length = 0xaabbccdd;
    memset(given_data, 0xaa, sizeof(given_data));
    error = 0xaa;
}

AfterEach(set_object_info)
{
    mtp_responder_free(mtp);
}

Ensure(set_object_info, operation_does_not_return_data_and_response_code)
{
    const uint8_t request[] = {
        0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x10,
        0x0F, 0x00, 0x00, 0xF0, 0x01, 0x00, 0x01, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF
    };

    error = mtp_responder_handle_request(mtp, request, sizeof(request));
    given_length = mtp_responder_get_data(mtp);
    assert_that(error, is_equal_to(0));
    assert_that(given_length, is_equal_to(0));
}

Ensure(set_object_info, operation_fails_if_wrong_storage)
{
    const uint8_t request[] = {
        0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x10,
        0x0F, 0x00, 0x00, 0xF0, 0x01, 0x01, 0x00, 0x01,
        0xFF, 0xFF, 0xFF, 0xFF
    };
    error = mtp_responder_handle_request(mtp, request, sizeof(request));
    given_length = mtp_responder_get_data(mtp);
    assert_that(error, is_equal_to(MTP_RESPONSE_INVALID_STORAGE_ID));
    assert_that(given_length, is_equal_to(0));
}

Ensure(set_object_info, operation_does_not_return_status_if_ok)
{
    error = mtp_responder_handle_request(mtp, operation_request, sizeof(operation_request));
    given_length = mtp_responder_get_data(mtp);
    assert_that(error, is_equal_to(0));
    assert_that(given_length, is_equal_to(0));
}

Ensure(set_object_info, data_returns_error_if_dataset_is_wrong)
{
    const uint8_t request[] = {
        0x5c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x10,
        0xe2, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0c, 0x77, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x63,
        0x00, 0x6f, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x2e,
        0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };

    expect(deserialize_object_info,
            when(data, is_equal_to(&request[12])),
            when(length, is_equal_to(80)),
            will_return(-1));

    mtp_responder_handle_request(mtp, operation_request, sizeof(operation_request));
    error = mtp_responder_handle_request(mtp, request, sizeof(request));
    given_length = mtp_responder_get_data(mtp);
    assert_that(error, is_equal_to(MTP_RESPONSE_INVALID_DATASET));
    assert_that(given_length, is_equal_to(0));
}

Ensure(set_object_info, data_calls_storage_api_and_returns_error_if_format_not_supported)
{
    const uint8_t request[] = {
        0x5c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x10,
        0xe2, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0c, 0x77, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x63,
        0x00, 0x6f, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x2e,
        0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };

    expect(deserialize_object_info,
            when(data, is_equal_to(&request[12])),
            when(length, is_equal_to(80)),
            will_return(0));
    expect(is_format_code_supported,
            will_return(false));

    mtp_responder_handle_request(mtp, operation_request, sizeof(operation_request));
    error = mtp_responder_handle_request(mtp, request, sizeof(request));
    given_length = mtp_responder_get_data(mtp);
    assert_that(error, is_equal_to(MTP_RESPONSE_INVALID_OBJECT_FORMAT_CODE));
    assert_that(given_length, is_equal_to(0));

}

Ensure(set_object_info, data_calls_storage_api_and_returns_error_if_fail)
{
    const uint8_t request[] = {
        0x5c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x10,
        0xe2, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0c, 0x77, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x63,
        0x00, 0x6f, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x2e,
        0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };

    expect(deserialize_object_info,
            when(data, is_equal_to(&request[12])),
            when(length, is_equal_to(80)),
            will_return(0));
    expect(mock_create,
            will_return(-1));
    expect(is_format_code_supported,
            will_return(true));

    mtp_responder_handle_request(mtp, operation_request, sizeof(operation_request));
    error = mtp_responder_handle_request(mtp, request, sizeof(request));
    given_length = mtp_responder_get_data(mtp);
    assert_that(error, is_equal_to(MTP_RESPONSE_STORE_NOT_AVAILABLE));
    assert_that(given_length, is_equal_to(0));
}

Ensure(set_object_info, data_returns_ok_if_file_created)
{
    const uint8_t request[] = {
        0x5c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x10,
        0xe2, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0c, 0x77, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x63,
        0x00, 0x6f, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x2e,
        0x00, 0x74, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };

    expect(deserialize_object_info,
            when(data, is_equal_to(&request[12])),
            when(length, is_equal_to(80)),
            will_return(0));
    expect(mock_create,
            will_return(0));
    expect(is_format_code_supported,
            will_return(true));

    mtp_responder_handle_request(mtp, operation_request, sizeof(operation_request));
    error = mtp_responder_handle_request(mtp, request, sizeof(request));
    given_length = mtp_responder_get_data(mtp);
    assert_that(error, is_equal_to(MTP_RESPONSE_OK));
    assert_that(given_length, is_equal_to(0));
}