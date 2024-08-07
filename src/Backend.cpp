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
                std::function<std::unique_ptr<Backend>(const tl::engine&, const json&)>> ProxyFactory::create_fn;

std::unordered_map<std::string,
                std::function<std::unique_ptr<Backend>(const tl::engine&, const json&)>> ProxyFactory::open_fn;

std::unique_ptr<Backend> ProxyFactory::createProxy(const std::string& backend_name,
                                                         const tl::engine& engine,
                                                         const json& config) {
    auto it = create_fn.find(backend_name);
    if(it == create_fn.end()) return nullptr;
    auto& f = it->second;
    return f(engine, config);
}

std::unique_ptr<Backend> ProxyFactory::openProxy(const std::string& backend_name,
                                                       const tl::engine& engine,
                                                       const json& config) {
    auto it = open_fn.find(backend_name);
    if(it == open_fn.end()) return nullptr;
    auto& f = it->second;
    return f(engine, config);
}

}
