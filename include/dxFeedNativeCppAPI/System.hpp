// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <dxfg_system.h>

#include "internal/CEntryPointErrors.hpp"
#include "internal/Common.hpp"
#include "internal/Isolate.hpp"

#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include <fmt/format.h>
#include <fmt/std.h>

namespace dxfcpp {
/**
 * A class that allows to set JVM system properties and get the values of JVM system properties.
 */
struct System {
    /**
     * Sets the JVM system property indicated by the specified key.
     *
     * @param key The name of the system property (UTF-8 string).
     * @param value The value of the system property (UTF-8 string).
     * @return true if the setting of the JVM system property succeeded.
     */
    static inline bool setProperty(const std::string &key, const std::string &value) {
        if constexpr (dxfcpp::detail::isDebug) {
            std::clog << fmt::format("System::setProperty(key = '{}', value = '{}')\n", key, value);
        }

        auto result = std::visit(
            [](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, detail::CEntryPointErrors>) {
                    return false;
                } else {
                    return arg;
                }
            },
            detail::Isolate::getInstance()->runIsolated(
                [key = key, value = value](detail::GraalIsolateThreadHandle threadHandle) {
                    return detail::CEntryPointErrors::valueOf(dxfg_system_set_property(
                               threadHandle, key.c_str(), value.c_str())) == detail::CEntryPointErrors::NO_ERROR;
                }));

        if constexpr (dxfcpp::detail::isDebug) {
            std::clog << fmt::format("System::setProperty(key = '{}', value = '{}') -> {}\n", key, value, result);
        }

        return result;
    }

    /**
     * Gets the system property indicated by the specified key.
     *
     * @param key The name of the system property (UTF-8 string).
     * @return The value of a JVM system property (UTF-8 string), or an empty string.
     */
    static inline std::string getProperty(const std::string &key) {
        if constexpr (detail::isDebug) {
            std::clog << fmt::format("System::getProperty(key = {})\n", key);
        }

        auto result = std::visit(
            [](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, detail::CEntryPointErrors>) {
                    return std::string{};
                } else {
                    return arg;
                }
            },
            detail::Isolate::getInstance()->runIsolated([key = key](detail::GraalIsolateThreadHandle threadHandle) {
                std::string resultString{};

                if (auto result = dxfg_system_get_property(threadHandle, key.c_str()); result != nullptr) {
                    resultString = result;
                    dxfg_system_release_property(threadHandle, result);
                }

                return resultString;
            }));

        if constexpr (detail::isDebug) {
            std::clog << fmt::format("System::getProperty(key = '{}') -> '{}'\n", key, result);
        }

        return result;
    }
};
} // namespace dxfcpp