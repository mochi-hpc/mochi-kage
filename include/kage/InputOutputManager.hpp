/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_IO_MANAGER_HPP
#define __KAGE_IO_MANAGER_HPP

#include <thallium.hpp>
#include <mercury_proc.h>
#include <kage/Backend.hpp>

namespace kage {

class ProviderImpl;

class InputOutputManager {

    friend class ProviderImpl;

    Backend&                 m_backend;
    const thallium::request& m_request;
    const char*              m_output = nullptr;
    size_t                   m_output_size = 0;

    InputOutputManager(Backend& backend, const thallium::request& req)
    : m_backend{backend}
    , m_request{req} {}

    public:

    template<typename A>
    void save(A& ar) const {
        ar.write(m_output, m_output_size);
    }

    template<typename A>
    void load(A& ar) {
        auto proc = ar.get_proc();
        auto input_size = hg_proc_get_size_left(proc);
        auto input = static_cast<char*>(hg_proc_save_ptr(proc, input_size));
        auto output_cb = [this](const char* output, size_t output_size) -> void {
            m_output = output;
            m_output_size = output_size;
            m_request.respond(*this);
            m_output = nullptr;
            m_output_size = 0;
        };
        m_backend.forward(input, input_size, output_cb);
    }
};

}

#endif
