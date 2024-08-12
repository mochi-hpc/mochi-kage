/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SERIALIZATION_HPP
#define __SERIALIZATION_HPP

#include <thallium.hpp>
#include <mercury_proc.h>
#include <kage/Backend.hpp>
#include <functional>

namespace kage {

class ProviderImpl;

class Serializer {

    const char* m_data;
    size_t      m_data_size;

    public:

    Serializer(const char* data, size_t data_size)
    : m_data{data}
    , m_data_size{data_size} {}

    template<typename A>
    void save(A& ar) const {
        ar.write(m_data, m_data_size);
    }
};

class Deserializer {

    std::function<void(const char*, size_t)> m_callback;

    public:

    Deserializer(std::function<void(const char*, size_t)> cb)
    : m_callback{std::move(cb)} {}

    template<typename A>
    void load(A& ar) {
        auto proc = ar.get_proc();
        auto data_size = hg_proc_get_size_left(proc);
        auto data = static_cast<char*>(hg_proc_save_ptr(proc, data_size));
        m_callback(data, data_size);
        hg_proc_restore_ptr(proc, data, data_size);
    }
};

}

#endif
