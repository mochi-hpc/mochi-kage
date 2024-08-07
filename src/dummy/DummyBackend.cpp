/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "DummyBackend.hpp"
#include <iostream>

KAGE_REGISTER_BACKEND(dummy, DummyProxy);

DummyProxy::DummyProxy(thallium::engine engine, const json& config)
: m_engine(std::move(engine)),
  m_config(config) {

}

void DummyProxy::sayHello() {
    std::cout << "Hello World" << std::endl;
}

std::string DummyProxy::getConfig() const {
    return m_config.dump();
}

kage::Result<int32_t> DummyProxy::computeSum(int32_t x, int32_t y) {
    kage::Result<int32_t> result;
    result.value() = x + y;
    return result;
}

kage::Result<bool> DummyProxy::destroy() {
    kage::Result<bool> result;
    result.value() = true;
    // or result.success() = true
    return result;
}

std::unique_ptr<kage::Backend> DummyProxy::create(const thallium::engine& engine, const json& config) {
    (void)engine;
    return std::unique_ptr<kage::Backend>(new DummyProxy(engine, config));
}

std::unique_ptr<kage::Backend> DummyProxy::open(const thallium::engine& engine, const json& config) {
    (void)engine;
    return std::unique_ptr<kage::Backend>(new DummyProxy(engine, config));
}
