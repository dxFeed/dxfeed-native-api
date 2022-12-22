// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <dxfg_endpoint.h>
#include <dxfg_system.h>
#include <graal_isolate.h>

#include "internal/CEntryPointErrors.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include <fmt/format.h>
#include <fmt/std.h>

namespace dxfcpp {

namespace detail {
using GraalIsolateHandle = std::add_pointer_t<graal_isolate_t>;
using GraalIsolateThreadHandle = std::add_pointer_t<graal_isolatethread_t>;

class Isolate final {
    struct IsolateThread final {
        GraalIsolateThreadHandle handle{};
        bool isMain{};
        std::thread::id tid{};
        std::size_t idx{};

        explicit IsolateThread(GraalIsolateThreadHandle handle = nullptr, bool isMain = false) noexcept
            : handle{handle}, isMain{isMain}, tid{std::this_thread::get_id()} {
            static size_t idx = 0;

            this->idx = idx++;

            if constexpr (isDebug) {
                std::clog << fmt::format("IsolateThread{{{}, isMain = {}, tid = {}, idx = {}}}()\n",
                                         std::bit_cast<std::size_t>(handle), isMain, tid, idx);
            }
        }

        CEntryPointErrors detach() {
            if constexpr (isDebug) {
                std::clog << fmt::format("{}::detach()\n", toString());
            }

            // OK if nothing is attached.
            if (!handle) {
                if constexpr (isDebug) {
                    std::clog << "\tNot attached \n";
                }

                return CEntryPointErrors::NO_ERROR;
            }

            auto result = CEntryPointErrors::valueOf(graal_detach_thread(handle));

            if (result == CEntryPointErrors::NO_ERROR) {
                if constexpr (isDebug) {
                    std::clog << "\tDetached \n";
                }

                handle = nullptr;
            }

            return result;
        }

        CEntryPointErrors detachAllThreadsAndTearDownIsolate() {
            if constexpr (isDebug) {
                std::clog << fmt::format("{}::detachAllThreadsAndTearDownIsolate()\n", toString());
            }

            if (!handle) {
                if constexpr (isDebug) {
                    std::clog << "\tNot attached\n";
                }

                return CEntryPointErrors::NO_ERROR;
            }

            auto result = CEntryPointErrors::valueOf(graal_detach_all_threads_and_tear_down_isolate(handle));

            if (result == CEntryPointErrors::NO_ERROR) {
                if constexpr (isDebug) {
                    std::clog << "\tAll threads have been detached. The isolate has been teared down.\n";
                }

                handle = nullptr;
            }

            return result;
        }

        ~IsolateThread() {
            if constexpr (isDebug) {
                std::clog << fmt::format("~{}()\n", toString());
            }

            if (isMain) {
                if constexpr (isDebug) {
                    std::clog << "\tThis is the main thread\n";
                }

                return;
            }

            detach();
        }

        std::string toString() const {
            return fmt::format("IsolateThread{{{}, isMain = {}, tid = {}, idx = {}}}",
                               std::bit_cast<std::size_t>(handle), isMain, tid, idx);
        }
    };

    mutable std::recursive_mutex mutex_{};

    GraalIsolateHandle handle_;
    IsolateThread mainIsolateThread_;
    static thread_local IsolateThread currentIsolateThread_;

    Isolate(GraalIsolateHandle handle, GraalIsolateThreadHandle mainIsolateThreadHandle)
        : handle_{handle}, mainIsolateThread_{mainIsolateThreadHandle, true} {

        currentIsolateThread_.handle = mainIsolateThreadHandle;
        currentIsolateThread_.isMain = true;

        if constexpr (isDebug) {
            std::clog << fmt::format("Isolate{{{}, main = {}, current = {}}}()\n", std::bit_cast<std::size_t>(handle),
                                     mainIsolateThread_.toString(), currentIsolateThread_.toString());
        }
    }

    static std::shared_ptr<Isolate> create() {
        if constexpr (isDebug) {
            std::clog << "Isolate::create()\n";
        }

        GraalIsolateHandle graalIsolateHandle{};
        GraalIsolateThreadHandle graalIsolateThreadHandle{};

        if (CEntryPointErrors::valueOf(graal_create_isolate(nullptr, &graalIsolateHandle, &graalIsolateThreadHandle)) ==
            CEntryPointErrors::NO_ERROR) {

            auto result = std::shared_ptr<Isolate>{new Isolate{graalIsolateHandle, graalIsolateThreadHandle}};

            if constexpr (isDebug) {
                std::clog << fmt::format("Isolate::create() -> *{}\n", result->toString());
            }

            return result;
        }

        if constexpr (isDebug) {
            std::clog << "\t-> nullptr \n";
        }

        return nullptr;
    }

