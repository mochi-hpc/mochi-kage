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

kage::Result<bool> EchoProxy::forwardOutput(hg_id_t rpc_id, const char* input, size_t input_size,
                                            const std::function<void(const char*, size_t)>& output_cb) {
    (void)rpc_id;
    kage::Result<bool> result;
    result.success() = true;
    output_cb(input, input_size);
    return result;
}

void EchoProxy::setInputProxy(kage::InputProxy proxy) {
    m_input_proxy = std::move(proxy);
}

kage::Result<bool> EchoProxy::destroy() {
    kage::Result<bool> result;
    result.value() = true;
    return result;
}

std::unique_ptr<kage::Backend> EchoProxy::create(
        const thallium::engine& engine,
        const json& config,
        const thallium::pool& pool) {
    (void)engine;
    (void)pool;
    return std::unique_ptr<kage::Backend>(new EchoProxy(engine, config));
}
