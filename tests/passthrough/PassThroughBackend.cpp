/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "PassThroughBackend.hpp"
#include <iostream>

KAGE_REGISTER_BACKEND(passthrough, PassThroughProxy);

PassThroughProxy::PassThroughProxy(thallium::engine engine, const json& config)
: m_engine(std::move(engine)),
  m_config(config) {

}

std::string PassThroughProxy::getConfig() const {
    return m_config.dump();
}

kage::Result<bool> PassThroughProxy::forwardOutput(hg_id_t rpc_id, const char* input, size_t input_size,
                                                   const std::function<void(const char*, size_t)>& output_cb) {
    return m_input_proxy.forwardInput(rpc_id, input, input_size, output_cb);
}

void PassThroughProxy::setInputProxy(kage::InputProxy proxy) {
    m_input_proxy = std::move(proxy);
}

kage::Result<bool> PassThroughProxy::destroy() {
    kage::Result<bool> result;
    result.value() = true;
    return result;
}

std::unique_ptr<kage::Backend> PassThroughProxy::create(
        const thallium::engine& engine,
        const json& config,
        const thallium::pool& pool) {
    (void)engine;
    (void)pool;
    return std::unique_ptr<kage::Backend>(new PassThroughProxy(engine, config));
}
