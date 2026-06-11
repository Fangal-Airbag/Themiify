/*
 * RadiiU - an internet radio player for the Wii U.
 *
 * Copyright (C) 2025-2026  Daniel K. O. <dkosmari>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef THREAD_SAFE_HPP
#define THREAD_SAFE_HPP

#include <mutex>
#include <type_traits>
#include <utility>


template<typename T>
class thread_safe {

    mutable std::mutex mutex;
    T data;

public:

    template<typename U>
    class guard {

        std::unique_lock<std::mutex> guard_;
        U* data_ = nullptr;

        guard(std::mutex& m,
              U* d) :
            guard_{m},
            data_{d}
        {}

        guard(std::unique_lock<std::mutex>&& locker,
              U* d) :
            guard_{std::move(locker)},
            data_{d}
        {}

        friend class thread_safe<std::remove_cv_t<U>>;

    public:

        guard(guard&& other) :
            guard_{std::move(other.guard_)},
            data_{other.data_}
        {
            other.data_ = nullptr;
        }

        guard&
        operator =(guard&& other)
            noexcept
        {
            if (this != other) {
                guard_ = std::move(other.guard_);
                data_ = other.data_;
                other.data_ = nullptr;
            }
            return *this;
        }

        U&
        operator *()
            noexcept
        {
            return *data_;
        }

        U*
        operator ->()
            const noexcept
        {
            return data_;
        }

        explicit
        operator bool()
            const noexcept
        {
            return guard_.owns_lock();
        }

    }; // class guard


    // thread_safe() = default;


    template<typename... Args>
    thread_safe(Args&& ...args) :
        data(std::forward<Args>(args)...)
    {}


    [[nodiscard]]
    guard<T>
    lock()
        &
    {
        return guard<T>{mutex, &data};
    }


    [[nodiscard]]
    guard<const T>
    lock()
        const &
    {
        return guard<const T>{mutex, &data};
    }


    [[nodiscard]]
    guard<const T>
    c_lock()
        const &
    {
        return guard<const T>{mutex, &data};
    }


    [[nodiscard]]
    guard<T>
    try_lock()
        &
    {
        return {std::unique_lock{mutex, std::try_to_lock}, &data};
    }


    [[nodiscard]]
    T
    load()
        const &
    {
        return *lock();
    }


    template<typename U>
    void
    store(U&& new_data)
        &
    {
        *lock() = std::forward<U>(new_data);
    }

}; // class thread_safe<T>

#endif
