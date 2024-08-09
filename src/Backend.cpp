/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "kage/Backend.hpp"

namespace tl = thallium;

namespace kage {

using json = nlohmann::json;

std::unordered_map<std::string,
    std::function<std::unique_ptr<Backend>(
            const tl::engine&, const json&, const tl::provider_handle&, const tl::pool&)>>
        ProxyFactory::create_fn;

std::unique_ptr<Backend> ProxyFactory::createProxy(const std::string& backend_name,
                                                   const tl::engine& engine,
                                                   const json& config,
                                                   const tl::provider_handle& target,
                                                   const tl::pool& pool) {
    auto it = create_fn.find(backend_name);
    if(it == create_fn.end()) return nullptr;
    auto& f = it->second;
    return f(engine, config, target, pool);
}

}