    CEntryPointErrors attach() {
        if constexpr (isDebug) {
            std::clog << fmt::format("{}::attach()\n", toString());
        }

        // We will not re-attach.
        if (!currentIsolateThread_.handle) {
            if constexpr (isDebug) {
                std::clog << "\tNeeds to be attached.\n";
            }

            GraalIsolateThreadHandle newIsolateThreadHandle{};

            if (auto result = CEntryPointErrors::valueOf(graal_attach_thread(handle_, &newIsolateThreadHandle));
                result != CEntryPointErrors::NO_ERROR) {

                if constexpr (isDebug) {
                    std::clog << fmt::format("\t-> {}\n", result.getDescription());
                }

                return result;
            }

            currentIsolateThread_.handle = newIsolateThreadHandle;
            currentIsolateThread_.isMain = mainIsolateThread_.handle == newIsolateThreadHandle;

            if constexpr (isDebug) {
                std::clog << fmt::format("\tAttached: {}\n", currentIsolateThread_.toString());
            }
        } else {
            if constexpr (isDebug) {
                std::clog << fmt::format("\tCached: {}\n", currentIsolateThread_.toString());
            }
        }

        return CEntryPointErrors::NO_ERROR;
    }

  public:
    Isolate() = delete;
    Isolate(const Isolate &) = delete;
    Isolate &operator=(const Isolate &) = delete;

    static std::shared_ptr<Isolate> getInstance() {
        if constexpr (isDebug) {
            std::clog << "Isolate::getInstance()\n";
        }

        static std::shared_ptr<Isolate> instance = create();

        if constexpr (isDebug) {
            std::clog << fmt::format("Isolate::getInstance() -> *{}\n", instance->toString());
        }

        return instance;
    }

    template <typename F>
    auto runIsolated(F &&f)
        -> std::variant<CEntryPointErrors, decltype(std::invoke(std::forward<F>(f), currentIsolateThread_.handle))> {
        std::lock_guard lock(mutex_);
        if constexpr (isDebug) {
            std::clog << fmt::format("{}::runIsolated({})\n", toString(), std::bit_cast<std::size_t>(&f));
        }

        if (auto result = attach(); result != CEntryPointErrors::NO_ERROR) {
            if constexpr (isDebug) {
                std::clog << fmt::format("\t-> {}\n", result.getDescription());
            }

            return result;
        }

        return std::invoke(std::forward<F>(f), currentIsolateThread_.handle);
    }

    ~Isolate() {
        std::lock_guard lock(mutex_);

        if constexpr (isDebug) {
            std::clog << fmt::format("~{}()\n", toString());
        }

        mainIsolateThread_.detachAllThreadsAndTearDownIsolate();
    }

    std::string toString() const {
        std::lock_guard lock(mutex_);

        return fmt::format("Isolate{{{}, main = {}, current = {}}}", std::bit_cast<std::size_t>(handle_),
                           mainIsolateThread_.toString(), currentIsolateThread_.toString());
    }
};

} // namespace detail

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
        if constexpr (dxfcpp::detail::isDebug) {
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

        if constexpr (dxfcpp::detail::isDebug) {
            std::clog << fmt::format("System::getProperty(key = '{}') -> '{}'\n", key, result);
        }

        return result;
    }
};

struct DXEndpoint {
    //    class Builder {
    //        dxfg_endpoint_builder_t handle_;
    //        std::mutex mutex_;
    //
    //        explicit Builder(dxfg_endpoint_builder_t handle) : handle_{handle}, mutex_{} {}
    //
    //      public:
    //        ~Builder() {
    //            std::lock_guard<std::mutex> guard{mutex_};
    //
    //            auto t = detail::Isolate::getInstance()->attachThread();
    //
    //            dxfg_endpoint_builder_release(t, &handle_);
    //        }
    //
    //        static std::shared_ptr<Builder> create() {
    //            dxfg_endpoint_builder_t builderHandle;
    //            auto t = detail::Isolate::getInstance()->attachThread();
    //
    //            if (!dxfg_endpoint_builder_create(t, &builderHandle)) {
    //                return nullptr;
    //            }
    //
    //            return std::shared_ptr<Builder>(new Builder{builderHandle});
    //        }
    //    };
};

} // namespace dxfcpp
