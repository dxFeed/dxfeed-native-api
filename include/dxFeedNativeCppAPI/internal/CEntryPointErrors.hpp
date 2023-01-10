// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <compare>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "Common.hpp"

namespace dxfcpp::detail {

/**
 * Possible error codes returned by internal GraalVM functions
 *
 *  [Graal:CEntryPointErrors](https://github.com/oracle/graal/blob/96a1a66347bd4e5e00ae4e8e79812ebaf8cd5e33/substratevm/src/com.oracle.svm.core/src/com/oracle/svm/core/c/function/CEntryPointErrors.java#L43)
 */
class CEntryPointErrors {
    std::int32_t code_{};
    std::string description_{};

    CEntryPointErrors(std::int32_t code, std::string description) noexcept
        : code_{code}, description_{std::move(description)} {}

  public:
    /// 0. No error occurred.
    static const CEntryPointErrors NO_ERROR;

    /// 1. An unspecified error occurred.
    static const CEntryPointErrors UNSPECIFIED;

    /// 2. An argument was NULL (nullptr).
    static const CEntryPointErrors NULL_ARGUMENT;

    /// 4. The specified thread is not attached to the isolate.
    static const CEntryPointErrors UNATTACHED_THREAD;

    /// 5. The specified isolate is unknown.
    static const CEntryPointErrors UNINITIALIZED_ISOLATE;

    /// 6. Locating the image file failed.
    static const CEntryPointErrors LOCATE_IMAGE_FAILED;

    /// 7. Opening the located image file failed.
    static const CEntryPointErrors OPEN_IMAGE_FAILED;

    /// 8. Mapping the heap from the image file into memory failed.
    static const CEntryPointErrors MAP_HEAP_FAILED;

    /// 801. Reserving address space for the new isolate failed.
    static const CEntryPointErrors RESERVE_ADDRESS_SPACE_FAILED;

    /// 802. The image heap does not fit in the available address space.
    static const CEntryPointErrors INSUFFICIENT_ADDRESS_SPACE;

    /// 9. Setting the protection of the heap memory failed.
    static const CEntryPointErrors PROTECT_HEAP_FAILED;

    /// 10. The version of the specified isolate parameters is unsupported.
    static const CEntryPointErrors UNSUPPORTED_ISOLATE_PARAMETERS_VERSION;

    /// 11. Initialization of threading in the isolate failed.
    static const CEntryPointErrors THREADING_INITIALIZATION_FAILED;

    /// 12. Some exception is not caught.
    static const CEntryPointErrors UNCAUGHT_EXCEPTION;

    /// 13. Initialization the isolate failed.
    static const CEntryPointErrors ISOLATE_INITIALIZATION_FAILED;

    /// 14. Opening the located auxiliary image file failed.
    static const CEntryPointErrors OPEN_AUX_IMAGE_FAILED;

    /// 15. Reading the opened auxiliary image file failed.
    static const CEntryPointErrors READ_AUX_IMAGE_META_FAILED;

    /// 16. Mapping the auxiliary image file into memory failed.
    static const CEntryPointErrors MAP_AUX_IMAGE_FAILED;

    /// 17. Insufficient memory for the auxiliary image.
    static const CEntryPointErrors INSUFFICIENT_AUX_IMAGE_MEMORY;

    /// 18. Auxiliary images are not supported on this platform or edition.
    static const CEntryPointErrors AUX_IMAGE_UNSUPPORTED;

    /// 19. Releasing the isolate's address space failed.
    static const CEntryPointErrors FREE_ADDRESS_SPACE_FAILED;

    /// 20. Releasing the isolate's image heap memory failed.
    static const CEntryPointErrors FREE_IMAGE_HEAP_FAILED;

    /// 21. The auxiliary image was built from a different primary image.
    static const CEntryPointErrors AUX_IMAGE_PRIMARY_IMAGE_MISMATCH;

    /// 22. The isolate arguments could not be parsed.
    static const CEntryPointErrors ARGUMENT_PARSING_FAILED;

    /// 23. Current target does not support the following CPU features that are required by the image.
    static const CEntryPointErrors CPU_FEATURE_CHECK_FAILED;

    static const std::unordered_map<std::int32_t, CEntryPointErrors> ALL;

    static CEntryPointErrors valueOf(std::int32_t code) {
        if (auto found = ALL.find(code); found != ALL.end()) {
            return found->second;
        }

        return UNSPECIFIED;
    }

    /// Returns the code
    [[nodiscard]] std::int32_t getCode() const { return code_; }

    /// Returns the description
    [[nodiscard]] const std::string &getDescription() const { return description_; }

    bool operator==(const CEntryPointErrors &error1) const { return this->getCode() == error1.getCode(); }
};
} // namespace dxfcpp::detail