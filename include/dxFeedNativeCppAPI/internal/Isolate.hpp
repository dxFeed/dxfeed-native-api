// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "CEntryPointErrors.hpp"
#include "Common.hpp"

#include <graal_isolate.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
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
                                         bit_cast<std::size_t>(handle), isMain, tid, idx);
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
            return fmt::format("IsolateThread{{{}, isMain = {}, tid = {}, idx = {}}}", bit_cast<std::size_t>(handle),
                               isMain, tid, idx);
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
            std::clog << fmt::format("Isolate{{{}, main = {}, current = {}}}()\n", bit_cast<std::size_t>(handle),
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

    // Please wrap non-primitive user-defined data types in std::unique_ptr
    template <typename F>
    auto runIsolated(F &&f)
        -> std::variant<CEntryPointErrors, decltype(std::invoke(std::forward<F>(f), currentIsolateThread_.handle))> {
        std::lock_guard lock(mutex_);
        if constexpr (isDebug) {
            std::clog << fmt::format("{}::runIsolated({})\n", toString(), bit_cast<std::size_t>(&f));
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

        return fmt::format("Isolate{{{}, main = {}, current = {}}}", bit_cast<std::size_t>(handle_),
                           mainIsolateThread_.toString(), currentIsolateThread_.toString());
    }
};
} // namespace detail

} // namespace dxfcpp