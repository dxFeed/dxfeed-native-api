// SPDX-License-Identifier: MPL-2.0

#include <dxFeedNativeCAPI/dxFeedNativeCAPI.h>
#include <dxFeedNativeCppAPI/dxFeedNativeCppAPI.hpp>

#include <cstring>
#include <utf8cpp/utf8.h>

namespace dxfcpp {

namespace detail {

const CEntryPointErrors CEntryPointErrors::NO_ERROR(0, "No error occurred.");
const CEntryPointErrors CEntryPointErrors::UNSPECIFIED(1, "An unspecified error occurred.");
const CEntryPointErrors CEntryPointErrors::NULL_ARGUMENT(2, "An argument was NULL (nullptr).");
const CEntryPointErrors CEntryPointErrors::UNATTACHED_THREAD(4, "The specified thread is not attached to the isolate.");
const CEntryPointErrors CEntryPointErrors::UNINITIALIZED_ISOLATE(5, "The specified isolate is unknown.");
const CEntryPointErrors CEntryPointErrors::LOCATE_IMAGE_FAILED(6, "Locating the image file failed.");
const CEntryPointErrors CEntryPointErrors::OPEN_IMAGE_FAILED(7, "Opening the located image file failed.");
const CEntryPointErrors CEntryPointErrors::MAP_HEAP_FAILED(8,
                                                           "Mapping the heap from the image file into memory failed.");
const CEntryPointErrors
    CEntryPointErrors::RESERVE_ADDRESS_SPACE_FAILED(801, "Reserving address space for the new isolate failed.");
const CEntryPointErrors
    CEntryPointErrors::INSUFFICIENT_ADDRESS_SPACE(802, "The image heap does not fit in the available address space.");
const CEntryPointErrors CEntryPointErrors::PROTECT_HEAP_FAILED(9, "Setting the protection of the heap memory failed.");
const CEntryPointErrors CEntryPointErrors::UNSUPPORTED_ISOLATE_PARAMETERS_VERSION(
    10, "The version of the specified isolate parameters is unsupported.");
const CEntryPointErrors
    CEntryPointErrors::THREADING_INITIALIZATION_FAILED(11, "Initialization of threading in the isolate failed.");
const CEntryPointErrors CEntryPointErrors::UNCAUGHT_EXCEPTION(12, "Some exception is not caught.");
const CEntryPointErrors CEntryPointErrors::ISOLATE_INITIALIZATION_FAILED(13, "Initialization the isolate failed.");
const CEntryPointErrors CEntryPointErrors::OPEN_AUX_IMAGE_FAILED(14,
                                                                 "Opening the located auxiliary image file failed.");
const CEntryPointErrors
    CEntryPointErrors::READ_AUX_IMAGE_META_FAILED(15, "Reading the opened auxiliary image file failed.");
const CEntryPointErrors CEntryPointErrors::MAP_AUX_IMAGE_FAILED(16,
                                                                "Mapping the auxiliary image file into memory failed.");
const CEntryPointErrors
    CEntryPointErrors::INSUFFICIENT_AUX_IMAGE_MEMORY(17, "Insufficient memory for the auxiliary image.");
const CEntryPointErrors
    CEntryPointErrors::AUX_IMAGE_UNSUPPORTED(18, "Auxiliary images are not supported on this platform or edition.");
const CEntryPointErrors CEntryPointErrors::FREE_ADDRESS_SPACE_FAILED(19,
                                                                     "Releasing the isolate's address space failed.");
const CEntryPointErrors CEntryPointErrors::FREE_IMAGE_HEAP_FAILED(20,
                                                                  "Releasing the isolate's image heap memory failed.");
const CEntryPointErrors CEntryPointErrors::AUX_IMAGE_PRIMARY_IMAGE_MISMATCH(
    21, "The auxiliary image was built from a different primary image.");
const CEntryPointErrors CEntryPointErrors::ARGUMENT_PARSING_FAILED(22, "The isolate arguments could not be parsed.");
const CEntryPointErrors CEntryPointErrors::CPU_FEATURE_CHECK_FAILED(
    23, "Current target does not support the following CPU features that are required by the image.");

const std::unordered_map<std::int32_t, CEntryPointErrors> CEntryPointErrors::ALL{
    {NO_ERROR.getCode(), NO_ERROR},
    {UNSPECIFIED.getCode(), UNSPECIFIED},
    {NULL_ARGUMENT.getCode(), NULL_ARGUMENT},
    {UNATTACHED_THREAD.getCode(), UNATTACHED_THREAD},
    {UNINITIALIZED_ISOLATE.getCode(), UNINITIALIZED_ISOLATE},
    {LOCATE_IMAGE_FAILED.getCode(), LOCATE_IMAGE_FAILED},
    {OPEN_IMAGE_FAILED.getCode(), OPEN_IMAGE_FAILED},
    {MAP_HEAP_FAILED.getCode(), MAP_HEAP_FAILED},
    {RESERVE_ADDRESS_SPACE_FAILED.getCode(), RESERVE_ADDRESS_SPACE_FAILED},
    {INSUFFICIENT_ADDRESS_SPACE.getCode(), INSUFFICIENT_ADDRESS_SPACE},
    {PROTECT_HEAP_FAILED.getCode(), PROTECT_HEAP_FAILED},
    {UNSUPPORTED_ISOLATE_PARAMETERS_VERSION.getCode(), UNSUPPORTED_ISOLATE_PARAMETERS_VERSION},
    {THREADING_INITIALIZATION_FAILED.getCode(), THREADING_INITIALIZATION_FAILED},
    {UNCAUGHT_EXCEPTION.getCode(), UNCAUGHT_EXCEPTION},
    {ISOLATE_INITIALIZATION_FAILED.getCode(), ISOLATE_INITIALIZATION_FAILED},
    {OPEN_AUX_IMAGE_FAILED.getCode(), OPEN_AUX_IMAGE_FAILED},
    {READ_AUX_IMAGE_META_FAILED.getCode(), READ_AUX_IMAGE_META_FAILED},
    {MAP_AUX_IMAGE_FAILED.getCode(), MAP_AUX_IMAGE_FAILED},
    {INSUFFICIENT_AUX_IMAGE_MEMORY.getCode(), INSUFFICIENT_AUX_IMAGE_MEMORY},
    {AUX_IMAGE_UNSUPPORTED.getCode(), AUX_IMAGE_UNSUPPORTED},
    {FREE_ADDRESS_SPACE_FAILED.getCode(), FREE_ADDRESS_SPACE_FAILED},
    {FREE_IMAGE_HEAP_FAILED.getCode(), FREE_IMAGE_HEAP_FAILED},
    {AUX_IMAGE_PRIMARY_IMAGE_MISMATCH.getCode(), AUX_IMAGE_PRIMARY_IMAGE_MISMATCH},
    {ARGUMENT_PARSING_FAILED.getCode(), ARGUMENT_PARSING_FAILED},
    {CPU_FEATURE_CHECK_FAILED.getCode(), CPU_FEATURE_CHECK_FAILED},
};

thread_local Isolate::IsolateThread Isolate::currentIsolateThread_{};

const auto I = Isolate::getInstance();
} // namespace detail

const std::string DXEndpoint::NAME_PROPERTY = "name";
const std::string DXEndpoint::DXFEED_PROPERTIES_PROPERTY = "dxfeed.properties";
const std::string DXEndpoint::DXFEED_ADDRESS_PROPERTY = "dxfeed.address";
const std::string DXEndpoint::DXFEED_USER_PROPERTY = "dxfeed.user";
const std::string DXEndpoint::DXFEED_PASSWORD_PROPERTY = "dxfeed.password";
const std::string DXEndpoint::DXFEED_THREAD_POOL_SIZE_PROPERTY = "dxfeed.threadPoolSize";
const std::string DXEndpoint::DXFEED_AGGREGATION_PERIOD_PROPERTY = "dxfeed.aggregationPeriod";
const std::string DXEndpoint::DXFEED_WILDCARD_ENABLE_PROPERTY = "dxfeed.wildcard.enable";
const std::string DXEndpoint::DXPUBLISHER_PROPERTIES_PROPERTY = "dxpublisher.properties";
const std::string DXEndpoint::DXPUBLISHER_ADDRESS_PROPERTY = "dxpublisher.address";
const std::string DXEndpoint::DXPUBLISHER_THREAD_POOL_SIZE_PROPERTY = "dxpublisher.threadPoolSize";
const std::string DXEndpoint::DXENDPOINT_EVENT_TIME_PROPERTY = "dxendpoint.eventTime";
const std::string DXEndpoint::DXENDPOINT_STORE_EVERYTHING_PROPERTY = "dxendpoint.storeEverything";
const std::string DXEndpoint::DXSCHEME_NANO_TIME_PROPERTY = "dxscheme.nanoTime";
const std::string DXEndpoint::DXSCHEME_ENABLED_PROPERTY_PREFIX = "dxscheme.enabled.";

} // namespace dxfcpp

dxfc_error_code_t dxfc_system_set_property(const char *key, const char *value) {
    return dxfcpp::System::setProperty(key, value) ? DXFC_EC_SUCCESS : DXFC_EC_ERROR;
}

dxfc_error_code_t dxfc_system_get_property(const char *key, char *buffer, size_t bufferSize) {
    auto value = dxfcpp::System::getProperty(key);

    if (buffer != nullptr && bufferSize > 0ULL) {
        auto size = std::min(value.size(), bufferSize - 1);

        buffer[size] = '\0';

        if (size == 0ULL) {
            return DXFC_EC_SUCCESS;
        }

        memcpy(buffer, value.c_str(), size);

        auto begin = buffer;
        auto end = buffer + size;

        // Looks for the first invalid UTF-8 character.
        auto found = utf8::find_invalid(begin, end);

        // An invalid character position was found. Trim the result string buffer to this invalid character.
        if (found != end) {
            *found = '\0';
        }
    }

    return DXFC_EC_SUCCESS;
}