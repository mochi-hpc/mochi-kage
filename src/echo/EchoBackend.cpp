/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "EchoBackend.hpp"
#include <iostream>

KAGE_REGISTER_BACKEND(echo, EchoProxy);

EchoProxy::EchoProxy(thallium::engine engine, const json& config)
: m_engine(std::move(engine)),
  m_config(config) {

}

std::string EchoProxy::getConfig() const {
    return m_config.dump();
}

kage::Result<bool> EchoProxy::forward(const char* input, size_t input_size,
                                      const std::function<void(const char*, size_t)>& output_cb) {
    kage::Result<bool> result;
    result.success() = true;
    output_cb(input, input_size);
    return result;
}

kage::Result<bool> EchoProxy::destroy() {
    kage::Result<bool> result;
    result.value() = true;
    return result;
}

std::unique_ptr<kage::Backend> EchoProxy::create(const thallium::engine& engine, const json& config) {
    (void)engine;
    return std::unique_ptr<kage::Backend>(new EchoProxy(engine, config));
}

std::unique_ptr<kage::Backend> EchoProxy::open(const thallium::engine& engine, const json& config) {
    (void)engine;
    return std::unique_ptr<kage::Backend>(new EchoProxy(engine, config));
}
